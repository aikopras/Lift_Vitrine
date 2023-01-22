//*****************************************************************************************************
// File:      Lift-Main.ino
// Author:    Aiko Pras
// History:   2021/04/24 AP Version 1.0: RS485 and stepper part ready
//            2022/01/04 AP Version 1.1: DCC part added
//
//
// Buttons:
// ========
// If the lift is not moving, the following button behavior is possible:
// - Short press of one of the 0..10 buttons: Lift starts moving to level 0..10
// - Short press of the RESET button: a homing cycle will be initiated
// - Press of ^ or v buttons: Lift moves in small steps up or down (jogging)
// - Long  press of one of the 0..10 buttons: position will be stored as new position for that level
//
// While the lift moves the associated button Led will slowly flash. 
// While moving, all RS-Bus bits will be cleared.
// After the move is complete, the Led will shortly light and subsequently dimm. 
// The corresponding RS-Bus level bit bit will be set, as well as the STEPPER_IDLE bit.
// If a new position is being stored, the LED will flash quickly
//
// If the lift is moving, we keep polling the button controller, but ignore all buttons except the RESET.
// If the RESET is pushed (short press), the lift will immediately be stopped (see emergence stop below).
//
// DCC and RS-Bus addresses
// ========================
// The DCC and RS-Bus address can be set using the onboard button, like all other decoder boards.
// Before doing so, it is important to understand some details.
// 1) The Lenz system allows RS-Bus feedback for switch addresses <= 256, thus for decoder / RS-Bus 
// addresses <= 64. Lenz reserves the RS-Bus addresses between 65 and 128 for feedback decoders.
// Although my libraries are able to use the entire RS-Bus address range for switches, limitations of
// the RS-Bus address space prevent RS-Bus feedback for addresses > 128 (switch addresses > 512).
// Without the special measures taken in this sketch, this would prevent the use of lift addreses > 512.
// 2) A lift decoder is not the same as a normal switch decoder. 
// Although this sketch uses 11 switch addresses to select the level (0..10) the lift should move to,
// usage of the RS-Bus feedback messages is slightly different compared to normal switches.
// To inform the traincontroller software at which level the lift currently is (0..10), 11 feedback
// bits are used. However, additional feedback bits are needed to signal that the stepper motors are IDLE
// (the lift has arrived at the requested level) and that no obstacles are detected by the IR system. 
// 3) Since the standard relation that exists for normal switches between the switch and RS-Bus
// address does not hold for a lift decoder, the best approach seems to use switch addresses > 512
// and reserve / "hard-code" the RS-Bus addresses 126 and 127 (RS-Bus 128 is already taken for PoM).
// 4) For the lift we need 11 switch addresses, thus 3 switch decoder addresses.
// In addition, we need 2 RS-Bus addresses.
// 5) This sketch uses the following logic to determine the switch and RS-Bus addresses.
// - If  (after pressing the onboard button) an switch address is selected <= 504, decoder and RS-Bus
// addresses will be calculated as normal. In that case all RS-Bus addresses will be <= 127.  
// - If  (after pressing the onboard button) an switch address is selected > 504, the decoder address
// will be calculated as normal, but the RS-Bus address will be "hard-wired" to addresses 126 and 127.
//
// Emergency stop
// ==============
// If the lift is moving, it can be stopped by either pushing the RESET button, or by sending a DCC 
// Emergency Stop command (= Broadcast ResetCmd). In both cases the RESET button will quickly flash.
// 1) BUTTON: The Button generates a soft_reset, which immediately halts the steppers and resets Grbl. 
// If reset while in motion, Grbl will throw an Alarm to indicate position may be lost from the 
// motion halt. The Alarm state is signalled via the RESET Button LED, which quickly flashes.
// A second push of the RESET button is needed to leave the Alarm state. The lift stays at an 
// undefined position.
// 2) DCC Emergency Stop command: generates a GRBL feedhold, 


