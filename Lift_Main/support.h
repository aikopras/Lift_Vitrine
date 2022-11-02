/*******************************************************************************************************
File:      support.h
Author:    Aiko Pras
History:   2021/04/24 AP Version 1.0


Purpose:   Support objects for the onboard LEDs and the LCD display
           Objects defined here have similar access as main to objects elsewhere 

******************************************************************************************************/
#pragma once

class led_class {
  public:
    led_class();          // Constructor for initialisation
    void toggle(uint8_t nummer);
};


class display_class {
  public:
    void init();              // Initialisation
    void show();              // Writes position and other info on the LCD display
    void homing();            // Writes on the LCD display that Homing started
};

/*****************************************************************************************************/
// Definition of external objects, which are declared here but used by main 
extern display_class   lcd_display;
extern led_class       led;
