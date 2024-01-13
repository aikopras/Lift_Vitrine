// ***********************************************************************************************************
// 
// purpose:   Control of the Loklift IR sensors
// File:      Lift_IR.ino
// Author:    Aiko Pras
// History:   2022/01/25 AP Version 1.0
//            2024/01/06 AP Version 2.0 - Added RSBus feedback, debugging via the Serial interface and LEDs.
//                                        Before compilation, make sure you've set the variables and #defines
//                                        contained in the file mySettings
// 
// The code is has been tested on the following lift controller boards 
// - SMD board: https://oshwlab.com/aikopras/support-lift-controller
// - THT board: https://oshwlab.com/aikopras/lift-decoder-arduino-mega-tht
//
// Instructions for compiling, see
// - SMD board: https://github.com/aikopras/Lift_Vitrine/blob/main/extras/Board-SMD/Compile.md
// - THT board: https://github.com/aikopras/Lift_Vitrine/blob/main/extras/Board-THT/Compile.md
//
// LEDs:
// - Blue:Indicates reception of RS485 message from the main decoder board  Should blinks 10x per second.
// - Green: Indicates if the IR-Sensors are free (no train blocks the IR-beam). On = free
// - Yellow: Decoder is in debugging mode
// - Red: Blinks whenever a RSbus feedback message is send. Also twice at startup.
//
// Button: if pushed, debugging mode is turned on/off. 
// In debugging mde, status information of each individual sensor will be send over the Serial Monitor. 
//
// ***********************************************************************************************************
//                                           Do not edit below this line
// ***********************************************************************************************************
#include <AP_DCC_Decoder_Core.h>      // Library for a basic DCC accesory decoder with RS-Bus feedback
#include <AP_RS485_Lift.h>            // Library to communicate with the main lift decoder
#include "mySettings.h" 
#include "hardware.h" 
#include "IR-Sensor.h"

RS485_Lift myRS485(IR_LEDS_ADDR);    // Instantiate the myRS485 object

IR_Sensors irSensors;                // Instantiate the irSensors object
uint16_t sensorValuesNew = 0;        // Set every 100ms by IR_Sensors::feedbackBit
uint16_t sensorValues = 0;           // Old values

DccLed redLed;                       // LED that signals transmission of RSBus messages
DccButton button;                    // Push to enter debug mode (use Serial interface for sensor feedback) 
bool debugFlag;                      // Determines if we are in debug mode or not

extern RSbusHardware rsbusHardware;  // This object is defined and instantiated in the RSbus library
RSbusConnection rsbusFirst;          // Object that represents the sensors 1..8
RSbusConnection rsbusSecond;         // Object that represents the sensors 9..16
unsigned long TLast;                 // Support variable for RSBus transmission


// ***********************************************************************************************************
void setup() {
  // The green LED indicates that all sensors are free 
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  // The blue LED indicates RS485 bus activity
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  // The yellow LED indicates debugging mode (status info is send via the Serial Interface)
  pinMode(LED_YELLOW, OUTPUT);
  digitalWrite(LED_YELLOW, LOW);
  //
  // The timers (4 and 5) are used to create the IR beam
  irSensors.init_timers();
  //
  // RSbus specific initialisation
  rsbusHardware.attach(rsBusUsart, rsBusRX);
  rsbusFirst.address = RS_AddresLow;
  rsbusSecond.address = RS_AddresHigh;
  redLed.attach(ledPin);
  TLast = millis();
  //
  // Debugmode specific initialisation
  button.attach(buttonPin);
  delay(500);                 // some time to allow the hardware to initialise
  button.read();              // Initial read is needed, to start in normal (not debug) mode 
  debugFlag = 0;
  //
  redLed.start_up();          // Blink twice to indicate startup
}


// *** RSBus ***
void PrepareRSBusFeedback() {
  // Stores in a buffer RSBus feedback messages for sensor values that have been changed
  if (rsbusHardware.rsSignalIsOK) {
    if ((sensorValues & 0x000F) != (sensorValuesNew & 0x000F)) rsbusFirst.send4bits(LowBits,    sensorValuesNew & 0x000F);
    if ((sensorValues & 0x00F0) != (sensorValuesNew & 0x00F0)) rsbusFirst.send4bits(HighBits,  (sensorValuesNew & 0x00F0) >> 4);
    if ((sensorValues & 0x0F00) != (sensorValuesNew & 0x0F00)) rsbusSecond.send4bits(LowBits,  (sensorValuesNew & 0x0F00) >> 8);
    if ((sensorValues & 0xF000) != (sensorValuesNew & 0xF000)) rsbusSecond.send4bits(HighBits, (sensorValuesNew & 0xF000) >> 12);
    if (sensorValues != sensorValuesNew) redLed.feedback();
  }
}


