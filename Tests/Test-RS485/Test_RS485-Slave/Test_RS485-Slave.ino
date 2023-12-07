//******************************************************************************************************
//
//                  Test the RS485 connections for the Lift Decoder - Slave Version
//
// purpose:   To tests the RS485 between a Master and a Slave. This sketch is for the slave!!
//            At startup the red onboard LED will be switched on.
//            Everytime a message is received, the onboard LED is changed.
//            Therefore: if the LED blinks (roughly every second), the RS485 interface is working 
// board:     Slave board  (= Button / IR sensor board). Master and slave may both be  
//            the dedicated lift decoder board, an Arduino Mega or, a combination of both
// author:    Aiko Pras
// version:   2023-11-14 V1.0 ap initial version
//
// hardware:  - For the RS485 transmit and receive: USART 3 (see AP_RS485_Lift.h)
//            - For RS485_ENABLE: PIN_PJ2 or PIN_PE4
//
// important: Use the Megacore board definition to compile and flash
//            See: https://github.com/MCUdude/MegaCore 
//
// This source file is subject of the GNU general public license 3,
// that is available at the world-wide-web at http://www.gnu.org/licenses/gpl.txt
//
//******************************************************************************************************
#include <Arduino.h>
#include <AP_RS485_Lift.h>

// Defines and objects used by this script
#define SLAVE_ADDRESS 1                 // Address of this slave
RS485_Lift myRS485(SLAVE_ADDRESS);

void setup() {
  // Turn on the internal LED, to signal we have power
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  if (myRS485.input()) { 
    myRS485.sendAck();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
