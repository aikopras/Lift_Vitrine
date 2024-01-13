//*****************************************************************************************************
// File:      Lift-Main.ino
// Author:    Aiko Pras
// History:   2021/04/24 AP Version 1.0: RS485 and stepper part ready
//            2022/01/04 AP Version 1.1: DCC part added
//            2024/01/10 AP Version 1.2: mySettings.h added, to ease user customization
//            2024/01/12 AP Version 2.0: Feedback is also provided via the onboard connectors
//
//
// Buttons:
// ========
// If the lift is not moving, the following button behavior is possible:
// - Short press of one of the 0..10 buttons: Lift starts moving to level 0..10
// - Short press of the RESET button: a homing cycle will be initiated
// - Press of ^ or v buttons: Lift moves in small steps up or down (jogging)
// - Long press of one of the 0..10 buttons: position will be stored as new position for that level
//
// While the lift moves, the associated button Led will slowly flash. 
// After the move is complete, the button Led will shortly light and subsequently dimm.
// If a new position is being stored, the button LED will flash quickly
//
// If the lift is moving, we keep polling the button controller, but ignore all buttons except the RESET.
// If the RESET is pushed (short press), the lift will immediately be stopped (see emergence stop below).
//
// DCC address
// ===========
// The common approach for all my DCC decoder boards, is to compile the DCC address with an
// "illegal" value. This triggers the board to notify the user, via blinking of the red LED, that
// a DCC address needs to be entered. Like all other decoder boards, this DCC address can be set using
// the onboard programming button. This new address is permanently stored (in EEPROM), and the next time
// the decoder starts, the valid DCC address is read from EEPROM and the red LED no longer blinks.
// You might prefer to set the DCC address already at compile time, however. This can be done by  
// changing the CV1 and CV9 settings in the file mySettings.h 
//
// RS-Bus address
// ==============
// The onboard programming button is not only used to set the DCC address, but indirectly also the 
// RS-Bus address. If you select a decoder address < 128 (thus a switch addresses <509), the RS-Bus
// address becomes equal to the decoder address (+1). If you select a decoder address >=128, the 
// RS-Bus address becomes 0, meaning "not used". 
// Via the onboard programming button it is therefore not possible to use decoder addresses >=128, 
// in combination with RS-Bus feedback.
// Fortunately, we can still use DCC decoder addresses >=128, in combination with RS-Bus feedback, 
// provided we select these addresses before compilation, in the file mySettings.h.
// We use 12 feedback bits to inform traincontroller (or whatever software we have) at which level
// the lift currently is (0..11). Additional feedback bits are used to signal that the stepper 
// motors are IDLE (the lift has arrived at the requested level) and that no obstacles are detected
// by the IR system. We therefore use 2 RS-Bus addresses. 
//
// Emergency stop
// ==============
// If the lift is moving, it can be stopped by either pushing the RESET button, or by sending a DCC 
// Emergency Stop command (= Broadcast ResetCmd). In both cases the RESET button will quickly flash.
// 1) BUTTON: The Button generates a soft_reset, which immediately halts the steppers and resets Grbl. 
// If reset while in motion, Grbl will throw an Alarm to indicate position may be lost from the 
// motion halt. The Alarm state is signaled via the RESET Button LED, which quickly flashes.
// A second push of the RESET button is needed to leave the Alarm state. The lift stays at an 
// undefined position.
// 2) DCC Emergency Stop command: generates a GRBL feedhold.
//
// Onboard LEDS:
// ============
// LED_GREEN: Power connected / IR sensors are free (or disabled)
// LED_BLUE: Lights if the steppers are busy
// LED_YELLOW: Mimics the LEDs on the remote button panel (without flashing)
// LED_RED: The standard programming and RS-Bus Feedback LED
//
// Adjusting behaviour
// ===================
// Some aspects of the lift decoder's behaviour can be modified by the user of this software.
// See the file mySettings.h for details
//
// Feedback
// ========
// Feedback is provided regarding the lift's position and status. There are three status bits:
// - The IR-sensors are free (provided IR-sensors are active)
// - The Lift has arrived / is at level x. There is no movement and the stepper motors are idle
// - The lift is ready. 
//   If the IR-Sensors are active, this bit is the same as 'IR-sensors are free' AND 'Lift is at level x'.
//   If the IR-Sensors are inactive, this bit is the same as 'Lift is at level x'.
//   Analysing this single bit may, in Train Control software, be easier than two bits.
// During movement of the lift, all feedback bits are cleared.
// Feedback is provided in two ways.
// 1) Using the RS-Bus. We use two addresses; the meaning of the individual bits, is as follows:
//   - Base address    : Level 0..7                (low and high nibble. Bit 0..7)
//   - Base address + 1: Level 8..11               (low nibble.          Bit 0..3)
//   - Base address + 1: IR-sensors are free       (high nibble          Bit 4)
//   - Base address + 1: Lift is at level x        (high nibble          Bit 5)
//   - Base address + 1: Lift Ready                (high nibble          Bit 7)
// 2) Using the connectors on the Main Lift Board connectors (added in V2.0).
// Its purpose is to facilitate the use of alternative feedback systems (such as S88). 
//   - The connectors labelled "IN 1..12" are used to tell which level the lift currently is.
//     The connector labelled "1" is for level "0", etc. 
//   - The connector labelled "IN 13" is used to tell that IR-sensors are free.
//   - The connector labelled "IN 14" is used to tell that lift has arrived / is at level x.
//   - The pin labelled "OUT 1" is to tell that the lift is ready. 
// Note that the IN connectors are directly connected to pins on the ATMega 2560 processor,
// (accidental) shortcut of these pins will destroy the processor. 
// Therefore ensure you always use resistors with a value of 1 kOhm or higher.
//
// Relays
// ======
// As an extra safety measure, it is possible to connect two bi-stable relays to the lift decoder.
// This allows the power of the lift tracks, or the track towards the lift, to be switched off.
// In case the lift is IDLE and at level 0, both relays are swiched to "position 1".
// In all other cases both relays will be swiched to "position 2".
//
//
//*****************************************************************************************************
#include <LiquidCrystal.h>        // Allow LCD output of the current state and position
#include "mySettings.h"           // Allows user of this sketch to tailor the behaviour 
#include "hardware.h"             // Pins for LEDs, DCC input, RS-Bus output etc.
#include "support.h"              // For the LCD Display and some on-board LEDs
#include "rs485.h"                // Use RS485 to read IR-sensors. button status, set button LEDs 
#include "stepper.h"              // Communication with the stepper motor controller (GRBL)
#include "feedback.h"             // Feedback (RS-Bus) specific code
#include "relays.h"               // For connecting two external relays 


