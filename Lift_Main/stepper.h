/*******************************************************************************************************
File:      stepper.h
Author:    Aiko Pras
History:   2021/04/24 AP Version 1.0


Purpose:   Implements the communication with the GRBL (stepper motor) controller, and keeps track 
           of the position and state of the lift.
           The GRBL configuration variables, which are stored in the 328 processor, can (only) be
           changed using the serial monitor (in step 1 of the main program).

******************************************************************************************************/
#pragma once
#include <MoToTimer.h>      // For the MoToTimebase


/*****************************************************************************************************/
// The lift class keeps track of the current position (in mm) of the lift, what positions it can
// move to, and offers a command to perform the actual move.
// A lift decoder board has 14 outputs, meaning that, in theory, we may support upto 14 lift levels.
// However; the feedback messages that indicate which level the lift currently is, have "only" 12 bits
// available for this. Therefore we "limit" the lift to 12 levels.
// Level 0 is used for arriving and departing trains. The levels 1..11 can be used to store trains. 
// The precise position (in mm) for each lift level is stored in a two-dimensional array,
// called "positions".
// The GRBL commands needed for moves look like: G90 X123.456 Y123.456.
// The numbers 123.456 are the lift position im mm., and can be stored as char arrays with a size
// defined by NUMBER_LENGHT. Since the lift can move 1000mm, numbers may be up to 4 digits before 
// the decimal separator (.), and 3 digits behind.
// The size is therefore 7 digits, a decimal separator (.) and a closing '\0' termination character.
#define MAX_LEVEL      12                // The number of levels the lift can move to
#define NUMBER_LENGHT  10                // Each lift positions is stored as char array with this size

class lift_class {
  public: 
    // Attributes:   
    char positions[MAX_LEVEL][NUMBER_LENGHT];      // We start at the 0-level
    char currentPosition[NUMBER_LENGHT];           // Holds the current lift position
    uint8_t level;                                 // The level where the lift is / should move to

    // Methods:
    lift_class();                                  // Constructor for initialisation
    void move(uint8_t level);                      // Move the lift to the requested level
    void storePosition(uint8_t level);             // Store the currentPosition at the given level

  private:
    uint16_t EpromStart;                           // Start address in EEPROM
    uint16_t EpromLevel[MAX_LEVEL];                // Start address for each level   
};


/*****************************************************************************************************/
// The GRBL class controls the interface to the GRBL controller, which runs on a seperate ATMega328 /
// Arduino Uno processor and connects via Serial2. The update() method should be called from Main as
// often as possible, to ensure that:
// - characters received from the GRBL controller are immediately parsed,
// - a GRBL status request (?) is periodically send and
// - the jog object keeps running.
class grbl {
  public:
    // The stepper motor may be in one of the following states
    typedef enum {IDLE, RUN, JOG, ALARM, HOLD, HOMING, UNKNOWN} grblState_t;

     // Attributes
    grblState_t state;                   // Can be analysed by main 
    bool accept_jog_commands;            // ok received. New (jog) commands are possible

    // Constructor for initialisation
    grbl();                              // Intialise timers and states
    
    // Method that should be called from main as often as possible
    void update();

    // Generic methods
    bool state_changed();                // To check if the lift state has changed 
    bool position_changed();             // For main to check if the lift position has changed
    
  private: 
    // Methods
    void query_status();                 // If possible, send a status request: ?
    void parse_grbl_input();             // Parse GRBL response characters

    // The GRBL output parser may be in one of the following states
    typedef enum {
      CrLf,                              // Carriage Return / Linefeed is detected
      StatusReport,                      // "<" detected: start of status report
      SR_state,                          // "<I" or "<R" detected. Number expected
      SR_Number,                         // ":" detected: number starts
      HO_state,                          // "<H" detected. HOME or HOLD expected
      Ok,                                // "o" detected after CR/LF
      Alarm,                             // "A" detected after CR/LF
      Skip                               // Remainder of line doesn't need to be parsed
    } parseState_t;
    parseState_t parseState;

    // Temporary buffer to store the digits of the lift position while parsing the GRBL response line
    char number[NUMBER_LENGHT];          // Buffer in which the lift position builds up
    uint8_t number_index;                // Index to the number char array
    void clear_number();                 // Clears the number char array
    void add_char2number(char inbyte);   // Adds a character to number char array
    void copyNumber2liftposition();      // Copy the temporary buffer if the number is complete
    
    // Needed to inform main that the state or lift position has changed  
    bool positionhasChanged;             // Is cleared after main calls if (position_changed()) 
    grblState_t previous_state;          // Needed to determine if a state change has occured
    grblState_t future_state;            // Store the state until the number is completely parsed 
    
    MoToTimebase query_time;             // Internal timer object for the GRBL controller
};


