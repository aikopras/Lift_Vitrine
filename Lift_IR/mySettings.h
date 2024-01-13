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
#define BOARD_SMD
// #define BOARD_THT


// 2) Tell the sketch which I/O ports are used for sensors
// =======================================================
// The following #define tells which Inputs and Outputs of the Liftdecoder board are being used.
// Modify to represent your specific sensor setting
// The lowest order bit corresponds to the IN/OUT pair of the blue connectors that are labelled 1
// Examples:
// - 0b0000000000000001 a single sensor is connected to IN/OUT 1
// - 0b0000000000000011 two sensors are connected to IN/OUT 1 and 2
// - 0b0000000000000010 a single sensor is connected to IN/OUT 2
// - 0b0000111111111111 twelve sensors are connected to IN/OUT 1..12
// - 0b0011111111111111 all 14 sensors are connected
#define MASK_SENSORS_CONNECTED 0b0000111111111111


// 3) Set the sensor sensitivity
// =============================
// The sensitivity of the IR Leds and sensors is best around 38kHz. 
// Therefore most IR systems operate at 38 Khz. At that frequency, a full pulse takes 26,3 us
//
//          +-------+
//          |       |
//          |       |
//          +       +-------+
//            13 us   13 us                                
//
// Unfortunately, at 38Khz the system may become too sensitive, and suffer from indirect reflections.
// Reflections result in false negatives: although a train blocks the direct IR-beam, this is not detected.
//
// Fortunately the LED and sensor also operate at lower (or higher) frequencies, although less sensitive.
// To decrease the sensitivity of the sensors, we will lower the frequency (in kHZ).
// In practice a value of 25Khz seems to give good results, but you may experiment with this value
// by modifying the #define KHZ value.
//
// To avoid the sensor from saturation, the burst of IR pulses may not become too long. 
// Reasonable values are 15 to 30 pulses, which corresponds (roughly) to 375 us (15*25) till 750us.
#define KHZ 25                       // Frequency at which we operate the IR system
#define BURST_TIME 600               // Time in us the burst will last


// 4) Set the RS-Bus addresses
// ===========================
// Although the Main Lift Controller board signals to the DCC system / control computer if all IR-Sensors
// are free or not (single bit), it may be convenient to also know the status of each individual sensor.
// This is particularly useful to detect possible errors. 
// Therefore we also use the RSBus to send feedback regarding individual sensors
// We use two RSBus feedback addresses, to accommodate a maximum of 16 sensors (although the board 
// supports "only" 14 sensors). You may change the addresses to whatever you like.
// If you don't use RSBus feedback, just keep the values below to avoid compile errors.
const uint8_t RS_AddresLow = 124;     // 1.. 128
const uint8_t RS_AddresHigh = 125;    // 1.. 128


// 5) Increase speed
// =================
// To increase speed, we can use two of the three GPIORs that are available on a ATMega 2560. 
// This requires, however, that these GPIORs are not yet used elsewhere. 
// The AP-DCC-Library, for example, already uses GPIOR0 and GPIOR1.
// In case we also need the AP-DCC-Library, use the volatile uint8_t variables instead
#define PORT GPIOR0                   // Fast, but may interfere with other libraries
#define BITMASK GPIOR1                 
//volatile uint8_t PORT;              // Slow, but more portable
//volatile uint8_t BITMASK;
