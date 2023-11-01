//******************************************************************************************************
//
// Test sketch for the Lift decoder board
// To check communication between the main processor (MEGA2560) and GRBL processor (MEGA 328)
// To check to stepper driver and motor
// 
// The USB-UART converter (monitor) is connected to serial 0, baudrate should be 115200
// The MEGA328 runnung GRBL is connected to serial 2, baudrate is also 115200
//
// Type on the Arduino serial monitor program the following commands:
// - $$ = view settings
// - x10 = Move X stepper to 10
// - x20 = Move X stepper to 20
// - x0  = Move X stepper back to 0
//
// 2020/08/03 AP - 2023/11/01 AP
//
//******************************************************************************************************
#include "Arduino.h"


void setup() {
  // Turn on the LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  // initialize serial:
  Serial.begin(115200);    // Monitor = USB UART-USB converter
  Serial2.begin(115200);   // GRBL - MEGA 328 
}


void loop() {
  // read from monitor port , send to GRBL port:
  if (Serial.available()) {
    char inByte = Serial.read();
    Serial2.print(inByte);
  }
  // read from GRBL port, send to monitor port:
  if (Serial2.available()) {
    char inByte = Serial2.read();
    Serial.print(inByte);
  }
}
