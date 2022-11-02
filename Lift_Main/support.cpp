/*******************************************************************************************************
File:      support.cpp
Author:    Aiko Pras
History:   2021/04/24 AP Version 1.0


Purpose:   All support functions and objects are here
           Note: the hardware specific LCD pin definitions are in "hardware.h"

******************************************************************************************************/
#include <Arduino.h>
#include "hardware.h"
#include "rs485.h"            // Needed to get direct access to the buttons
#include "stepper.h"          // Needed to get direct access to the stepper state and lift position
#include "dcc_rs.h"           // Needed to get access to the CV values
#include <hd44780.h>          // The default Arduino LCD Library is too slow
#include <hd44780ioClass/hd44780_pinIO.h>
#include "support.h"

// Instantiate the objects needed
hd44780_pinIO lcd(RS, RW, ENABLE, D4, D5, D6, D7);
display_class lcd_display;    // External object   
led_class     led;            // External object


//*****************************************************************************************************
//******************************************* The LED object ******************************************
//*****************************************************************************************************
led_class::led_class() {
  // Initialise the LEDs on the PCB
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);               // Standard decoder programming and RS-Bus feedback
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);                  // RS-485
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);                // Power on
  pinMode(LED_YELLOW, OUTPUT);
  digitalWrite(LED_YELLOW, LOW);                // Mimics the remote LED button (without flashes)
}

// Define a common LED function 
void led_class::toggle(uint8_t nummer) {
  digitalWrite(nummer, !digitalRead(nummer));
}


//*****************************************************************************************************
//***************************************** The display object ****************************************
//*****************************************************************************************************
// Create two new characters: Arrow up and Arrow down
byte ArrowUp[8] = {
0b00000,
0b00100,
0b01110,
0b11111,
0b00100,
0b00100,
0b00100,
0b00100
};

byte ArrowDown[8] = {
0b00000,
0b00100,
0b00100,
0b00100,
0b00100,
0b11111,
0b01110,
0b00100
};


void display_class::init() {
  // Initialise the LCD display. 16 charactors and 2 rows
  lcd.begin(16,2);
  lcd.print("Liftdecoder");
  lcd.setCursor(0, 1);
  lcd.print("Version 1.0");
  //
  lcd.createChar(0, ArrowUp);
  lcd.createChar(1, ArrowDown);
}


void display_class::show() {
  // The LCD library is extremely slow; one call to show() takes over 5 ms.
  // It is possible to improve performance, by sending only a single character
  // per main-loop cycle.
  // Therefore the LCD should preferabley only be used for debugging! 
  // Usage of the LCD display can be enabled / disabled via the CV `LCD_Display`
  if (cvValues.read(LCD_Display)) {
    lcd.clear();                                      // takes 140us
    switch (btn_cntrl.buttonNumber) {
      case 12: lcd.print("UP");    break;
      case 13: lcd.print("Down");  break;
      default:
        lcd.print("Level: ");                         // takes 2930us
        lcd.print(lift.level);                        // takes 200us
      break;
    }
    lcd.setCursor(0, 1);
    if (stepper.state == grbl::ALARM) lcd.print("Alarm");
    if (stepper.state == grbl::RUN) {
      lcd.print("Moving: ");
      lcd.print(lift.currentPosition);
    }
    if (stepper.state == grbl::IDLE) {
      lcd.print("Idle: ");
      lcd.print(lift.currentPosition);
    }
    if (stepper.state == grbl::JOG) {
      if (btn_cntrl.buttonUpOrDown == btn_cntrl.UP) {lcd.write(byte(0));}
        else {lcd.write(byte(1));}
      lcd.setCursor(2, 1);
      lcd.print("Jog: ");
      lcd.print(lift.currentPosition);
    }
  }
}


void display_class::homing() {
  if (cvValues.read(LCD_Display)) {
    lcd.clear(); 
    lcd.print("Homing started");             
  }
}
