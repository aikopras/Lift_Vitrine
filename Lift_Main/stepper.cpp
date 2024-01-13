/*******************************************************************************************************
File:      stepper.cpp
Author:    Aiko Pras
History:   2021/04/24 AP Version 1.0

Purpose:   Implements the communication with the GRBL (stepper motor) controller

******************************************************************************************************/
#include <Arduino.h>
#include <EEPROM.h>
#include <MoToTimer.h>               // For the MoToTimebase
#include <AP_DCC_Decoder_Core.h>     // For the Serial_Line CV 
#include "stepper.h"
#include "mySettings.h"              // For the default lift positions


// Instantiate the external objects. 
lift_class            lift;                  // External object, used by main
grbl                  stepper;               // External object, used by main
jog_class             jog_object;            // External object, used by main
reset_class           reset_object;          // External object, used by main


//*****************************************************************************************************
//******************************* External Methods for the Lift object ********************************
//*****************************************************************************************************
lift_class::lift_class() {
  currentPosition[0] = '\0';                 // Make the currentPosition an empty string
  level = 0;                                 // Assume we start at Level 0
  // We start the liftpositions array at the end of the EEPROM space
  EpromStart = EEPROM.length() - (MAX_LEVEL * NUMBER_LENGHT) - 1;
  // Determine the EEPROM address of each inidividual lift position
  for (uint8_t i=0; i < MAX_LEVEL; i++) 
    EpromLevel[i] = EpromStart + (i * NUMBER_LENGHT);
  // Check the character before the lift positions to determine if we are initialised.
  if ((EEPROM.read(EpromStart - 1) != 0b01010101) || (FORCE_EEPROM_WRITE)) {
    // No, we are not initialised . Set default values, to avoid all values being FF (255)
    strcpy(lift.positions[0], LEVEL00);
    strcpy(lift.positions[1], LEVEL01);
    strcpy(lift.positions[2], LEVEL02);
    strcpy(lift.positions[3], LEVEL03);
    strcpy(lift.positions[4], LEVEL04);
    strcpy(lift.positions[5], LEVEL05);
    strcpy(lift.positions[6], LEVEL06);
    strcpy(lift.positions[7], LEVEL07);
    strcpy(lift.positions[8], LEVEL08);
    strcpy(lift.positions[9], LEVEL09);
    strcpy(lift.positions[10], LEVEL10);
    strcpy(lift.positions[11], LEVEL11);
    // Now that we have defaults, store these in the EEPROM
    for (uint8_t i=0; i < MAX_LEVEL; i++) 
      EEPROM.put(EpromLevel[i], lift.positions[i]);
    // And set the character before the lift positions, to avoid re-initialisation.
    EEPROM.update(EpromStart - 1, 0b01010101);
  }
  else { // We are initialised. Retrieve values from EEPROM
    for (uint8_t i=0; i < MAX_LEVEL; i++) 
      EEPROM.get(EpromLevel[i], lift.positions[i]);
  }
}


void lift_class::move(uint8_t level) {
  // To the GRBL controller
  Serial2.print("G90 X");
  Serial2.print(positions[level]);
  Serial2.print(" Y");
  Serial2.println(positions[level]);
  if (cvValues.read(Serial_Line)) { 
    Serial.print("G90 X");
    Serial.print(positions[level]);
    Serial.print(" Y");
    Serial.println(positions[level]); 
  } 
}


void lift_class::storePosition(uint8_t i){
  EEPROM.put(EpromLevel[i], currentPosition);
}


//*****************************************************************************************************
//******************************* External Methods for the GRBL object ********************************
//*****************************************************************************************************
// The constructor below initialises the object
grbl::grbl() {
  query_time.setBasetime(1000);              // How often we send the Status Report Query (?) 
  state = UNKNOWN;                           // The external state machine, seen by main
  previous_state = UNKNOWN;                  // Internal variable, to detect state changes
  parseState = Skip;                         // The internal state machine. We skip the first line
  clear_number();                            // Make the temporary buffer for the position an empty string
}