// Onboard LEDS:
// ============
// LED_GREEN: Power connected / IR sensors are free (if CV enabled)
// LED_BLUE: Lights if the steppers are busy
// LED_YELLOW: Mimics the LEDs on the remote panel (without flashing)
// LED_RED: The standard programming and RS-Bus Feedback LED
//
// The code is developed for Arduino 2560 microcontrollers and has been tested on the dedicated 
// lift controller board: https://easyeda.com/aikopras/support-lift-controller
// See "hardware.h" for details regarding this board, as well as compile and flash settings
//
//*****************************************************************************************************
#include <LiquidCrystal.h>        // Allow LCD output of the current state and position
#include "hardware.h"             // Pins for LEDs, DCC input, RS-Bus output etc.
#include "support.h"              // For the LCD Display and some on-board LEDs
#include "rs485.h"                // Use RS485 to read button status and set button LEDs
#include "stepper.h"              // Communication with the stepper motor controller (GRBL)
#include "dcc_rs.h"               // DCC and RS-Bus specific code
#include "relays.h"               // For connecting two external relays 

unsigned int firstDecoderAddress;


//*****************************************************************************************************
// SETUP
//*****************************************************************************************************
void setup() {
  // The monitor is connected via Serial (UART0) and the GRBL controller via Serial2 (UART2)
  // The Serial_Line CV disables (value = 0) or enables the Serial interface.
  // This interface is connected to UART0, provides 5V logic and may be connected to a computer via a
  // Serial to USB converter. 
  // If enabled, interference on the interface line may be possible. Enabling is needed, however, to
  // make GRBL configuration changes (for example, $27=8.6 to change the zero offset).
  // A value of 1 gives basic (debugging) information, a value of 2 gives detailed information.
  if (cvValues.read(Serial_Line)) Serial.begin(115200);
  Serial2.begin(115200);                  // GRBL = MEGA 328 
  // While debugging, the lcd_display may be active for debugging
  lcd_display.init();
  // Initialise the DCC and RS-Bus part of the lift decoder. See above for details regarding addresses.
  // After cvValues.init() is called, CV default values may be modified using cvValues.defaults[...]
  cvValues.init(LiftDecoder,11);          // software version may be added as 2nd parameter. Default: 10
  decoderHardware.init();
  // We use 11 switch addresses, one per level. Therefore we need 3 decoder addresses
  firstDecoderAddress = cvValues.storedAddress();
  accCmd.setMyAddress(firstDecoderAddress, firstDecoderAddress + 2);
  if (cvValues.storedAddress() < 126) rsbus.init(cvValues.read(myRSAddr));
    else rsbus.init(126);
  // At start-up the values for lift.level and lift.currentPosition are zero, 
  // and the RS-Bus level bit and STEPPER_IDLE bit will be set and returned to the RS-Bus master.
  // To ensure a consistent state, do a homing cycle first. Wait 1 sec to ensure GRBL is up and running
  delay(1000);
  if (cvValues.read(StartHoming)) reset_object.home(); 
  /* Write a 1 to the LCD_Display CV to enable the LCD Display for debugging */
  // cvValues.write(LCD_Display, 1);
}


void serialMonitor() {
  // For testing purposes read from monitor port and send to GRBL port. Note that we can't read here 
  // what comes back from the GRBL controller, since that is handled by the grbl parser.
  // We have installed GRBL V1.1h (with modified config.h)
  // - $$  = view settings
  // - x20 = Move X stepper to 20 mm
  // - ?   = status request
  if (cvValues.read(Serial_Line)) {
    if (Serial.available()) {
      char inByte = Serial.read();
      Serial2.print(inByte);
    }
  }
}
//*****************************************************************************************************
// Main Loop
//*****************************************************************************************************
uint8_t level;
bool dccReset = 0; 

