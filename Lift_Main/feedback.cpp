/*******************************************************************************************************
File:      feedback.cpp
Author:    Aiko Pras
History:   2022/01/05 AP Version 1.0
           2024/01/11 AP Version 1.5 - Feedback via blue board connectors added


Purpose:   Implements feedback via the RS-Bus interface and the onboard (blue) connectors

******************************************************************************************************/
#include <Arduino.h>
#include "stepper.h"          // Needed to get direct access to the stepper state and lift position
#include "feedback.h"


// Instantiate the feedback object
// This object allows the main sketch to send RS-Bus messages and set the onboard feedback pins
// This object hides the complexity of having two RS-Bus addresses and 4 nibbles.
feedbackController   feedback;


// Instantiate the two RS-bus objects that send feedback messages regarding the state of the lift.
// The first object uses the "base RS-Bus address", as stored in myRSAddr (CV10). 
// The second object uses an address one higher (myRSAddr + 1);
RSbusConnection rsbus1;                      // RS-Bus object for level 0..7
RSbusConnection rsbus2;                      // RS-Bus object for level 8..10, plus ready, moving etc.
uint8_t feedbackData1 = 0;                   // The feedback data for the first rsbus object (rsbus1)
uint8_t feedbackData2 = 0;                   // The feedback data for the second rsbus object (rsbus2)


void feedbackController::init(uint8_t address) {
  if ((address >= 1) && (address <= 126)) {
    rsbus1.address = address;                // Minimum value is 1
    rsbus2.address = address + 1;            // Maximum value is 127 (128 is reserved for PoM)
  } 
  irFree = false;                            // We only announce FREE if this has been checked
  liftAtLevel = false;                       // Same here
  // We manupulate the feedback ports directly, since that is trivial
  DDRC = 0xFF;     // PORTC: All outputs
  DDRL = 0xFF;     // PORTL: All outputs
  DDRF = 0xFF;     // PORTF: All outputs
  // Make sure all the feedback ports are switched off
  PORTC = 0;
  PORTL = 0;
  PORTF = 0;
}


void feedbackController::sendMainNibble() {
  uint8_t nibble;
  nibble = (liftAtLevel << RS_STEPPER_IDLE);
  if (cvValues.read(IR_Detect)) {
    nibble |= (irFree << RS_IR_FREE);
    if (irFree && liftAtLevel) nibble |= (1 << RS_LIFT_READY);
  }
  else if (liftAtLevel) nibble |= (1 << RS_LIFT_READY);  
  rsbus2.send4bits(HighBits, nibble);
  feedbackData2 = (nibble << 4) + (feedbackData2 & 0b00001111);
  PORTC = feedbackData2;
  PORTF = (feedbackData2 >> 7);  // Only the Lift Ready bit
}


void feedbackController::setLiftLevel(uint8_t level) {
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
  PORTL = feedbackData1;
  PORTC = feedbackData2;
  sendMainNibble();
}


void feedbackController::clearFeedbackBits() {
  uint8_t nibble;
  nibble = (feedbackData1 & 0b00001111);
  if (nibble != 0) rsbus1.send4bits(LowBits, 0);
  nibble = (feedbackData1 & 0b11110000);
  if (nibble != 0) rsbus1.send4bits(HighBits, 0);
  nibble = (feedbackData2 & 0b00001111);
  if (nibble != 0) rsbus2.send4bits(LowBits, 0);
  feedbackData1 = 0;
  feedbackData2 = 0;
  PORTL = 0;
  PORTC = 0;
  PORTF = 0;
  liftAtLevel = false;
  sendMainNibble();
}


void feedbackController::update() {
  // update is called by main at the end of every loop
  // As frequent as possible we should check if the RS-Bus asks for the most recent feedback data.
  if (rsbus1.feedbackRequested) rsbus1.send8bits(feedbackData1);
  if (rsbus2.feedbackRequested) rsbus2.send8bits(feedbackData2);
  rsbus1.checkConnection();
  rsbus2.checkConnection();
}
