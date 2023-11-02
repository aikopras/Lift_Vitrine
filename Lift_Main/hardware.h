// *******************************************************************************************************
// File:      hardware.h
// Author:    Aiko Pras
// History:   2021/04/24 AP Version 1.0
//
// Purpose:   Lift decoder - Board details
//
// All lift controller code has been developed for the Arduino 2560 microcontroller and should run,
// after some minor modifications, on generic Arduino MEGA boards.
//
// Specific components, such as the GRBL controller and RS485 interface, should be connected to various pins.
// This file describes which pins are needed for use with the dedicated Lift Controller board, as can be found on:
// https://easyeda.com/aikopras/support-lift-controller
// Note that buying assembled boards should be relatively cheap; in summer 2020 ordering five boards did cost
// less than 100 Euro, including shipping and tax.
//
// The boards above use some AVR pins that are not available on standard Arduino MEGA boards:
// - RS485 Enable/Disable: PJ2
// - Blue LED: PD4
// - Green LED: PD5
// - Yellow LEDs: PD6
//
// To easily access these pins, for compiling via the Arduino IDE the MEGACORE - ATmega2560 board has been used
// See: https://github.com/MCUdude/MegaCore for installation and usage.
//
// The following settings are used (under the Tools section of the Arduino IDE):
// - Board: ATmega2560             - Important to set this right
// - Clock: External 16Mhz         - Important to set this right
// - BOD: 2,7V                     - Seems a save value
// - EEPROM: ***                   - Seems that retained doesn't work
// - Compiler: LTO enabled         - More efficient code
// - Pinout: Arduino MEGA Pinout   - Important to set this right
// - Bootloader: No                - Important - See below
// - Port: usbserial....           - Depends on the specifics of your development system
// - Programmer: USBASP (Megacore) - Depends on the specifics of your development system
//
// Bootloader: although the board makes UART0 available as monitoring port, which can be connected
// to the development system via a serial to usb adapter, in general we won't be able to use such adapter
// to upload new code. The main reason is that a bootloader requires a RESET before it will start
// uploading new code. For this to happen the DTR signal of a serial adapter should be connected
// to the boards reset pin. Most adapters don't export the DTR signal.
//
// Setting the Fuses
// Before the board can be used first, the fuses need to be set. There are two options for that:
// 1) Make sure that "Bootloader: No" is selected. After that select Tools => Burn Bootloader
//    This will set the fuse bits, but will not upload a bootloader
// 2) Use the development system's command line to issue the following command (assuming usbasp):
// avrdude -c usbasp -p m2560 -U lfuse:w:0xEE:m -U hfuse:w:0xD9:m -U efuse:w:0xFE:m
//
// Note: all DCC and RS-Bus pins are defined and initialised in boards.h from the
// AP_DCC_Decoder_Core library, which is included by dcc_rs.h
// //
// Board OUT number:  1   2   3   4   5   6   7   8   9  10  11  12  13  14
// Arduino Pin:      54  55  56  57  58  59  60  61  62  53  64  65  66  67
// PORT              PF  PF  PF  PF  PF  PF  PF  PF  PK  PK  PK  PK  PK  PK
// PIN                0   1   2   3   4   5   6   7   0   1   2   3   4   5
//
// ******************************************************************************************************
#pragma once
// Note 1: The common DCC and RS-Bus settings are made in the AP_DCC_Common library
// Note 2: : The RS485 specific settings are set in the library file RS485_Lift.h

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


// Pins for external relays:
#define RELAY1_POS1    63  // PIN_PK1 - Number on PCB: OUT 10
#define RELAY1_POS2    64  // PIN_PK2 - Number on PCB: OUT 11
#define RELAY2_POS1    65  // PIN_PK3 - Number on PCB: OUT 12
#define RELAY2_POS2    66  // PIN_PK4 - Number on PCB: OUT 13
