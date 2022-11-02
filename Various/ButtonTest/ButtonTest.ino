// Test-Sketch for the Lift buttons
// Press once is LED on; press again is LED off

// mit Entprellung der Taster und Aufteilung in Eingabe - Verarbeitung - Ausgabe
// mit Flankenerkennung im Eingabeblock

#include <MoToButtons.h>

const byte LEDPinNr[]      = { 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67 };
const byte buttonPinNr []  = { 49, 48, 47, 46, 45, 44, 43, 42, 37, 36, 35, 34, 33, 32 };
const byte numberOfButtons = sizeof(buttonPinNr);

button_t getHW( void ) {
  // Einlesen der Tasterstates
button_t tasterTemp = 0;
  for (byte i = 0; i < numberOfButtons; i++) {
    bitWrite( tasterTemp,i,!digitalRead(buttonPinNr[i]) );     // Fragt den Taster ab und merkt sich den Status
  }
  return tasterTemp;
}

MoToButtons Taster1( getHW, 50, 500 );  // 50ms Debounce time, 500ms as difference between short and long press.

void setup()
{
  Serial.begin(115200);
  for (int i = 0; i < numberOfButtons; i++)  {
    pinMode(LEDPinNr[i], OUTPUT);
    pinMode(buttonPinNr[i], INPUT_PULLUP);
  }
  Serial.println("Start loop");
}


void loop() {
  //--------------------------------------------------------
  // Block "Eingabe": Taster entprellt einlesen und Startzeit merken
  Taster1.processButtons();
  // Ende Block "Eingabe"
  //--------------------------------------------------------
  
  // Block "Verarbeitung / Ausgabe": TasterStellung auswerten und Aktion durchführen
  for (byte i = 0; i < numberOfButtons; i++) {
    // Mit dieser Bedingung wird mit steigender Flanke (also beim Drücken des Tasters) geschaltet
    // -> beim Drücken rote Led toggeln
 //   if ( Taster1.longPress(i) ) {
 //     digitalWrite(LEDrotPinNr[i], !digitalRead(LEDrotPinNr[i]));      // rote LED einschalten
 //     Serial.print(" Langer Tastendruck: "); Serial.println(i);
 //   }

    // Mit dieser Bedingung wird mit fallender Flanke (also beim Loslassen des Tasters) geschaltet
    // -> beim Loslassen gruene Led toggeln
    if ( Taster1.shortPress(i) ) {
      digitalWrite(LEDPinNr[i], !digitalRead(LEDPinNr[i]));      // rote LED ausschalten
      Serial.print(" Kurzer Tastendruck: "); Serial.println(i);
    }
  }
  // Ende Block "Verarbeitung / Ausgabe"

} // End loop
