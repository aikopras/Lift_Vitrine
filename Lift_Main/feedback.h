/*******************************************************************************************************
File:      feedback.h
Author:    Aiko Pras
History:   2022/01/05 AP Version 1.0
           2024/01/11 AP Version 2.0 - Feedback via blue board connectors added


Purpose:   Implements RS-Bus interface
           The RS-Bus hardware is defined and initialised in the AP_DCC_Decoder_Basic library
           The main Lift sketch the meaning and location of the various feedback bits.

******************************************************************************************************/
#pragma once
#include <AP_DCC_Decoder_Core.h>      // Library for a basic DCC accesory decoder with RS-Bus feedback


#define RS_IR_FREE      0             // Bit number 0 of second nibble (HighBits)    
#define RS_STEPPER_IDLE 1             // Bit number 1 of second nibble (HighBits)    
#define RS_LIFT_READY   3             // Bit number 3 of second nibble (HighBits)    


//******************************************** RS-BUS CONTROLLER **************************************
// The RS-BUS controller sets the appropriate feedback bits of the two RS-Bus channels being used. 
class feedbackController {
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
// Definition of external objects, which are declared in feedback.cpp but used by main 
extern feedbackController   feedback;

extern RSbusConnection rsbus1;                // RS-Bus object for level 0..7
extern RSbusConnection rsbus2;                // RS-Bus object for level 8..10, plus ready, moving etc.
extern uint8_t feedbackData1;                 // feedback data for rsbus1 / IN 1..8
extern uint8_t feedbackData2;                 // feedback data for rsbus2 / IN 9..14 / OUT 1..3
