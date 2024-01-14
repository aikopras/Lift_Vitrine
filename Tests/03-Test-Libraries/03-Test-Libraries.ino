// Test-sketch for the Lift decoder boards.
//
// The goal of this test sketch is to check if the required libraries are installed 
// For the test to be succesfull, no compile errors may be raised, upload should work
// and all LEDs should blink, thus the red (onboard / DCC), blue, green and yellow LED.

// *********************************************************************************************
// Before compilation, make sure you have installed the following board definition and libraries
// - MegaCore board: https://github.com/MCUdude/MegaCore#boards-manager-installation
// - AP_DCC_Decoder_Core library: https://github.com/aikopras/AP_DCC_Decoder_Core
// - AP_DCC_library: https://github.com/aikopras/AP_DCC_library
// - RSbus library: https://github.com/aikopras/RSbus
// - AP_RS485_Lift linrary: https://github.com/aikopras/AP_RS485_for_Lift_decoders
// - MobaTools: https://github.com/MicroBahner/MobaTools
//
// For compiling via the Arduino IDE, see the file Step02-Install-MegaCore.md
//
// *********************************************************************************************
#include <AP_DCC_Decoder_Core.h>

#define LED_BLUE       PIN_PD4 
#define LED_GREEN      PIN_PD5 
#define LED_YELLOW     PIN_PD6 
#define LED_BLUE2      PIN_PE3 
#define LED_GREEN2     PIN_PG5 
#define LED_YELLOW2    PIN_PE5 



void setup() {
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE2, OUTPUT);
  pinMode(LED_GREEN2, OUTPUT);
  pinMode(LED_YELLOW2, OUTPUT);
  // The onboard / DCCC LED (ledPin) is already initialised in the AP_DCC_Decoder_Core library
}


void loop() {
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_YELLOW, HIGH);
  digitalWrite(LED_BLUE2, HIGH);
  digitalWrite(LED_GREEN2, HIGH);
  digitalWrite(LED_YELLOW2, HIGH);
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_BLUE2, LOW);
  digitalWrite(LED_GREEN2, LOW);
  digitalWrite(LED_YELLOW2, LOW);
  digitalWrite(ledPin, LOW); 
  delay(1000);
}
