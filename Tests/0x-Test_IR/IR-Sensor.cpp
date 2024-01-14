// ***********************************************************************************************************
// File:       IR-Sensor.cpp
// Author:     Aiko Pras
// history:    2022-01-25 V1.0.0 ap initial version
// 
// purpose:    Control of an IR sensor (LED plus sensor)
//
// IR-LEDs operate at (roughly) 38 Khz, which means that each full pulse takes 26,3 us
//
//          +-------+
//          |       |
//          |       |
//          +       +-------+
//            13 us   13 us                                
//
// Although the sensitivity of the IR Leds and sensors is best around 38Khz, the LED and sensor also operate
// at lower or higher frequencies. If the sensitivity of the sensors should be decreased, the frequency may
// be changed by modifying the KHZ value.
// To avoid the sensor from saturation, the burst of IR pulses may not become too long. Reasonable values 
// are 15 to 30 pulses, which corresponds (roughly) to 375 (15*25) till 750us.
//
// ***********************************************************************************************************
#include <Arduino.h>
#include "IR-Sensor.h"


// ***********************************************************************************************************
// Defines that may be changed
// ***********************************************************************************************************
// To make things fast, we use two of the three available GPIORs. This requires however that these
// GPIORs are not yet used elsewhere. The AP-DCC-Library, for example, already uses GPIOR0 and GPIOR1.
// In case the AP-DCC-Library is needed, use the volatile uint8_t alternatives below
#define PORT GPIOR0                   
#define BITMASK GPIOR1                 
//volatile uint8_t PORT;              // Slower, but more portable alternative
//volatile uint8_t BITMASK;


// At 40Khz the system is too sensitive, meaning that indirect reflections lead to false positives
// (no block, although the direct IR-beam is blocked by a train).
// A value of 25Khz seems to give more reliable results, but you may experiment with this value.
#define KHZ 25                       // Frequency at which we operate the IR system
#define BURST_TIME 600               // Time in us the burst will last


// ***********************************************************************************************************
// Defines that should not be changed
// ***********************************************************************************************************
#define STOP_TIMER4  TCCR4B = 0;
#define STOP_TIMER5  TCCR5B = 0;
#define START_TIMER4 TCCR4B = (1<<WGM42) | (1<<CS40)
#define START_TIMER5 TCCR5B = (1<<WGM52) | (1<<CS50)

extern bool debugSerial;  // Needed to access this from the .ino file

IR_Sensors::IR_Sensors() {
  // Constructor to initialise the object
  // The sensors are connected to Ports L and C, which are set as inputs
  // The IR-Leds are connected to Ports F and K, which are set as outputs
  DDRC = 0x00;    // PORTC: All inputs
  DDRL = 0x00;    // PORTL: All inputs
  DDRF = 0xFF;    // PORTF: All outputs
  DDRK = 0xFF;    // PORTK: All outputs
  PORTC = 0xFF;    // PORTC: Pull-up
  PORTL = 0xFF;    // PORTL: Pull-up
}


