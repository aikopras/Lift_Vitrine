/*******************************************************************************************************
File:      hardware.h
Author:    Aiko Pras
History:   2021/04/24 AP Version 1.0
 
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

******************************************************************************************************/
#pragma once

// Note: The RS485 specific settings are set in the library file RS485_Lift.h

#define LED_RED        PIN_PB7    // Same as LED_BUILTIN  / Mega Pin 13
#define LED_BLUE       PIN_PD4 
#define LED_GREEN      PIN_PD5 
#define LED_YELLOW     PIN_PD6 

#define TASTER         PIN_PD7    // Same as Mega Pin 38
#define DCC_IN         PIN_PD1    // Same as Mega Pin 20

// Monitoring Pins = Serial
#define MON_RXD        PIN_PE0    // Same as RXD0 / Mega Pin 0
#define MON_TXD        PIN_PE1    // Same as TXD0 / Mega Pin 1

// RS-Bus Pins = Serial1
#define RS_RECEIVE     PIN_PD2    // Same as RXD1 / Mega Pin 19
#define RS_SEND        PIN_PD3    // Same as TXD1 / Mega Pin 18

// GRBL Pins = Serial 2
#define GRBL_RXD       PIN_PH0    // Same as RXD2 / Mega Pin 17
#define GRBL_TXD       PIN_PH1    // Same as TXD2 / Mega Pin 16
