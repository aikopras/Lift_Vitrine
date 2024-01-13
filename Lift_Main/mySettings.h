/*******************************************************************************************************
File:      mySettings.h
Author:    Aiko Pras
History:   2024/01/10 AP Version 1.0


Purpose:   Allow users of this sketch to select the board (SMD or THT) and tailor its behaviour.
           Such tailoring may also be done via Programming on the Main (PoM) messages,
           but for most users it may be easier to adjust the settings below.

******************************************************************************************************/
// All lift controller code has been developed for the Arduino 2560 microcontroller.  
// It is tested on two lift decoder boards, which can be obtained from the OSHWLAB.
// - SMD board: https://oshwlab.com/aikopras/support-lift-controller
// - THT board: https://oshwlab.com/aikopras/lift-decoder-arduino-mega-tht
// Below you have to select the #define that matches your board (remove the starting //). 
// #define BOARD_SMD
#define BOARD_THT


// By default, a homing cycle for the stepper motor(s) is performed at program start.
// Such homing cycle ensures that the lift will move to a defined position.
// Uncomment the #define (remove the starting //) if homing is NOT desired
#define NO_HOMING


// By default, before the lift starts moving, the IR-sensors connected to the dedicated IR-Sensor Board
// will be checked. If a train blocks an IR-beam, the lift will not move.
// Uncomment the #define (remove the starting //) if the IR-sensors should not be checked.
// This may be needed for testing purposes, or if no IR_Sensor board is connected
#define NO_IR_SENSORS


// By default, the LCD is disabled, to avoid interference with RS-bus feedback and other
// time critical functions. Such interference is caused by the fact that writing to the LCD screen takes
// quite some time. For testing purposes, or in cases where time critical functions like RS-bus feedback
// are not needed, the LCD display may be enabled.
// #define ENABLE_LCD


// By default, the serial monitor interface is switched off (value = 0). 
// For entering GRBL commands or debugging, it may be convenient to enable the serial monitor. 
// If the value = 1, input from the serial line will be copied to the GRBL processor, and information 
// gets displayed regarding the current lift position.
// If the value = 2, input from the serial line will be copied to the GRBL processor, and all 
// coming back from the GRBL processor gets displayed.
// #define SERIAL_MONITOR 1


// The decoder can listen to DCC accessory commands to move the lift to a certain level.
// Each lift level has its own switch address; the first switch address is for level 0;
// the second switch address is for level 1, the third for level 2, etc.
// Per 4 switch addresses we need one decoder address. Since the maximum number of lift levels is 12,
// we listen to three decoder addresses.
// The first decoder address is stored in CV1 plus CV9. The relationship between CV1, CV9 
// and the decoder address is explained in RCN-213 (Section 2.1) and RCN-225.
// - the valid range for CV1 is 1..63 (if CV9 == 0) or 0..63 (if CV9 !=0)
// - the valid range for CV9 is 0..3  (or 128, if the decoder has not been initialised).
// If you don't #define these CVs here, the default value for CV1 (myAddrL) = 1 and 
// for CV9 (myAddrH) = 128 (0x80);
// A CV9 value of 128 indicates that the address has not been set by the user yet (by pushing the button).
// In that case the decoder's red LED starts blinking
// If your lift will not  be controlled by DCC, but only manually by buttons, you should change CV9 to 0.
// Uncomment both #defines below to set the address CVs yourself
#define CV1 56
#define CV9 3


// The RS-Bus address is stored in CV10 (myRSAddr). Valid addresses are between 1..128. 
// The default value is 0, meaning that the RSbus becomes inactive. 
// The RS-Bus address 128 is used by all my decoders for PoM feedback. 
// We need two RS-Bus addresses for all feedback information; only the first address
// needs to be entered below. This address should therefore be between 1..126.
#define RS_ADDRESS 126


// Pins for external relays. They must be somewhere on the OUT 9..14 pins (Port K):
#define RELAY1_POS1    63  // PIN_PK1 - Number on PCB: OUT 10
#define RELAY1_POS2    64  // PIN_PK2 - Number on PCB: OUT 11 
#define RELAY2_POS1    65  // PIN_PK3 - Number on PCB: OUT 12 
#define RELAY2_POS2    66  // PIN_PK4 - Number on PCB: OUT 13 


// Initial lift positions. Will be entered into EEPROM if and only if the EEPROM has not been initialised.
// Once the EEPROM is initialised, values will not be written to EEPROM again, even if you make changes 
// here. Later changes regarding lift positions should be made via the buttons. In case you don't have
// buttons (since the lift is operated via DCC only), you can enable FORCE_EEPROM_WRITE (see below).
// After these values are stored in EEPROM, do NOT modify the values again!!
// Values are in millimeter (point as decimal separator).
/*
#define LEVEL00      "0.000"
#define LEVEL01     "10.000"
#define LEVEL02     "20.000"
#define LEVEL03     "30.000"
#define LEVEL04     "40.000"
#define LEVEL05     "50.000"
#define LEVEL06     "60.000"
#define LEVEL07     "70.000"
#define LEVEL08     "80.000"
#define LEVEL09     "90.000"
#define LEVEL10    "100.000"
#define LEVEL11    "110.000"
*/

// Set the #define below to 1, if the new values MUST be written to EEPROM. Don't forget to change it
// back to 0 once the new settings are stored, to avoid EEPROM wear-out.
#define FORCE_EEPROM_WRITE 0

#define LEVEL00       "0.000"
#define LEVEL01     "170.900"
#define LEVEL02     "251.000"
#define LEVEL03     "331.900"
#define LEVEL04     "412.200"
#define LEVEL05     "503.000"
#define LEVEL06     "595.000"
#define LEVEL07     "686.100"
#define LEVEL08     "778.200"
#define LEVEL09     "869.300"
#define LEVEL10     "960.600"
#define LEVEL11     "990.600"
