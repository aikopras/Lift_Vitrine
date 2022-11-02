// ***********************************************************************************************************
// 
// purpose:   Control of an IR lock (LED plus sensor)
// File:      Lift_IR.ino
// Author:    Aiko Pras
// History:   2022/01/25 AP Version 1.0
//
// Sketch for the IR decoder.
// 
// The code is developed for Arduino 2560 microcontrollers and has been tested on the dedicated 
// lift controller board: https://easyeda.com/aikopras/support-lift-controller
// Since this boards uses some AVR pins (RS485 Enable, Blue, Green and Yellow LEDs) that are not available  
// on standard Arduino MEGA boards, for compiling via the Arduino IDE use the MEGACORE - ATmega2560 board 
// See: https://github.com/MCUdude/MegaCore for installation and usage
// 
// The following settings are used (under the Tools section of the Arduino IDE):
// - Board: ATmega2560             - Important to set this right
// - Clock: External 16Mhz         - Important to set this right
// - BOD: 2,7V                     - Seems a save value
// - EEPROM: Retained              - May be this setting will change in the future
// - Compiler: LTO enabled         - No specific reason for this choice, however
// - Pinout: Arduino MEGA Pinout   - Important to set this right
// - Bootloader: No                - Important - See below  
// - Port: usbserial....           - Depends on the specifics of your development system
// - Programmer: USBASP (Megacore) - Depends on the specifics of your development system
// 
// Bootloader: although the board makes UART0 available as monitoring port, which can be connected
// to the development system via a serial to usb adapter, in general we won't be able to use such adapter 
// to upload new code. The main reason is that a bootloader requires a RESET before it will start 
// uploading new code. For this to happen the DTR signal of a serial adapter should be connected 
// to the boards reset pin. Most adapters don't export the DTR signal. 
// 
// Setting the Fuses
// Before the board can be used first, the fuses need to be set. There are two options for that:
// 1) Make sure that "Bootloader: No" is selected. After that select Tools => Burn Bootloader 
//    This will set the fuse bits, but will not upload a bootloader
// 2) Use the development system's command line to issue the following command (assuming usbasp):
// avrdude -c usbasp -p m2560 -U lfuse:w:0xEE:m -U hfuse:w:0xD9:m -U efuse:w:0xFE:m 
// 
// ***********************************************************************************************************
#include <AP_RS485_Lift.h> 
#include "hardware.h" 
#include "IR-Lock.h"

// The following #define tells which Inputs and Outputs of the Liftdecoder board are beining used.
// The lowest order bit corresponds with the blue connectors labelled 1
// #define LOCKS_CONNECTED 0b0000000000000001
#define LOCKS_CONNECTED 0b0000111111111111


// ***********************************************************************************************************
RS485_Lift myRS485(IR_LEDS_ADDR);  // Instantiate the myRS485 object
IR_Locks irLocks;                  // Instantiate the irLocks object


void setup() {
  // Initialise the LEDs on the board
  // The RED builtin LED is used like all standard decoders
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  // The green LED indicates the board has started 
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  // The blue LED indicates RS485 bus activity
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  // The yellow LED is used to indicate that one or more sensor are occupied 
  pinMode(LED_YELLOW, OUTPUT);
  digitalWrite(LED_YELLOW, LOW);
  // The timers are used to create the IR pulse train
  irLocks.init_timers();
   // For debugging a monitor may be connected via serial (UART0)
  Serial.begin(115200);
  delay(100); 
  Serial.println();
  Serial.println("Start IR controller");
  
}


void toggleLed (byte nummer) {
  digitalWrite(nummer, !digitalRead(nummer));
}


// ***********************************************************************************************************
void loop() {
  // We continuously check the IR-Sensors, and return their values if requested by the master station
  // Every 100ms we receive a POLL message from the master controller.
  // After reception we poll all sensors, and send a REPLY message back. 
  // Total time between reception of the POLL and transmission of the REPLAY is less than 10ms.
  if (myRS485.input()) { 
    toggleLed(LED_BLUE);
    // STEP 1: Check all IR locks, also those not connected
    for (uint8_t i=0; i < (MAX_LOCKS); i++) {
      irLocks.checkStatus(i);       
    }
    // STEP 2: Determine the result value
    bool result = irLocks.feedbackBit(LOCKS_CONNECTED);
    // Send the result value
    if ((result)) myRS485.sendIrSensorsFree();
      else myRS485.sendIrSensorsBusy();
    // The Green LED goes off if one or more locks are occupied
    if (result) digitalWrite(LED_GREEN, 1);
      else digitalWrite(LED_GREEN, 0);
  }
}
