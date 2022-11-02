/*******************************************************************************************************
File:      hardware.h
Author:    Aiko Pras
History:   2021/04/24 AP Version 1.0

Purpose:   Lift decoder - Board details
 
All lift controller code has been developed for the Arduino 2560 microcontroller and should run, 
after some minor modifications, on generic Arduino MEGA boards. 

Specific components, such as the GRBL controller and RS485 interface, should be connected to various pins.
This file describes which pins are needed for use with the dedicated Lift Controller board, as can be found on: 
https://easyeda.com/aikopras/support-lift-controller
Note that buying assembled boards should be relatively cheap; in summer 2020 ordering five boards did cost
less than 100 Euro, including shipping and tax.

The boards above use some AVR pins that are not available on standard Arduino MEGA boards: 
- RS485 Enable/Disable: PJ2
- Blue LED: PD4
- Green LED: PD5
- Yellow LEDs: PD6

To easily access these pins, for compiling via the Arduino IDE the MEGACORE - ATmega2560 board has been used
See: https://github.com/MCUdude/MegaCore for installation and usage. 
See the main sketch for compiler settings

How are the IR-LEDs and sensors connected to the board??

Sensors:
Board IN number:   1   2   3   4   5   6   7   8   9  10  11  12  13  14
Arduino Pin:      49  48  47  46  45  44  43  42  37  36  35  34  33  32
PORT              PL  PL  PL  PL  PL  PL  PL  PL  PC  PC  PC  PC  PC  PC
PIN                0   1   2   3   4   5   6   7   0   1   2   3   4   5

IR-LEDs:
Board OUT number:  1   2   3   4   5   6   7   8   9  10  11  12  13  14
Arduino Pin:      54  55  56  57  58  59  60  61  62  53  64  65  66  67
PORT              PF  PF  PF  PF  PF  PF  PF  PF  PK  PK  PK  PK  PK  PK
PIN                0   1   2   3   4   5   6   7   0   1   2   3   4   5

******************************************************************************************************/
#pragma once
// Note 1: The common DCC and RS-Bus settings are made in the AP_DCC_Common library
// Note 2: The RS485 specific settings are set in the library file RS485_Lift.h

#define LED_BLUE       PIN_PD4 
#define LED_GREEN      PIN_PD5 
#define LED_YELLOW     PIN_PD6 

// Monitoring Pins
#define MON_RXD        PIN_PE0    // Same as RXD0 / Mega Pin 0
#define MON_TXD        PIN_PE1    // Same as TXD0 / Mega Pin 1

// GRBL Pins
#define GRBL_RXD       PIN_PH0    // Same as RXD2 / Mega Pin 17
#define GRBL_TXD       PIN_PH1    // Same as TXD2 / Mega Pin 16

// LCD Pins (on the IDC 16 pin connector)
#define RS       53   // PB0 - Connector: Pin 6
#define RW       51   // PB2 - Connector: Pin 5
#define ENABLE   50   // PB3 - Connector: Pin 15
#define D4       12   // PB6 - Connector: Pin 2
#define D5       11   // PB5 - Connector: Pin 3
#define D6        9   // PH6 - Connector: Pin 4
#define D7       10   // PB4 - Connector: Pin 5
