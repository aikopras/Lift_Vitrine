/*******************************************************************************************************
File:      LEDs.h
Author:    Aiko Pras
History:   2021/04/24 AP Version 1.0  First version, based on the DDC_LED library for a single LED

purpose:   Control the LEDs on the panel with the buttons that operate the loclift.
           Individual LEDs can be switched on, switched off or put in flashing mode.

For flashing mode, the following attributes  can be set:
- ontime:  (0..255) time the LED is on (in 100 msec steps)
- offtime: (0..255) time between to flashes 
- count:   (0..255) the total number of flashes (within a single serie)
- mode:    (singleFlashSerie, neverStopFlashing) whether we have a single series of flashes, or repeating  
- pause:   (0..255) time between two series of flashes (in case of continuous flashing)
All times are in steps of 100 msec.
                                                    count
                   ontime
    ----+          +----+         +----+         +----+                                    +----+
        |          |    |         |    |         |    |                                    |    |
        |          |    |         |    |         |    |                                    |    |
        +----------+    +---------+    +---------+    +------------------------------------+    +----
                          offtime                                      pause

                                                                             mode = neverStopFlashing

******************************************************************************************************/
#pragma once
#define MAXLEDS 14   // The number of LEDs we have

class Led_series {
  
  public:
    // Constructor for initialisation
    Led_series(const uint8_t* pinNumbers, const uint8_t ledCnt); 
    // Schould be called from the main loop as often as possible
    void update(void);
    // Generic methods
    void turn_on(const uint8_t ledNumber);
    void turn_off(const uint8_t ledNumber);
    void flash(const uint8_t ledNumber, uint8_t ticks = 5);        // Single Flash
    void flashSlow(const uint8_t ledNumber);    // Continuous series of slow flashes
    void flashFast(const uint8_t ledNumber);    // Continuous series of fast flashes

//================================================================================
  private:
    enum ledMode_t{
        alwaysOn,
        alwaysOff,
        singleFlashSerie,
        neverStopFlashing
      };             
      
    struct Led_t{ 
      uint8_t pin_nr;                 // Hardware pin to which this specific LED is connected
      uint8_t ontime;                 // Time (in 100 ms steps) the LED should be on       (see figure above)
      uint8_t offtime;                // Time (in 100 ms steps) the LED should be off      (see figure above)
      uint8_t count;                  // Number of flashes before a pause                  (see figure above)
      uint8_t pause;                  // Longer LED off time: between a series of flashes  (see figure above)
      uint8_t flash_number_now;       // Number of flashes thusfar
      uint8_t time_remain;            // Remaining time before this LED status changes. In Ticks (100ms)
      unsigned long last_time;        // time in msec since we last updated this LED

      ledMode_t mode;                 // The states this specific LED may be in
    };

    Led_t myLeds[MAXLEDS];            // An Array with all LEDs
    uint8_t myLedCnt;                 // The actual number of LEDs we have
};
