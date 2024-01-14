//******************************************************************************************************
//
// Test sketch for the Lift Decoder board
// To check communication between the main processor (MEGA 2560) and GRBL processor (MEGA 328)
// To configure GRBL variables and check the stepper driver(s) and motor(s)
// 
// The connector on the Lift Decoder board labelled Monitor 2560 should be connected to a Serial to USB
// connector, which in turn connects to the PC/Mac running the Raduino IDE.
// Note that not all USB to Serial connectors turned out to operate reliable.
// The baudrate is 115200.
//
// Type on the Arduino serial monitor program the following commands:
// - $$ = view settings
// - x10 = Move X stepper to 10
// - x20 = Move X stepper to 20
// - x0  = Move X stepper back to 0
//
// 2020/08/03 AP
//
//******************************************************************************************************
#include "Arduino.h"


void setup() {
  // Turn on the LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
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
