// 2020-05-25 AP
// Test serial communication between an Arduino Mega and an UNO/Nano running GRBL.
// The UNO/Nano runs GRBL, while the MEGA just copies characters between PC/MAC and UNO
//
// The MEGA is connected via USB to the PC/MAC
// The UNO/Nano and MEGA are connected via the TX->RX2 and RX<-TX2 pins
// On the MEGA Serial=USB; Serial2 are the pins TX2 & RX2 (UNO/Nano with GRBL)
//
// Powering the UNO/Nano:
// The 5V pins of the UNO/Nano and MEGA are connected
// The GND pins of the UNO/Nano and MEGA are connected
// The UNO/Nano is NOT connected via USB to a PC/MAC
//
// Type on the Arduino serial monitor program the following commands:
// - $$ = view settings
// - x10 = Move X stepper to 10
// - x20 = Move X stepper to 20
// - x0  = Move X stepper back to 0

void setup() {
  // put your setup code here, to run once:
  // initialize serial:
  Serial.begin(115200);
  Serial2.begin(115200);
}

void loop() {
  // read from port 0 (USB), send to port 2 (UNO/Nano):
  if (Serial.available()) {
    char inByte = Serial.read();
    Serial2.print(inByte);
  }
  // read from port 2, send to port 0:
  if (Serial2.available()) {
    char inByte = Serial2.read();
    Serial.print(inByte);
  }
}
