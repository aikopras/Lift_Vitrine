// ***********************************************************************************************************
// File:       timers.h
// Author:     Aiko Pras
// history:    2023-12-31 V1.0.0 ap initial version
// 
// purpose:    Has all the low-level code to control generation of an IR-Beam
//
// Uses Timer 4 and Timer 5, as well as (optional) GPIOR0 and GPIOR1
//
//
//
#include "config.h" 



// ***********************************************************************************************************
// Do not change below this line
// ***********************************************************************************************************
#define STOP_TIMER4  TCCR4B = 0;
#define STOP_TIMER5  TCCR5B = 0;
#define START_TIMER4 TCCR4B = (1<<WGM42) | (1<<CS40)
#define START_TIMER5 TCCR5B = (1<<WGM52) | (1<<CS50)

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
