//******************************************************************************************************
//
//                   Test the RS485 connections for the Lift Decoder - Master Version
//
// purpose:   To tests the RS485 between a Master and a Slave. This sketch is for the master!!
//            At startup the red onboard LED will be switched on.
//            Every second a RS485 message is send to the slave and the onboard LED is switched off.
//            If the return message from the slave is received, the LED is switched on again.
//            Therefore: if the LED stays on, the RS485 interface is working 
// board:     Master board. Master and slave may both be the dedicated lift decoder board, 
//            an Arduino Mega or, a combination of both
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
#include <AP_RS485_Lift.h>        // This library


// Defines and objects used by this script
#define ADDRESS 1                 // RS485 destination address
RS485_Lift myRS485(MASTER_ADDR);
unsigned long currentMillis;
unsigned long previousMillis;
unsigned long interval = 1000;


//******************************************************************************************************
//                                                  Setup
//******************************************************************************************************
void setup() {
  // Turn on the internal red LED, to signal we have power
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  previousMillis = millis();
}


//******************************************************************************************************
//                                               Main loop
//******************************************************************************************************
void loop() {
  // We send one command per second
  currentMillis = millis();
  if(currentMillis - previousMillis > interval)
  { 
    previousMillis = currentMillis;
    digitalWrite(LED_BUILTIN, LOW);
    myRS485.sendPoll(ADDRESS);
  }
  if (myRS485.input()) { 
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