//*****************************************************************************************************
// SETUP
//*****************************************************************************************************
unsigned int firstDecoderAddress;

void mySettings() {
  #if defined(NO_HOMING) 
    cvValues.write(StartHoming, 0);              // CV-defaults = 1
  #endif
  #if defined(NO_IR_SENSORS) 
    cvValues.write(IR_Detect, 0);                // CV-defaults = 1
  #endif
  #if defined(ENABLE_LCD)            
    cvValues.write(LCD_Display, 1);              // CV-defaults = 1
  #endif
  #if defined(SERIAL_MONITOR)
    cvValues.write(Serial_Line, SERIAL_MONITOR); // Default value = 0
  #endif
  #if defined(CV1)
    cvValues.write(myAddrL, CV1);                // Default value = 0x01
  #endif
  #if defined(CV9)
    cvValues.write(myAddrH, CV9);                // Default value = 0x80
  #endif
  #if defined(RS_ADDRESS)
    cvValues.write(myRSAddr, RS_ADDRESS);        // Default value = 0
  #endif
}


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
  cvValues.init(LiftDecoder,12);          // software version may be added as 2nd parameter. Default: 10  
  mySettings();                           // To override default settings with values from mySettings.h
  decoderHardware.init();                 // Use the CV values stored in EEPROM
  // The softare supports a maximum of 12 lift levels; each level has its own switch address.
  // For 12 switch addresses, we need to listen to 3 decoder addresses
  firstDecoderAddress = cvValues.storedAddress();
  accCmd.setMyAddress(firstDecoderAddress, firstDecoderAddress + 2);
  // Initialise the feedback system. At start-up, the values for lift.level and lift.currentPosition 
  // are zero. The level bit and STEPPER_IDLE bit will be set. The RS-Bus master will be informed.
  feedback.init(cvValues.read(myRSAddr));
  // To ensure a consistent state, do a homing cycle first. Wait 1 sec to ensure GRBL is up and running
  delay(1000);
  if (cvValues.read(StartHoming)) reset_object.home(); 
  // Display some settings
  if (cvValues.read(Serial_Line)) {
    #ifdef BOARD_SMD
      Serial.println("SMD board"); 
      #endif
    #ifdef BOARD_THT
      Serial.println("THT board"); 
      #endif
    Serial.print("myAddrL: "); 
    Serial.println(cvValues.read(myAddrL)); 
    Serial.print("myAddrH: "); 
    Serial.println(cvValues.read(myAddrH)); 
    Serial.print("First DCC decoder address:"); 
    Serial.println(firstDecoderAddress);   
    Serial.print("First RS-Bus address:"); 
    Serial.println(cvValues.read(myRSAddr)); 
    Serial.print("IR-sensors: ");
    Serial.println(cvValues.read(IR_Detect));
    Serial.println(); 
  }
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
        feedback.setLiftLevel(lift.level);
        relaysCntrl.lift_idle(lift.level);  // if at level 0, switch the relays to POS1
        if (cvValues.read(Serial_Line)) {
          Serial.print("Lift at level: ");
          Serial.println(lift.currentPosition);
        }
      }      
      else {
        feedback.clearFeedbackBits();       // to catch fast changes, such as step-up
        digitalWrite(LED_BLUE, HIGH);       // Indicate the steppers are busy 
      }
    }
    else {
      feedback.clearFeedbackBits();         // to catch normal changes
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
    feedback.irFree = ir_cntrl.sensorIsFree;
    feedback.sendMainNibble();
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
         feedback.clearFeedbackBits();      // We leave the IDLE state and the current level
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
          if (cvValues.read(Serial_Line)) {
            Serial.print("Move lift to level: ");
            Serial.println(lift.level);
          };
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
  feedback.update();
  relaysCntrl.update();
}
