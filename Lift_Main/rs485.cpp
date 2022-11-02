/*******************************************************************************************************
File:      rs485.cpp
Author:    Aiko Pras
History:   2021/04/24 AP Version 1.0


Purpose:   Implements the RS485 communication between this Master controller end the 
           Button and IR-LED controllers

******************************************************************************************************/
#include <Arduino.h>
#include <MoToTimer.h>            // For the MoToTimebase
#include <AP_RS485_Lift.h>        // My private library wrapper for Nick Gammon's non-blocking RS485 library
#include <AP_DCC_Decoder_Core.h>  // To get access to cvValues[]
#include "rs485.h"
#include "hardware.h"             // Pins for LEDs, DCC input, RS-Bus output etc.
#include "support.h"              // For toggleLed


// Instantiate some objects. 
// Since timer_50ms is used in the constructor for talk_to_controllers, it must be instatioated before
MoToTimebase          timer_50ms;            // Internal timer object for the RS485 bus
MoToTimer             timer_1000ms;          // Keep alive timer for the IR sensors board         
RS485_Lift            myRS485(MASTER_ADDR);  // Internal RS485 object, only used here
talk_to_controllers   controllers;           // External object, used by main
ir_controller         ir_cntrl;              // External object, used by main
button_controller     btn_cntrl;             // External object, used by main


//*********************************************************************************************************
talk_to_controllers::talk_to_controllers() {
  timer_50ms.setBasetime(50);                // 50ms seems reasonable to over the RS485 BUS 
  timer_1000ms.setTime(1000);                // Start the keep-alive timer for the IR board
  IrNext = false;                            // Any start value is OK
}


void talk_to_controllers::talk485() {
  // Should be called by main as often as possible
  // Part 1: Poll the IR and Button decoders
  if (timer_50ms.tick()) {
    if (IrNext) {
      // Part 1: Poll the IR LED-Sensors controller
      if (cvValues.read(IR_Detect)) myRS485.sendPoll(IR_LEDS_ADDR);
      IrNext = false;
    }
    else {
      // Part 2: Button controller
      // Check if a BUTTON_LED command is waiting, otherwise send a POLL
      if (btn_cntrl.ledActionRequested) {
        // The code below takes 700 microseconds
        myRS485.setButtonLEDs(btn_cntrl.ledAction, btn_cntrl.ledNumber);
        btn_cntrl.ledActionRequested = false;
        if ((btn_cntrl.ledAction == LED_OFF) || (btn_cntrl.ledAction == SINGLE_FLASH) || (btn_cntrl.ledAction == DELAYED_OFF)) {      
          // If we send a command to (ultimately) switch off the remote LED, turn the local LED off as well
          digitalWrite(LED_YELLOW, LOW);
         }
        }
      else {
        // The code below takes 525 microseconds
        myRS485.sendPoll(BUTTONS_ADDR);
      }
    IrNext = true;
    }
  }
  // Part 2: Check if / what input we received from the IR and Button decoders
  if (myRS485.input()) { 
    switch (myRS485.command) {
      case IR_FREE: 
      case IR_BUSY: 
        ir_cntrl.analyse_irled_response();
        timer_1000ms.restart();      // Restart the IR Board keep-alive timer 
      break;
      case BUTTON: 
        btn_cntrl.analyse_button_response();
      break;
      default:
      break;
    };
  };
  // Part 3: check the IR-Board keep-alive timer
  if (timer_1000ms.expired()) {
    if (cvValues.read(IR_Detect)) {
      digitalWrite(LED_GREEN, LOW);  // Turn the local green LED off
      ir_cntrl.sensorIsFree = false;
      ir_cntrl.sensorStateChanged = true;
    }
  }
}


//*********************************************************************************************************
ir_controller::ir_controller() {
  sensorIsFree = false;                      // Initial values should be false.
  sensorStateChanged = false;
}


void ir_controller::analyse_irled_response() { 
  if ((myRS485.command == IR_FREE) && (!sensorIsFree)) {
    digitalWrite(LED_GREEN, HIGH);       // Turn the local green LED on
    sensorIsFree = true;
    sensorStateChanged = true;
  };
  if (myRS485.command == IR_BUSY) {
    digitalWrite(LED_GREEN, LOW);  // Turn the local green LED off
    sensorIsFree = false;
    sensorStateChanged = true;
  };
}


bool ir_controller::stateChanged() { 
  bool returnValue = sensorStateChanged;
  sensorStateChanged = false;
  return returnValue;
}


//*********************************************************************************************************
// The constructor below initialises the object
button_controller::button_controller() {
  button_level_flag = false;
  button_up_down_flag = false;
  button_alarm_flag = false;
}


void button_controller::analyse_button_response() {
      buttonNumber = myRS485.value;        // Save which button was pressed / released
      buttonAction = myRS485.action;       // Was it pressed, long or short, or released?
      switch (myRS485.value) {             
        case RESET_BUTTON: 
          button_alarm_flag = true;
          break;
        case UP_BUTTON:
          button_up_down_flag = true;
          buttonUpOrDown = UP;
          break;
        case DOWN_BUTTON:
          button_up_down_flag = true;
          buttonUpOrDown = DOWN;
          break;
        default: // Button 0..10
          if (myRS485.value <= 10) {
            button_level_flag = true;
          }
          break;
      }
      if (buttonAction != RELEASED) {      // If we received a (short or long) press command
        digitalWrite(LED_YELLOW, HIGH);    // Turn the local yellow LED on, to mimic the LEDs on the remote panel
      };
}


bool button_controller::level_button_event() { 
  // Manages the button_level_flag, associated with the buttons 0..10
  bool result = false;
  if (button_level_flag) {
    result = true;
    button_level_flag = false;
  }
  return result;
}

bool button_controller::up_down_button_event() {
  // Manages the button_up_down_flag, associated with the buttons UP and DOWN
  bool result = false;
  if (button_up_down_flag) {
    result = true;
    button_up_down_flag = false;
  }
  return result;
}

bool button_controller::alarm_button_event() {
  // Manages the button_alarm_flag, associated with the ALARM button
  bool result = false;
  if (button_alarm_flag) {
    result = true;
    button_alarm_flag = false;
  }
  return result;
}


void button_controller::prepare_LED(const byte action, const byte number) {
  ledActionRequested = true;
  ledNumber = number;
  ledAction = action;
}