//*****************************************************************************************************
//******************************* External Methods for the GRBL object ********************************
//*****************************************************************************************************
void grbl::update() {
  // Should be called from main as often as possible
  query_status();
  parse_grbl_input();
  jog_object.update();
}



void grbl::query_status() {
  // Should be called from main as often as possible
  // Query the grbl controller by sending every second a ? character (Status Report Query)
  // We use a "write", since this is a bit faster than a "print".
  // We don't need a CR/LF (which would result in an "ok" message), thus "println" is not needed
  if (query_time.tick()) {
    Serial2.write("?");
  }
}


void grbl::parse_grbl_input() {
  // We will check the serial port for the GRBL controller if a new character has arrived
  // If such character has arrived we will parse it immediately to determine the status
  // of the stepper motor. We use an internal state machine to keep track of where we are
  // and once we have determined the state of the GRBL controller and its precise position,
  // we inform the main program.
  if (Serial2.available()) {
    char inByte = Serial2.read ();
    if (cvValues.read(Serial_Line) > 1) Serial.write(inByte);
    switch (parseState) {
      case Skip:
        // In this state we ignore everything, except the start of a new line
        if ((inByte == '\r') || (inByte == '\n')) {
          parseState = CrLf;
        }
        break;
      case CrLf:
        // We expect the start of a status report (<), an ok (o), the start of an Alarm (A)
        // or another CR / LF. If we receive something else, we will skip what comes after
        if (inByte == '<') parseState = StatusReport;
        else if (inByte == 'o') parseState = Ok;
        else if (inByte == 'A') parseState = Alarm;
        else if ((inByte == '\r') || (inByte == '\n')) parseState = CrLf;
        else parseState = Skip;
        break;
      case StatusReport:
        // Status reports start with Idle, Run, Hold, Jog, Alarm, Door, Check, Home, Sleep, ...
        // We care for Idle commands (the lift does not move) and Run commands (the lift is busy)
        // We do not immediately inform main, but wait till we also parsed the number representing
        // the current liftpostion. The new state will therefore temporarily be saved in "future-state".
        if (inByte == 'I') {
          future_state = IDLE;
          parseState = SR_state;
       }
        else if (inByte == 'R') {
          future_state = RUN;
          parseState = SR_state;
        }
        else if (inByte == 'J') {
          future_state = JOG;
          parseState = SR_state;
        }
        else if (inByte == 'H') { // Homing or Hold
          parseState = HO_state;
        }
        else parseState = Skip;
        break;
      case SR_state:
        // We saw the start of an Idle or Run status report. We are now interested in 
        // the first number (the position of the X-stepper). The number starts after a colon (:)
        if (inByte == ':') {
          parseState = SR_Number;
          clear_number();
        }
        break;
      case SR_Number:
        // we will now receive a decimal value. The value ends with a comma (,)
        // In general the value may start with a negative sign (-). However, in our application
        // numbers should always be positive. The decimal seperator is a point (.), in general
        // there will be 3 digits after this seperator, but we will not check for this.
        // If we receive anything different from a comma, digit or point, we will ignore everything
        if ((isDigit(inByte)) || (inByte == '.')) add_char2number(inByte);
        else if (inByte == ',') {
          // Now we have parsed those parts of the statusline that we need.
          copyNumber2liftposition();
          state = future_state;
          parseState = Skip;
        }
        break;
      case HO_state:
        // We saw the start of a HOMING or HOLD status report. We are now interested  
        // in the first character after the "O": "M" or "L"?
        if (inByte == 'o') parseState = HO_state;
        else if (inByte == 'm') {
          state = HOMING;
          parseState = Skip;
        }
        else if (inByte == 'l') {
          state = HOLD;
          parseState = Skip;
        }
        break;
      case Ok:
        // For flow-control purposes. New (jog) commands are accepted
        accept_jog_commands = true;
        parseState = Skip;
        break;
      case Alarm:
        // What will we do in this state??
        state = ALARM;
        parseState = Skip;
        break;
    } // Switch
  } // While
}


bool grbl::state_changed() {
// Main should check if the lift has changed state state
  bool result = false;
  if ((state != previous_state) ) {
    result = true;
    previous_state = state;
  }
  return result;
}


