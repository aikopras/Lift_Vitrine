/*******************************************************************************************************
File:      Lift_Buttons.ino
Author:    Aiko Pras
History:   2021/04/24 AP Version 1.0
           2024/01/12 AP V1.1: mySettings added / Tested on THT board.

Purpose:   Sketch for the panel with buttons that operate the loclift
           Whenever a button is pushed, the sketch sends a RS485 message to the master controller,
           indicating the kind of push-action performed (pressed, short-press, long-press or released).
           It listens to commands from the master controller before making changes to the LED's status.
           What exactly happens after a button is pushed, is not decided in this script, but 
           determined by the master controller.
******************************************************************************************************/ 
// The code is has been tested on the following lift controller boards 
// - SMD board: https://oshwlab.com/aikopras/support-lift-controller
// - THT board: https://oshwlab.com/aikopras/lift-decoder-arduino-mega-tht
//
// Instructions for compiling, see
// - SMD board: https://github.com/aikopras/Lift_Vitrine/blob/main/extras/Board-SMD/Compile.md
// - THT board: https://github.com/aikopras/Lift_Vitrine/blob/main/extras/Board-THT/Compile.md
//
// LEDs:
// - Blue:Indicates reception of RS485 message from the main decoder board  Should blink 10x per second.
// - Green: The green LED indicates the board has started
// - Yellow: The yellow LED is set immeditely after a button is pressed and switched off after
//   a RS485 message is received that says LEDs should be turned off
// - Red: Not used.
// ****************************************************************************************************** 
#include <AP_RS485_Lift.h> 
#include "hardware.h" 

// Instantiate the myRS485 object
RS485_Lift myRS485(BUTTONS_ADDR);

// Declaration of everything that belongs to the LEDS
#include "LEDs.h"
const byte LedPinNr[]   = { 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67 };
const byte numberOfLeds = sizeof(LedPinNr);
Led_series MyLeds(LedPinNr, numberOfLeds);


// Declaration of everything that belongs to the buttons
// The button pins are connected to the Mega 2560 Ports L0..7 and C0..5
// On the Arduino Mega these are pins 49..42 and 37..32
#include <MoToButtons.h>
const byte buttonPinNr []  = { 49, 48, 47, 46, 45, 44, 43, 42, 37, 36, 35, 34, 33, 32 };
const byte numberOfButtons = sizeof(buttonPinNr);
MoToButtons myButtons(buttonPinNr, numberOfButtons, 50, 3000 );


void setup() {
  // Initialise the LEDs on the PCB
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  pinMode(LED_YELLOW, OUTPUT);
  digitalWrite(LED_YELLOW, LOW);
  // For debugging a monitor can be connected via serial (UART0)
  Serial.begin(115200);
  delay(1000); 
  Serial.println("Start Button controller");
}


void toggleLed (byte nummer) {
  digitalWrite(nummer, !digitalRead(nummer));
}


// ***********************************************************************************************************
void loop() {
  // The loop is only activated after we receive a message from the master controller
  // Every 50ms we should receive a POLL message. 
  if (myRS485.input()) { 
    toggleLed(LED_BLUE);
    // Step 1: Check if we need to change a LED
    if (myRS485.command == BUTTON_LED) {
      byte ledNumber = myRS485.value;
      // Check the required action
      switch (myRS485.action) {             
      case LED_OFF:           
        MyLeds.turn_off(ledNumber);
        digitalWrite(LED_YELLOW, LOW);
        break;
      case LED_ON: 
        MyLeds.turn_on(ledNumber);
        break;
      case SINGLE_FLASH: 
        MyLeds.flash(ledNumber);
        break;
      case FLASH_SLOW: 
        MyLeds.flashSlow(ledNumber);
        digitalWrite(LED_YELLOW, HIGH);
        break;
      case FLASH_FAST: 
        MyLeds.flashFast(ledNumber);
        break;      
      case DELAYED_OFF:    // ON, and after some delay OFF
        MyLeds.flash(ledNumber, 20);
        digitalWrite(LED_YELLOW, LOW);
        break;
      }      
    }
    // Step 2: Read the status of all buttons, and respond in case of a button action 
    myButtons.processButtons();
    // Analyse each individual button
    for (byte i = 0; i < numberOfButtons; i++) {
      if (myButtons.pressed(i)) {
        myRS485.sendButtons(PRESSED, i);
        digitalWrite(LED_YELLOW, HIGH);
        }
      else if (myButtons.shortPress(i)) {
        myRS485.sendButtons(SHORTPRESS, i);
        }
      else if (myButtons.longPress(i)) {
        myRS485.sendButtons(LONGPRESS, i);
        }
      else if (myButtons.released(i)) {
        myRS485.sendButtons(RELEASED, i);
        }
      }  // end for loop
    // Call as frequent as possible the LED updater  
    MyLeds.update();
  }
}
