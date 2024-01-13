//******************************************************************************************************
//
// file:      mySettings.h
// author:    Aiko Pras
// history:   2024-01-6 V1.0.0 ap initial version
//
// purpose:   Variables and #defines that must be set by the user
//
//******************************************************************************************************
#pragma once


// 1) Tell the sketch which board we are using
// ===========================================
// All lift controller code has been developed for the Arduino 2560 microcontroller.  
// It is intended for the Lift decoder boards that are available from the OSHWLAB.
// - SMD board: https://oshwlab.com/aikopras/support-lift-controller
// - THT board: https://oshwlab.com/aikopras/lift-decoder-arduino-mega-tht
// Select the board that will be used. 
// #define BOARD_SMD
#define BOARD_THT
