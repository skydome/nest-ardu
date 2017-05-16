#include "Arduino.h"
#include <Wire.h>

static const char intPin = 8;

void receiveEvent(int bytes);
void requestEvent();

void setup()
{
  pinMode(intPin, OUTPUT);

	Serial.begin(9600);
  Wire.begin(0x03);
  Wire.onRequest(requestEvent);
}

long counter = 1;

void loop() {

  counter++;
  digitalWrite(intPin, HIGH);
  delay(20);
  digitalWrite(intPin, LOW);
}

void requestEvent() {
  // Both the Arduino and RPi are little-endian, no conversion needed...
  Wire.write((uint8_t *)&counter, sizeof(counter));
}
