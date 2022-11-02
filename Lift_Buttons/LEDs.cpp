/*******************************************************************************************************
File:       LEDs.cpp
Author:     Aiko Pras
History:    2021/04/24 AP Version 1.0 First version, based on the DDC_LED library for a single LED

purpose:    Control the LEDs of the panel with the buttons that operate the loclift.

******************************************************************************************************/
#include <Arduino.h>
#include "LEDs.h"


// The constructor below initialises the object
Led_series::Led_series(const uint8_t* pinNumbers, const uint8_t ledCnt) {
  myLedCnt = ledCnt;
  for (int i = 0; i < ledCnt; i++)  {
    pinMode(pinNumbers[i], OUTPUT);
    myLeds[i].pin_nr = pinNumbers[i];
    myLeds[i].mode = alwaysOff;
    myLeds[i].last_time = millis();
  }
}


void Led_series::turn_on(const uint8_t ledNumber) { 
  digitalWrite(myLeds[ledNumber].pin_nr, HIGH);
  myLeds[ledNumber].mode = alwaysOn;
}


void Led_series::turn_off(const uint8_t ledNumber) { 
  digitalWrite(myLeds[ledNumber].pin_nr, LOW);
  myLeds[ledNumber].mode = alwaysOff;
}


// Single short flash. Ticks is an optional parameter with default value 5
void Led_series::flash(const uint8_t ledNumber, uint8_t ticks) {
  myLeds[ledNumber].ontime = ticks;                 // expressed in ticks of 100ms
  myLeds[ledNumber].count = 1;                      // single flash
  myLeds[ledNumber].mode = singleFlashSerie;
  myLeds[ledNumber].time_remain = myLeds[ledNumber].ontime;
  myLeds[ledNumber].flash_number_now = 1;
  digitalWrite(myLeds[ledNumber].pin_nr, HIGH);
}


// Continuous series of slow flashes
void Led_series::flashSlow(const uint8_t ledNumber) {
  myLeds[ledNumber].ontime = 5;                     // 0,5 sec
  myLeds[ledNumber].offtime = 5;                    // 0,5 sec
  myLeds[ledNumber].count = 1;                      // 1 flashes
  myLeds[ledNumber].pause = 5;
  myLeds[ledNumber].mode = neverStopFlashing;
  myLeds[ledNumber].time_remain = myLeds[ledNumber].ontime;
  myLeds[ledNumber].flash_number_now = 1;
  digitalWrite(myLeds[ledNumber].pin_nr, HIGH);
}


// Continuous series of fast flashes
void Led_series::flashFast(const uint8_t ledNumber) {
  myLeds[ledNumber].ontime = 1;                     // 0,1 sec
  myLeds[ledNumber].offtime = 1;                    // 0,2 sec
  myLeds[ledNumber].count = 1;                      // 1 flashes
  myLeds[ledNumber].pause = 2;
  myLeds[ledNumber].mode = neverStopFlashing;
  myLeds[ledNumber].time_remain = myLeds[ledNumber].ontime;
  myLeds[ledNumber].flash_number_now = 1;
  digitalWrite(myLeds[ledNumber].pin_nr, HIGH);  
}



void Led_series::update() {
  for (int i = 0; i < myLedCnt; i++)  {
    bool actionNeeded = true;
    // If the LED must be always ON or OFF, we do not need to update
    if (myLeds[i].mode == alwaysOn) actionNeeded = false; 
    if (myLeds[i].mode == alwaysOff) actionNeeded = false;           
    if (actionNeeded) {
      unsigned long current_time = millis();                    // Seems we do a series of flashes
      if ((current_time - myLeds[i].last_time) >= 100) {        // We only update the LED every 100 msec
        myLeds[i].last_time = current_time;   
        --myLeds[i].time_remain;                                // Another 100 msec passed
        if (myLeds[i].time_remain == 0) {                       // Do we need to change state?
          if (digitalRead(myLeds[i].pin_nr)) {                  // LED is currently ON (AVR Pin for LED is high)
            digitalWrite(myLeds[i].pin_nr, LOW);                // Change is needed, thus change LED to OFF 
            if (myLeds[i].flash_number_now != myLeds[i].count)  // We did not yet had all flashes of the series
              myLeds[i].time_remain = myLeds[i].offtime;        // So we will wait a normal off time
            else {                                              // We had the complete series of flashes
              if (myLeds[i].mode == neverStopFlashing) {        // We have to start a new series of flashes
                myLeds[i].time_remain = myLeds[i].pause;        // Next we will wait a longer pause
                myLeds[i].flash_number_now = 0;                 // Restart the counter for the required number of flashes
              }
              else myLeds[i].mode = alwaysOff;                  // We don't need to start a new series of flashes
            }
          }
          else {                                                // LED is OFF
            digitalWrite(myLeds[i].pin_nr, HIGH);               // Change is needed, thus change LED to ON
            myLeds[i].time_remain = myLeds[i].ontime;           // Next will be a certain time on
            myLeds[i].flash_number_now++;                       // Increment the number of blinks we did
          }
        }
      }
    }
  }
}
