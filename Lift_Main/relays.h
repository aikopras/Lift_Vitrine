/*******************************************************************************************************
File:      relays.h
Author:    Aiko Pras
History:   2022/05/222 AP Version 1.0


Purpose:   Allows connection of two bi-stable relays. These relays can, for example, be used to:
           1) ensure that the track that connects the lift to the remaining tracks, will only be
           powered whenever the lift is at level 0. This is an additional safety measure
           2) allow a change of boosters, depending if the lift is at level 0 or at another level
           This avoids potential problems at the border between two booster sections

******************************************************************************************************/
#pragma once
#include <AP_DccRelay.h>            // Library to control a single bi-stable relay


/*****************************************************************************************************/
// The relays class allows the use of two bi-stable relays. See hardware.h to see to which pins 
// these relays should be connected to.
//
// Everytime the lift position changed, main calls the relays object. 
// In case the lift is IDLE and at level 0, both relays will be swiched to "position 1".
//   => the power of the tracks towards the lift can be swiched on, and the tracks on the lift be
//      connected to booster 1
// In all other cases both relays will be swiched to "position 2".
//    => no power on the tracks towards the lift, the tracks on the lift be connected to booster 2
// 
// Note: The Lift-decoder outputs become, once activated, low. The load should therefore be connected
// between the output and +5V. Once activated, the differential voltage becomes something like 4 Volt. 
// This might be enough for a 5 (or 3,3V) relay, but certainly not for a 12V relay. 
// Therefore the connection towards 12V relays should be performed via optocouplers,
// which "translate" between the 5V output domain, and a separate 12V domain for the relays.   


class relaysController {
  public:
    relaysController();               // Constructor for initialisation
    void lift_idle(uint8_t level);    // Called by main after the stepper state changed to IDLE
    void lift_moving();               // Called by main after the stepper state is no longer IDLE
    void update();                    // Called by main as often as possible
};


/*****************************************************************************************************/
// Definition of external objects, which are declared in relays.cpp but used by main 
extern relaysController  relaysCntrl;
