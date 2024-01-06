//******************************************************************************************************
//
// file:      IR-Sensor.h
// author:    Aiko Pras
// history:   2022-01-25 V1.0.0 ap initial version
//
// purpose:   Control of an IR sensor (LED plus sensor)
//
// The main sketch calls only:
// - init_timers()
// - checkStatus(i)
// - feedbackBit(mask)
//
// The software allows a maximum number of MAX_SENSORS sensors. 
// Each sensor consists of an IR-LED and IR-Sensor.
// For each individual sensor we maintain a boolean variable 'blocked" and an integrator.
// The integrator filters spikes: an individual measurement will not immediately lead to a response.  
//
//******************************************************************************************************
#pragma once
#include "hardware.h" 


class IR_Sensors {
  public:
    #define MAX_SENSORS      16         // Number of IR sensors we could support
    #define LOW_TRESHOLD      0         // Result becomes LOW if integrator reaches this value (default: 0)
    #define HIGH_TRESHOLD    10         // Number of successive "ticks" before integrator is HIGH
    #define HIGH_STEP         5         // Number of "ticks" to increase after a "hit"

    IR_Sensors();                       // Constructor for initialisation
    void init_timers();
    void checkStatus(uint8_t sensor);  
    bool feedbackBit(uint16_t mask); 

  private:
    class Single_Sensor {
      public:
        uint8_t integrator;          // Integrator values range from LOW_TRESHOLD to HIGH_TRESHOLD
        bool blocked;                // The previous / most recent stable button position
    }; 
   
   Single_Sensor allSensors[MAX_SENSORS]; // Array, one element per sensor

};