void loop() {
  // The main loop handles parses status and receices commands from:
  // - The serial monitor 
  // - The Stepper controller
  // - The IR-Led controller
  // - The Button controller
  // - The DCC controller
  // The serial monitor, Button and DCC controller issue commands to move the steppers 
  // Feedback information is provided via the RS-Bus, LCD display and Button LEDs. 
  
  //===================================================================================
  // Step 1: Serial monitor
  // For testing purposes we accept input via the serial monitor. Input should be valid 
  // GRBL commands, such as G90 X20 Y20 (absolute position) or G91 X20 Y20 (delta move)
  serialMonitor();
  
  //===================================================================================
  // Step 2: Stepper Controller 
  // Update the stepper controller. This involves parsing characters received via the
  // serial interface from the GRBL controller, interpreting the received GRBL response
  // message and periodically sending the GRBL controller a poll message (?).
  // Via the stepper controller we also update the jog_object, which handles stepper 
  // moves while an UP or DOWN button is being pressed.
  stepper.update(); 
  //
  // After parsing a number of GRBL response characters we may conclude that
  // the stepper state and/or stepper position have changed
  if (stepper.state_changed()) {
    lcd_display.show();     // Show the buttonnumber and the stepper status
    if (stepper.state == grbl::IDLE) {
      digitalWrite(LED_BLUE, LOW);          // Indicate the steppers are idle
      btn_cntrl.prepare_LED(LED_OFF, btn_cntrl.buttonNumber);  // buttonNumber: 0..13
      // Ensure that the lift position (in mm) matches the requested level.
      // In the (unlikely) case that only one of the two ATMega processors (2560 and 328)
      // performed a reset, both processors will be in different and thus inconsistent
      // states. If that happens, the user should perform a homing cycle (push RESET)
      if (strcmp(lift.currentPosition,lift.positions[lift.level]) == 0) {
        rsbus.setLiftLevel(lift.level);
        relaysCntrl.lift_idle(lift.level);  // if at level 0, switch the relays to POS1
      }      
      else {
        rsbus.clearFeedbackBits();          // to catch fast changes, such as step-up
        digitalWrite(LED_BLUE, HIGH);       // Indicate the steppers are busy 
      }
    }
    else {
      rsbus.clearFeedbackBits();            // to catch normal changes
      digitalWrite(LED_BLUE, HIGH);         // Indicate the steppers are busy 
    }
  }
  else if (stepper.position_changed()) { 
    lcd_display.show();
    relaysCntrl.lift_moving();              // Not at level 0, switch the relays to POS2
  }
  //===================================================================================
  // Step 3: Send the IR-Sensor controller a poll message, and to the Button controller
  // a poll message or a command to change the button LEDs.
  // The controller object send these messages over the RS-485 bus with 50ms intervals.
  controllers.talk485();
  // 
  //===================================================================================
  // Step 4: If the IR-Sensor controller changes state, send feedback via the RS-Bus
  // sensorIsFree is a boolean variable, indicating if no IR-lightbeam is blocked
  if (ir_cntrl.stateChanged()) {
    rsbus.irFree = ir_cntrl.sensorIsFree;
    rsbus.sendMainNibble();
  }
  //
  //===================================================================================
  // Step 5: Button controller
  // Did an event occur associated with a level button (buttons 0..10)
  // and is this not the button of the level we already are?  
  if (btn_cntrl.level_button_event()) {
    if (stepper.state == grbl::IDLE) {
      // Store the button number, to enable future display and LED-OFF messages  
      lift.level = btn_cntrl.buttonNumber;
      // If the stepper motors are not moving, new commands can be accepted 
      if (btn_cntrl.buttonAction == PRESSED) {
        // Immediately after being pressed, put the associated LED on
        btn_cntrl.prepare_LED(LED_ON, lift.level);
      }
      else if (btn_cntrl.buttonAction == SHORTPRESS) {
        // In case of a short press, the lift moves to the requested level
        // provided the lift's position is not yet at the requested level
        if (strcmp(lift.currentPosition,lift.positions[lift.level])) {
          lift.move(lift.level);
          lcd_display.show();
          btn_cntrl.prepare_LED(FLASH_SLOW, lift.level);
        }
        else btn_cntrl.prepare_LED(LED_OFF, lift.level);
      }
      else if (btn_cntrl.buttonAction == LONGPRESS) {
        // The current position should be used and stored
        strcpy(lift.positions[lift.level], lift.currentPosition);
        lift.storePosition(lift.level);
        btn_cntrl.prepare_LED(LED_OFF, lift.level);
      }
    }
  } 
  // Did an event occur associated with the UP or DOWN button?
  if (btn_cntrl.up_down_button_event()) {
    if (stepper.state == grbl::IDLE) {
      // If the stepper motors are not moving, new commands can be accepted        
      if (btn_cntrl.buttonAction == PRESSED) {
        // Immediately after being pressed, start moving and put the associated LED on
        if (btn_cntrl.buttonUpOrDown == btn_cntrl.UP) jog_object.start(jog_object.UP);
        else jog_object.start(jog_object.DOWN);
        btn_cntrl.prepare_LED(LED_ON, btn_cntrl.buttonNumber);
      }
    }
    if (btn_cntrl.buttonAction == RELEASED) {
      jog_object.cancel();
      btn_cntrl.prepare_LED(LED_OFF, btn_cntrl.buttonNumber); // switch off the LED for this button
    }
  }  
  // Did an event occur associated with the RESET button?
  // If we the state is IDLE, a homing cycle will be started
  // If the state is RUN, JOG or HOLD, a Soft-Reset message will be send
  // If the state is ALARM, an unlock message will be send  
  // If the alarm state is unlocked, a new Homing cycle may be needed 
  if (btn_cntrl.alarm_button_event()) {
   if (btn_cntrl.buttonAction == PRESSED) {
     switch (stepper.state) {
       case grbl::IDLE:
         rsbus.clearFeedbackBits();         // We leave the IDLE state and the current level
         lift.level = 0;                    // This will become the new level
         btn_cntrl.prepare_LED(FLASH_SLOW, RESET_BUTTON);
         lcd_display.homing();
         reset_object.home(); 
       break;
       case grbl::RUN: 
       case grbl::JOG: 
       case grbl::HOLD: 
         reset_object.soft_reset(); 
         btn_cntrl.prepare_LED(FLASH_FAST, RESET_BUTTON);
       break;
       case grbl::ALARM: 
         reset_object.unlock(); 
         btn_cntrl.prepare_LED(LED_OFF, lift.level);
       break;
       default:                             // UNKNOWN, HOMING
       break;
      }
    }
  }

  //===================================================================================
  // Step 6: DCC controller
  // If the DCC Controller receives an emergency stop, the best seems to send a  
  // feed-hold command. This allows to lift to continue its movement afterwards. 
  // In case the lift should not restart its movement after a DCC emergency stop, while still  
  // in the HOLD state the reset-button may be pushed and a soft-reset will be performed
  if (dcc.input()) {
    switch (dcc.cmdType) {
      // The next two to stop the lift via feedhold
      // ResetCmd is send after the STOP button on the LH100 is pushed, or after TC Einfrieren
      // MyEmergencyStopCmd is never received from a LZV100, but included for possible future versions
      case Dcc::ResetCmd :
      case Dcc::MyEmergencyStopCmd:  
        reset_object.feedhold(); 
        btn_cntrl.prepare_LED(FLASH_FAST, RESET_BUTTON);
        lcd_display.show();
        dccReset = true;
      break;
      // Emergency stop is over. Send a resume
      case Dcc::SomeLocoSpeedFlag :
      if (dccReset) {
        reset_object.resume();
        btn_cntrl.prepare_LED(LED_OFF, RESET_BUTTON);
        dccReset = false;
        lcd_display.show();
      }
      break;
      // Move the lift to the requested leve;
      case Dcc::MyAccessoryCmd :
        if (accCmd.command == Accessory::basic)
        // If the stepper motors are inactive and the switch poition is '+', move the lift
        if ((stepper.state == grbl::IDLE) && (accCmd.position == 1)) {
          lift.level = (accCmd.decoderAddress - firstDecoderAddress) * 4 + accCmd.turnout - 1;
          lift.move(lift.level);
          if (cvValues.read(Serial_Line)) Serial.print("Move lift to level: ");
          if (cvValues.read(Serial_Line)) Serial.println(lift.level);
          lcd_display.show();
        }
      break;      
      case Dcc::MyPomCmd :
        cvProgramming.processMessage(Dcc::MyPomCmd);
        break;
      case Dcc::SmCmd :
        cvProgramming.processMessage(Dcc::SmCmd);
        break;
      default:
        break;
    }
  }
  // As frequent as possible we should call the RS-Bus address polling routine, check if the 
  // programming button is pushed, and if the status of the onboard LED should be changed.
  decoderHardware.update();
  rsbus.update();
  relaysCntrl.update();
}
