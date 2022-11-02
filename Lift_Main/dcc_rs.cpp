/*******************************************************************************************************
File:      dcc_rs.cpp
Author:    Aiko Pras
History:   2022/01/05 AP Version 1.0


Purpose:   Implements the DCC and RS-Bus interface


******************************************************************************************************/
#include <Arduino.h>
#include "stepper.h"          // Needed to get direct access to the stepper state and lift position
#include "dcc_rs.h"


// Instantiate the rsbus object, which serves as RS-Bus interface to the main sketch.
// This object hides the complexity of having two RS-Bus addresses and 4 nibbles.
rsbusController   rsbus;

// Instantiate the two RS-bus objects that send feedback messages regarding the state of the lift.
// The first object uses the "base RS-Bus address", as stored in myRSAddr (CV10). 
// The second object uses an address one higher (myRSAddr + 1);
RSbusConnection rsbus1;                      // RS-Bus object for level 0..7
RSbusConnection rsbus2;                      // RS-Bus object for level 8..10, plus ready, moving etc.
uint8_t feedbackData1 = 0;                   // The feedback data for the first rsbus object (rsbus1)
uint8_t feedbackData2 = 0;                   // The feedback data for the second rsbus object (rsbus2)


void rsbusController::init(uint8_t address) {
  if ((address >= 1) && (address <= 126)) {
    rsbus1.address = address;                // Minimum value is 1
    rsbus2.address = address + 1;            // Maximum value is 127 (128 is reserved for PoM)
  } 
  irFree = false;                            // We only announce FREE if this has been checked
  liftAtLevel = false;                       // Same here
}


void rsbusController::sendMainNibble() {
  uint8_t nibble;
  nibble = (liftAtLevel << RS_STEPPER_IDLE);
  if (cvValues.read(IR_Detect)) {
    nibble |= (irFree << RS_IR_FREE);
    if (irFree && liftAtLevel) nibble |= (1 << RS_LIFT_READY);
  }
  else if (liftAtLevel) nibble |= (1 << RS_LIFT_READY);  
  rsbus2.send4bits(HighBits, nibble);
  feedbackData2 = (nibble << 4) + (feedbackData2 & 0b00001111);
}


void rsbusController::setLiftLevel(uint8_t level) {
  // Main calls setLiftLevel if the stepper state is IDLE and the level has been double checked
  uint8_t nibble = 0;
  switch (level) {
    case 0: 
    case 1:
    case 2:
    case 3:
      bitSet(nibble, level);
      rsbus1.send4bits(LowBits, nibble);
      feedbackData1 = nibble;
    break;
    case 4: 
    case 5:
    case 6:
    case 7:
      bitSet(nibble, level - 4);
      rsbus1.send4bits(HighBits, nibble);
      feedbackData1 = (nibble << 4);
    break;
    case 8: 
    case 9:
    case 10:
      bitSet(nibble, level - 8);
      rsbus2.send4bits(LowBits, nibble);
      feedbackData2 = nibble;
    break;
  }
  liftAtLevel = true;
  sendMainNibble();
}


void rsbusController::clearFeedbackBits() {
  uint8_t nibble;
  nibble = (feedbackData1 & 0b00001111);
  if (nibble != 0) rsbus1.send4bits(LowBits, 0);
  nibble = (feedbackData1 & 0b11110000);
  if (nibble != 0) rsbus1.send4bits(HighBits, 0);
  nibble = (feedbackData2 & 0b00001111);
  if (nibble != 0) rsbus2.send4bits(LowBits, 0);
  feedbackData1 = 0;
  feedbackData2 = 0;
  liftAtLevel = false;
  sendMainNibble();
}


void rsbusController::update() {
  // update is called by main at the end of every loop
  // As frequent as possible we should check if the RS-Bus asks for the most recent feedback data.
  if (rsbus1.feedbackRequested) rsbus1.send8bits(feedbackData1);
  if (rsbus2.feedbackRequested) rsbus2.send8bits(feedbackData2);
  rsbus1.checkConnection();
  rsbus2.checkConnection();
}