bool grbl::position_changed() {
// Main should check if the lift has changed position
  bool result = false;
  if (positionhasChanged) {
    result = true;
    positionhasChanged = false;
  }
  return result;
}


//*****************************************************************************************************
//******************************* Internal Methods for the GRBL object ********************************
//*****************************************************************************************************
void grbl::clear_number() {
  // clears the temporary char array that holds the lift position
  number_index = 0;
  number[number_index] = '\0';
}


void grbl::add_char2number(char inbyte) {
  // adds a character to number char array
  if (number_index < (NUMBER_LENGHT - 1)) {
    number[number_index] = inbyte;
    number_index++;
    number[number_index] = '\0';
  }
}


void grbl::copyNumber2liftposition() {
  // If the number is complete (a comma was received from GRBL), we will copy this
  // number to the currentPosition character array if the position has changed. 
  // In that case we also set the boolean "positionhasChanged", which will be cleared
  // once main calls position_changed().
  if (strcmp(lift.currentPosition,number)) {
    // The new position, as contained in "number", differs from the old position
    strcpy(lift.currentPosition, number);
    positionhasChanged = true;
  }
}


//*****************************************************************************************************
//******************************* External Methods for the JOG object *********************************
//*****************************************************************************************************
#include "rs485.h"          // Use RS485 to read button status and set button LEDs

void jog_class::start(dir_t dir) {
  speed = slow;                     // We start with the slowest speed
  jog_interval.setBasetime(500);    // We send a jog command every 500 ms
  jog_slow.setBasetime(1500);       // After 1.5 seconds the jog speed increases to medium
  jog_medium.setBasetime(4000);     // After 4 seconds the jog speed increases to fast
  jog_fast.setBasetime(8000);       // After 6 seconds the jog speed increases to faster
  direction = dir;                  // Should we move UP or the DOWN?
  if (direction == UP) {Serial2.println(slow_up); }
  else {Serial2.println(slow_down); }
  // The accept_jog_commands flag is used by the Simple Send-Response streaming protocol
  stepper.accept_jog_commands = true;
}



void jog_class::update() { 
  if (jog_slow.tick())   {jog_slow.stop();   speed = medium;}
  if (jog_medium.tick()) {jog_medium.stop(); speed = fast;}
  if (jog_fast.tick())   {jog_fast.stop();   speed = faster;}
  // Send jog commands periodically and check if GRBL is ready for a new command
  if ((jog_interval.tick()) && (stepper.accept_jog_commands)) {
    stepper.accept_jog_commands = false;
    switch (speed) {
      case slow:
        if (direction == UP) {Serial2.println(slow_up); }
        else {Serial2.println(slow_down); }
      break;
      case medium:
        if (direction == UP) {Serial2.println(medium_up); }
        else {Serial2.println(medium_down); }
      break;
      case fast:
        if (direction == UP) {Serial2.println(fast_up); }
        else {Serial2.println(fast_down); }
      break;
      case faster:
        if (direction == UP) {Serial2.println(faster_up); }
        else {Serial2.println(faster_down); }
      break;
    }
    Serial2.flush();
  }
}


void jog_class::cancel() { 
  Serial2.write(0x85);          // Jog Cancel command
  jog_interval.stop();
  jog_slow.stop();
  jog_medium.stop();
  jog_fast.stop();
}


//*****************************************************************************************************
//****************************** External Methods for the RESET object ********************************
//*****************************************************************************************************
reset_class::reset_class() {
  homing = false;
}

void reset_class::soft_reset() {                
  // Immediately halts and safely resets Grbl
  Serial2.write(0x18);   // ^x
}


void reset_class::unlock() {
  // To unlock after a soft-reset.
  Serial2.println("$X");
}


void reset_class::feedhold() {
  // The machine decelerates to a stop and then be suspended
  Serial2.write("!");
}


void reset_class::resume(){
  // To resume after a feedhold
  Serial2.write("~");
}


void reset_class::home() {
  // Perform a homing cycle. Avoid new cycles when the old cycle hasn't completed.
//  if (!homing) {
    Serial2.println("$H");
//    homing = true;          // grbl::state_changed() sets to false once stepper state changed
//  }
}
