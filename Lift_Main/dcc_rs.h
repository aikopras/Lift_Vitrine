/*******************************************************************************************************
File:      dcc_rs.h
Author:    Aiko Pras
History:   2022/01/05 AP Version 1.0


Purpose:   Implements the DCC and RS-Bus interface
           All DCC and RS-Bus pins are defined and initialised in the AP_DCC_Decoder_Basic library

RS-Bus:
- Base address    : Level 0..7                (low and high nibble. Bit 0..7)
- Base address + 1: Level 8..10               (low nibble.          Bit 0..2)
- Base address + 1: Lift OK: IDLE at level x  (high nibble          Bit 7)
- Base address + 1: IR blocked                (high nibble          Bit 6)
- Base address + 1: Emergency stop            (high nibble          Bit 5)

******************************************************************************************************/
#pragma once
#include <AP_DCC_Decoder_Core.h>      // Library for a basic DCC accesory decoder with RS-Bus feedback


#define RS_IR_FREE      0             // Bit number 0 of second nibble (HighBits)    
#define RS_STEPPER_IDLE 1             // Bit number 1 of second nibble (HighBits)    
#define RS_LIFT_READY   3             // Bit number 3 of second nibble (HighBits)    


//******************************************** RS-BUS CONTROLLER **************************************
// The RS-BUS controller sets the appropriate feedback bits of the two RS-Bus channels being used. 
class rsbusController {
  public:
    void init(uint8_t address);       // Sets the two RS-Bus addresses
    void sendMainNibble();            // Send the bits for IR free, stepper idle and lift ready
    void setLiftLevel(uint8_t level); // Set the RS-Bus bits that corresponds to the current level 
    void clearFeedbackBits();         // Clear all RS-Bus bit corresponding to the lift level and state
    void update();                    // Called at the end of the Main loop as frequent as possible
    bool irFree;                      // To indicate if the IR sensors are free or occupied 
    bool liftAtLevel;                 // To indicate if the lift arrived at the expected level 
};

//*****************************************************************************************************
// Definition of external objects, which are declared in dcc_rs.cpp but used by main 
extern rsbusController   rsbus;

extern RSbusConnection rsbus1;                // RS-Bus object for level 0..7
extern RSbusConnection rsbus2;                // RS-Bus object for level 8..10, plus ready, moving etc.
extern uint8_t feedbackData1;                 // The feedback data for the first rsbus object (rsbus1)
extern uint8_t feedbackData2;                 // The feedback data for the second rsbus object (rsbus2)
