// *******************************************************************************************************
// File:      hardware.h
// Author:    Aiko Pras
// History:   2021/04/24 AP Version 1.0
//            2023/11/13 AP Version 1.1 - Added #defines for the LEDs, to ease usage of a MEGA board 
//            2023/30/12 AP Version 1.2 - Share same file between Main, Button and IR board
//                                        Select between the THT and SMD board via compiler directive  
// 
// Purpose:   Pin definitions for the Lift decoder Main, Button and IR boards
//
// Note:      The RS485 specific settings are defined in the RS485_Lift.h library           
//            The common DCC and RS-Bus settings are defined by including the AP_DCC_Decoder_Core
//            library (which in turn includes the AP_DCC_Library)
//  
// ******************************************************************************************************
#pragma once

// All lift controller code has been developed for the Arduino 2560 microcontroller.  
// It is intended for the Lift decoder boards that are available from the OSHWLAB.
// The SMD board can be found at: https://oshwlab.com/aikopras/support-lift-controller
// The THT board can be found at: TODO

// Select the board that will be used. 
#define BOARD SMD
// #define BOARD THT


// Do not edit below this line
// ---------------------------
#if (BOARD == SMD) 
#define LED_BLUE       PIN_PD4 
#define LED_GREEN      PIN_PD5 
#define LED_YELLOW     PIN_PD6 
#elif (BOARD == THT)
#define LED_BLUE       PIN_PE3 
#define LED_GREEN      PIN_PG5 
#define LED_YELLOW     PIN_PE5 
#else
#warning BOARD NOT DEFINED!
#endif

// Monitoring Pins
#define MON_RXD        PIN_PE0    // Same as RXD0 / Mega Pin 0
#define MON_TXD        PIN_PE1    // Same as TXD0 / Mega Pin 1

// GRBL Pins
#define GRBL_RXD       PIN_PH0    // Same as RXD2 / Mega Pin 17
#define GRBL_TXD       PIN_PH1    // Same as TXD2 / Mega Pin 16

// LCD Pins (on the IDC 16 pin connector)
#define RS       53   // PB0 - Connector: Pin 6   / Mega Pin 53
#define RW       51   // PB2 - Connector: Pin 5   / Mega Pin 51
#define ENABLE   50   // PB3 - Connector: Pin 15  / Mega Pin 50
#define D4       12   // PB6 - Connector: Pin 2   / Mega Pin 12
#define D5       11   // PB5 - Connector: Pin 3   / Mega Pin 11
#define D6        9   // PH6 - Connector: Pin 4   / Mega Pin 9
#define D7       10   // PB4 - Connector: Pin 5   / Mega Pin 10


// Pins for external relays:
#define RELAY1_POS1    63  // PIN_PK1 - Number on PCB: OUT 10
#define RELAY1_POS2    64  // PIN_PK2 - Number on PCB: OUT 11 
#define RELAY2_POS1    65  // PIN_PK3 - Number on PCB: OUT 12 
#define RELAY2_POS2    66  // PIN_PK4 - Number on PCB: OUT 13 


// ******************************************************************************************************
// For completeness of documentation, below the IN and OUT pins, as used by the Buttons / IR-Sensors
// Board IN number:   1   2   3   4   5   6   7   8   9  10  11  12  13  14
// Arduino Pin:      49  48  47  46  45  44  43  42  37  36  35  34  33  32
// PORT              PL  PL  PL  PL  PL  PL  PL  PL  PC  PC  PC  PC  PC  PC
// PIN                0   1   2   3   4   5   6   7   0   1   2   3   4   5
//
// Board OUT number:  1   2   3   4   5   6   7   8   9  10  11  12  13  14
// Arduino Pin:      54  55  56  57  58  59  60  61  62  63  64  65  66  67
// PORT              PF  PF  PF  PF  PF  PF  PF  PF  PK  PK  PK  PK  PK  PK
// PIN                0   1   2   3   4   5   6   7   0   1   2   3   4   5
//