void SendRSBusFeedback() {
  // Listen to RS-Bus polling messages for our turn
  rsbusHardware.checkPolling();
  // The RSBus can not send faster than once per 30 ms. Therefore we check only once per 20ms
  unsigned long TNow = millis();            // millis() is expensive, so call it only once
  if ((TNow - TLast) >= 20) {               // 20ms passed?
    TLast = TNow; 
    // If a RS-Bus feedback decoder starts, or after certain errors, it needs to send its feedback data to the master station
    if (rsbusFirst.feedbackRequested)  rsbusFirst.send8bits(0);
    if (rsbusSecond.feedbackRequested) rsbusSecond.send8bits(0);
    // Check if the RSBus buffer contains feedback messages, and give these to the ISR and USART for actual transmission
    rsbusFirst.checkConnection();
    rsbusSecond.checkConnection();
  }
}


// *** LEDs ***
void setLed(byte nummer) { digitalWrite(nummer, HIGH); }
void clearLed(byte nummer) { digitalWrite(nummer, LOW); }
void toggleLed(byte nummer) { digitalWrite(nummer, !digitalRead(nummer)); }


// *** Debugging ***
void IniSerial() {
  // For debugging a monitor may be connected via serial (UART0)
  Serial.begin(115200);
  delay(1000); 
  Serial.println();
  Serial.println("IR controller sensor info");
  Serial.print("- IR frequency: ");
  Serial.print(KHZ);
  Serial.println(" KHZ");
  Serial.print("- IR burst time: ");
  Serial.print(BURST_TIME);
  Serial.println(" microseconds");
  Serial.print("- RSBus addresses: ");
  Serial.print(RS_AddresLow);
  Serial.print(" & ");
  Serial.println(RS_AddresHigh);
  Serial.println();
  Serial.println("Sensor values (in HEX format - Free = 0):");
}


void debugMode() {
  // Step 1: Do we need to send any info via the Serial interface?
  if (debugFlag) {
    if (sensorValues != sensorValuesNew) 
      Serial.printf("%x\n", sensorValuesNew);
  }
  // Step 2: if button was pushed, toggle LED and debugFlag
  button.read();
  if (button.wasReleased()) {
    toggleLed(LED_YELLOW);
    if (debugFlag) {
      debugFlag = 0;
    }
    else {
      debugFlag = 1;
      IniSerial();
    }
  }
}
 

// ***********************************************************************************************************
void loop() {
  // Every 100ms we should receive a POLL message from the master controller.
  // After reception, we poll all sensors. 
  // To the master controller we reply with a RS485 message containing a single bit, indicating if all sensors
  // are free or not. Total time between reception of the POLL and transmission of the REPLAY is less than 10ms.
  // If the value of any sensors changed, a RSBus feedback message is send to tell which sensor changed.
  if (myRS485.input()) {    
    toggleLed(LED_BLUE);
    // STEP 1: Check all IR sensors, also those not connected
    for (uint8_t i=0; i < (MAX_SENSORS); i++) {
      irSensors.checkStatus(i);       
    }
    // STEP 2: Determine the result value
    // A mask is used, to ensure we only check the sensors that are connected
    bool result = irSensors.feedbackBit(MASK_SENSORS_CONNECTED);
    // STEP 3: Send the result value via the RS485 bus and set the green LED
    if ((result)) {
      myRS485.sendIrSensorsFree();
      setLed(LED_GREEN);
    }
      else {
        myRS485.sendIrSensorsBusy();
        clearLed(LED_GREEN);
      }
    // STEP 4: Any changes that need to be send via the RSBus?
    PrepareRSBusFeedback();
    // STEP 5: Check debug button / send sensor info via serial interface
    debugMode();  
    // STEP 6: update sensorValues
    sensorValues = sensorValuesNew;
  }
  // STEP 7: check as often as possible RSBus activity and LED
  SendRSBusFeedback();
  redLed.update();
}
