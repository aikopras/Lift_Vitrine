//******************************************************************************************************
//
// file:      IR-Lock.h
// author:    Aiko Pras
// history:   2022-01-25 V1.0.0 ap initial version
//
// purpose:    Control of an IR lock (LED plus sensor)
//
// The main sketch calls only:
// - init_timers()
// - checkStatus(i)
// - feedbackBit(mask)

//
// We have MAX_LOCKS individual locks. Each lock consists of an IR-LED and IR-Sensor.3
// For each individual lock we maintain a boolean variable 'blocked" and an integrator.
// The integrator filters spikes: an individual measurement will not immediately lead to a response.  
//
//******************************************************************************************************
#pragma once
#include "hardware.h" 


class IR_Locks {
  public:
    #define MAX_LOCKS        16       // Number of IR locks we could support
    #define LOW_TRESHOLD      0       // Result becomes LOW if integrator reaches this value (default: 0)
    #define HIGH_TRESHOLD    10       // Number of successive "ticks" before integrator is HIGH
    #define HIGH_STEP         5       // Number of "ticks" to increase after a "hit"

    IR_Locks();                       // Constructor for initialisation
    void init_timers();
    void checkStatus(uint8_t lock);   // 
    bool feedbackBit(uint16_t mask); 

  private:
    class Single_Lock {
      public:
        uint8_t integrator;          // Integrator values range from LOW_TRESHOLD to HIGH_TRESHOLD
        bool blocked;                // The previous / most recent stable button position
    }; 
   
   Single_Lock allLocks[MAX_LOCKS]; // Array, one element per lock

};
