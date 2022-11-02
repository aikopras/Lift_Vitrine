/*******************************************************************************************************
File:      rs485.h
Author:    Aiko Pras
History:   2021/04/24 AP Version 1.0


Purpose:   Implements the RS485 communication between this Master controller end the 
           Button and IR-LED controllers

******************************************************************************************************/
#pragma once
#include <AP_RS485_Lift.h>         // Needed since we use several constants from there in main


#define RESET_BUTTON   11
#define UP_BUTTON      12
#define DOWN_BUTTON    13


//****************************************** SEND COMMANDS ********************************************
// An instance of the talk_to_controllers class should be called from main as often as possible.
// It sends commands (POLL, BUTTON_LED or IR_REQUEST) to the button and IR-LED controllers
// To avoid collisions on the RS485 bus, such commands are only send every 50ms
class talk_to_controllers {
  public:
    talk_to_controllers();            // Constructor for initialisation
    void talk485();                   // Schould be called from the main loop as often as possible
  private:
    bool IrNext;                      // Determines if the next RS485 message goes to IR or Button   
};


//************************************ ANALYSE IR-LED INPUT RECEIVED **********************************
class ir_controller {
  public:
    ir_controller();                  // Constructor for initialisation
    bool sensorIsFree;                // True if all IR-receivers receive light from their IR-sender 
    void analyse_irled_response();    // Called if a RS-485 message is received and sets sensorIsFree
    bool stateChanged();              // Function, called from main
    bool sensorStateChanged;          // Variable (to keep state)
};


//************************************ ANALYSE BUTTON INPUT RECEIVED **********************************
// The button_changed method of a button_controller should be called from main as often as possible.
// If a button message is received, the buttonNumber and buttonAction provide further information 
class button_controller {
  
  public:
    // Constructor for initialisation
    button_controller(); 

    // Types
    typedef enum {UP, DOWN} up_down_t; // For the UP and DOWN buttons

    // Attributes set by prepare_LED, and used by talk_to_controllers for sending the actual LED command
    bool    ledActionRequested;       // true, if a BUTTON_LED command should be send in the next cycle
    uint8_t ledNumber;                // Which LED should be modified in the new cycle?
    uint8_t ledAction;                // LED_OFF, LED_ON, SINGLE_FLASH, FLASH_SLOW, FLASH_FAST, DELAYED_OFF

    // Attributes for main regarding a received button command:
    uint8_t buttonNumber;             // Which button was pressed? (0..15)
    uint8_t buttonAction;             // PRESSED, SHORTPRESS, LONGPRESS, RELEASED   
    up_down_t buttonUpOrDown;         // For the UP and DOWN buttons  
    
    // Methods that should be called by main as often as possible
    void analyse_button_response();   // Checks if a button command is received and if yes, for which button
    bool level_button_event();        // Manages the button_level_flag, associated with the buttons 0..10
    bool up_down_button_event();      // Manages the button_up_down_flag, associated with the buttons UP and DOWN
    bool alarm_button_event();        // Manages the button_alarm_flag, associated with the ALARM button

    
    void prepare_LED(const byte action, const byte number);
                                      // Can be called by the main program, or from this class
                                      // Sets the private attributes ledActionRequested, ledNumber and ledAction
  private:
    bool button_level_flag;           // Set by analyse_button_response(), cleared by level_button_event()
    bool button_up_down_flag;         // Set by analyse_button_response(), cleared by up_down_button_event()
    bool button_alarm_flag;           // Set by analyse_button_response(), cleared by alarm_button_event()

};


//*********************************************************************************************************
// Definition of external objects, which are defined here but used by main 
extern talk_to_controllers   controllers;
extern ir_controller         ir_cntrl;
extern button_controller     btn_cntrl;
