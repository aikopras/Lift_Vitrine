//******************************************************************************************************
//
// file:      debug-liftdecoder.h
// purpose:   Debug pins for the lift-decoder board
// author:    Aiko Pras
// version:   2021-12-29 Version 1.0 Initial version 
//
// Five testpoints are defined: TP0 ... TP4. The pins correspnd to Port L on the various boards
// There is ini, set, clear and toggle.
//                                                                       
//******************************************************************************************************
#pragma once
#include <Arduino.h>

// Arduino MEGA 2560 Board
#define INI_TP0  DDRL |=  (1<<0)    // PL0
#define SET_TP0 PORTL |=  (1<<0)
#define CLR_TP0 PORTL &= ~(1<<0)
#define TGL_TP0 PORTL ^=  (1<<0)
#define INI_TP1  DDRL |=  (1<<1)    // PL1
#define SET_TP1 PORTL |=  (1<<1)
#define CLR_TP1 PORTL &= ~(1<<1)
#define TGL_TP1 PORTL ^=  (1<<1)
#define INI_TP2  DDRL |=  (1<<2)    // PL2
#define SET_TP2 PORTL |=  (1<<2)
#define CLR_TP2 PORTL &= ~(1<<2)
#define TGL_TP2 PORTL ^=  (1<<2)
#define INI_TP3  DDRL |=  (1<<3)    // PL3
#define SET_TP3 PORTL |=  (1<<3)
#define CLR_TP3 PORTL &= ~(1<<3)
#define TGL_TP3 PORTL ^=  (1<<3)
#define INI_TP4  DDRL |=  (1<<4)    // PL4
#define SET_TP4 PORTL |=  (1<<4)
#define CLR_TP4 PORTL &= ~(1<<4)
#define TGL_TP4 PORTL ^=  (1<<4)
#define INI_TP5  DDRL |=  (1<<5)    // PL5
#define SET_TP5 PORTL |=  (1<<5)
#define CLR_TP5 PORTL &= ~(1<<5)
#define TGL_TP5 PORTL ^=  (1<<5)
#define INI_TP6  DDRL |=  (1<<6)    // PL6
#define SET_TP6 PORTL |=  (1<<6)
#define CLR_TP6 PORTL &= ~(1<<6)
#define TGL_TP6 PORTL ^=  (1<<6)