void IR_Sensors::checkStatus(uint8_t sensor) {
  // A call takes roughly 800us (= BURST_TIME)
  // STEP 1: BITMASK and PORT determine which IR LED/Sensor pair will be used
  switch (sensor) {
    case 0:
      BITMASK = (1<<0);
      PORT = LOW;
    break;
    case 1:
      BITMASK = (1<<1);
      PORT = LOW;
    break;
    case 2:
      BITMASK = (1<<2);
      PORT = LOW;
    break;
    case 3:
      BITMASK = (1<<3);
      PORT = LOW;
    break;
    case 4:
      BITMASK = (1<<4);
      PORT = LOW;
    break;
    case 5:
      BITMASK = (1<<5);
      PORT = LOW;
    break;
    case 6:
      BITMASK = (1<<6);
      PORT = LOW;
    break;
    case 7:
      BITMASK = (1<<7);
      PORT = LOW;
    break;
    case 8:
      BITMASK = (1<<0);
      PORT = HIGH;
    break;
    case 9:
      BITMASK = (1<<1);
      PORT = HIGH;
    break;
    case 10:
      BITMASK = (1<<2);
      PORT = HIGH;
    break;
    case 11:
      BITMASK = (1<<3);
      PORT = HIGH;
    break;
    case 12:
      BITMASK = (1<<4);
      PORT = HIGH;
    break;
    case 13:
      BITMASK = (1<<5);
      PORT = HIGH;
    break;
    default:
    break;
  }
  // ******************************************
  // STEP 2: Turn on the IR-LED
  START_TIMER4;
  START_TIMER5;
  // ******************************************
  // STEP 3: As long as the IR beam is active (thus as long as Timer 4 is running),
  // we poll the associated sensor. Becomes true if IR light is detected
  bool lightSeen = false;
  if (PORT == LOW)    // inputs numbered 1..8
    while (TCCR4B) { if (!(PINL & BITMASK)) lightSeen = true; }
  else               // inputs numbered 9..14
    while (TCCR4B) { if (!(PINC & BITMASK)) lightSeen = true; }
  // Make sure all IR-LEDs are switched off and return
  PORTF = 0;
  PORTK = 0;
  // ******************************************
  // STEP 4: Store the result in the integrator.
  // To limit the effect of reflections, a blocked IR-beam counts more than detected beams.
  // If the IR-beam is blocked, add 5 (HIGH_STEP) to the integrator
  // but use HIGH_TRESHOLD to limit the maximum integrator value
  if ((!lightSeen) && (allSensors[sensor].integrator < HIGH_TRESHOLD))
    allSensors[sensor].integrator = allSensors[sensor].integrator + HIGH_STEP;
  // If the IR-beam is free, decrement the integrator
  if ((lightSeen) && (allSensors[sensor].integrator > LOW_TRESHOLD))  
    allSensors[sensor].integrator--;
  // ******************************************
  // Step 5: update the status for each individual IR-LED / IR Sensor pair
  if ((allSensors[sensor].integrator >= HIGH_TRESHOLD) && (allSensors[sensor].blocked == false)) { 
    allSensors[sensor].blocked = true;
    if (debugSerial) {
      Serial.print(sensor); 
      Serial.println(": Blocked"); 
    }
  }
  if ((allSensors[sensor].integrator <= LOW_TRESHOLD) && (allSensors[sensor].blocked == true)) {
    allSensors[sensor].blocked = false;
    if (debugSerial) {
      Serial.print(sensor); 
      Serial.println(": Free"); 
    }
  }
}


// ***********************************************************************************************************
// Initialise the timers
// ***********************************************************************************************************
void IR_Sensors::init_timers() {
  noInterrupts();
  // Timer 4
  STOP_TIMER4;
  TCCR4A = 0;
  OCR4A  = 8000 / KHZ;       // TOP value for the count
  TIMSK4 = (1 << OCIE4A);    // Call the ISR when the TCNT4 matches OCR4A
  // Timer 5
  STOP_TIMER5;
  TCCR5A = 0;
  TCCR5B = 0;
  OCR5A  = BURST_TIME * 16;  // TOP value for the count
  TIMSK5 = (1 << OCIE5A);    // Call the ISR when the TCNT5 matches OCR5A
  interrupts();
}


// ***********************************************************************************************************
// Timer ISRs
// ***********************************************************************************************************
// Timer 4 toggles the IR-LED.
ISR(TIMER4_COMPA_vect) {
  // Determine which port (PF or PK) and which bit to toggle (0..7)
  if (PORT == LOW) PORTF ^=  BITMASK;
    else PORTK ^=  BITMASK;
}

// Timer 5 stops timer 4
ISR(TIMER5_COMPA_vect) {
  STOP_TIMER4;
  STOP_TIMER5;
}


// ***********************************************************************************************************
// Functions to determine the Feedback bit
// ***********************************************************************************************************
/*
void IR_Sensors::busy(uint8_t sensor) {
  // If a IR-sensor is busy, we set the value to 10.
  // Once it is free, every call to free will substract 1, till it is 0
  sensorStatus[sensor] = 10;
}

void IR_Sensors::free(uint8_t sensor) {
  if (sensorStatus[sensor] > 0) sensorStatus[sensor]--;
}

bool IR_Sensors::feedbackBit(uint16_t mask) {
  uint16_t bits = 0;
  uint8_t bitValue = 0;
  for (uint8_t i=0; i < (MAX_SENSORS); i++) {
    bitValue = 0;
    if (sensorStatus[i] > 0) bitValue = 1;
    bits |= (bitValue << i);
  }
  bits = bits & mask;
  if (bits > 0) return 0;
    else return 1;
}
*/


bool IR_Sensors::feedbackBit(uint16_t mask) {
  uint16_t bits = 0;
  uint8_t bitValue = 0;
  for (uint8_t i=0; i < (MAX_SENSORS); i++) {
    bitValue = 0;
    if (allSensors[i].blocked) bitValue = 1;
    bits |= (bitValue << i);
  }
  bits = bits & mask;
  Serial.println(bits, BIN);
  if (bits > 0) return 0;
    else return 1;
}