/*****************************************************************************************************/
// The jog class allows the lift to move up/down as long as the UP/DOWN buttons are pushed.
// Jogging starts slowly, to allow steps of 0,1mm to be made. After some time, lift movements
// become faster, although for several reasons maximum speed can not be attained.
// Although the basic operation of jogging is relatively simple, there are some
// complexities to consider.
// First, we should avoid overloading the GRBL input buffer. Therefore we implement the simple
// Send-Response streaming protocol, as described on the "Interface" page on the GRBL wiki. 
// The basic idea is to use a "accept_jog_commands" flag, which is set by the grbl parser once
// an "ok" response is received, and cleared once the software sends a new jog command.
// This flow-control mechanism will avoid incomplete commands and "error 1" response messages.
// NOTE: this flow-control mechanism may possible also be useful for other GRBL commands.
// Second, the GRBL v1.1 controller shows some unexpected behavior, in the sense that it sometimes
// resumes jogging although the jog-cancel command was received just before. The exact cause of
// this behavior is unknown, but seems to occur when previous jog commands did not 100% complete
// before the cancel command was received. Therefore it is important to define the jog-commands
// in such a way that they can be completed before a new command is received.
// Parameters that influence this behaviour are distance and speed of a jog command. If the distance
// is too much, or the speed too low, jog commands may not 100% complete before a next is received.
// Note that (of course) also the interval between two commands plays a role, but that interval
// determines how fast a new command is send after a button event, thus should remain around 500ms.
class jog_class {
  
  public:   
    typedef enum {UP, DOWN} dir_t;    // For the UP and DOWN buttons
    void start(dir_t dir);            // Can be called by the main loop
    void update();                    // Should be called as frequent as possible
    void cancel();                    // Can be called by the main loop

  private:
    dir_t direction;                  // UP or DOWN jogging
    MoToTimebase jog_interval;        // How often do we issue jog commands
    MoToTimebase jog_slow;            // First period in which we jog slow 
    MoToTimebase jog_medium;          // Second period in which we jog a bit faster
    MoToTimebase jog_fast;            // Third period in which we jog fast. After this it gets very fast  
    enum {slow, medium, fast, faster} speed;
    // Declare the various jog commands
    // Be careful to change these values, since the interval between jog commands (500ms)
    // should be large enough to avoid the GRBL controller from overload
    // accomodate a complete jog command. In other words, we
    // should avoid an overflow of the GRBL input buffer.
    // There seems to be several (pragmatic) approaches to verify overflow doesn't occur
    // If we hear that the speed of the motors is varying, we know that the motors accelerate and
    // and slow down. In such case there is no buffer overflow.
    const char* slow_up     = "$J=G91 X0.1  Y0.1  F50";
    const char* slow_down   = "$J=G91 X-0.1 Y-0.1 F50";
    const char* medium_up   = "$J=G91 X1  Y1  F200";
    const char* medium_down = "$J=G91 X-1 Y-1 F200";
    const char* fast_up     = "$J=G91 X5  Y5  F750";
    const char* fast_down   = "$J=G91 X-5 Y-5 F750";
    const char* faster_up   = "$J=G91 X10  Y10  F1700";
    const char* faster_down = "$J=G91 X-10 Y-10 F1700";
};


/*****************************************************************************************************/
// The reset class is intended to stop the lift or perform a homing action.
// There are two possibilities to stop the stepper motors:
// 1) Soft-reset: If the lift is moving, it will immediately halt and GRBL will move into the ALARM
//    state. This state will be left after sending an unlock command.
//    The Grbl processor may have lost position, which means that a rehoming cycle may be needed.
// 2) Feed Hold: Grbl is placed into a HOLD state. If in motion, the machine will decelerate
//    to a stop and then be suspended. The HOLD state will be left after sending a resume or
//    soft-reset command.
// If the RESET button is pushed, the best is to send a soft-reset. This ensures the lift will not
// restart its movements afterwards.
// If the DCC Controller receives an emergency stop, the best seems to send a feed-hold command. 
// This allows to lift to continue its movement afterwards. 
// In case the lift should not restart its movement after a DCC emergency stop, while still in the 
// HOLD state the reset-button may be pushed to perform a soft-reset
class reset_class {
  public:
    reset_class();                    // Constructor for initialisation
    void soft_reset();                // Immediately halts and safely resets Grbl
    void unlock();                    // To unlock after a soft-reset. A new homing cycle may be needed
    void feedhold();                  // The machine decelerates to a stop and then be suspended
    void resume();                    // To resume after a feedhold
    void home();                      // Perform a homing cycle
    bool homing;                      // Boolean to indicate we are in a homing cycle
};


/*****************************************************************************************************/
// Definition of external objects, which are declared here but used by main 
extern lift_class   lift;                // Keeps track were the lift is and can move to
extern grbl         stepper;             // To control the stepper motors
extern jog_class    jog_object;          // Used as long as an UP or DOWN button is pushed
extern reset_class  reset_object;        // To handle various reset and alarm cases
