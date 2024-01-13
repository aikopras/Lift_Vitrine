/*******************************************************************************************************
File:      relays.cpp
Author:    Aiko Pras
History:   2022/05/22 AP Version 1.0
           2024/01/11 AP version 1.1: relay pins are defined in mySettings

Purpose:   Implements the objects to control external relaays

******************************************************************************************************/
#include <Arduino.h>
#include "relays.h"
#include "mySettings.h"

#define HOLD_TIME 3                  // 3 * 20ms = 60ms

// Instantiate some objects. 
relaysController  relaysCntrl;       // Used by the main sketch
relayClass relay1;                   // First relay
relayClass relay2;                   // Second relay


//*****************************************************************************************************
relaysController::relaysController() {
  // constructor
  // Initialise each relay and specify the pins the coils for POS1 and POS 2 are connected to
  relay1.init(RELAY1_POS1, RELAY1_POS2, HOLD_TIME);
  relay2.init(RELAY2_POS1, RELAY2_POS2, HOLD_TIME); 
}


void relaysController::lift_idle(uint8_t level) {
  // The stepper motors have become IDLE. Check at which level we are
  if (level == 0) {
    // both relays should now be switched to position 1
    relay1.activate(relayClass::POS1);
    relay2.activate(relayClass::POS1);
  }
  else {
    relay1.activate(relayClass::POS2);
    relay2.activate(relayClass::POS2);    
  }
}


void relaysController::lift_moving() {
  // The lift is moving, so we can not be at level 0
  // both relays should now be switched to position 2
  relay1.activate(relayClass::POS2);
  relay2.activate(relayClass::POS2);  
}


void relaysController::update() {
  relay1.update();
  relay2.update();
}
