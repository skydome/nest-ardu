#include "Arduino.h"
#include <Wire.h>

static const char intPin = 9;

void receiveEvent(int bytes);
void requestEvent();

void setup()
{
  pinMode(intPin, OUTPUT);

  Serial.begin(9600);
  Wire.begin(0x03);
  Wire.onRequest(requestEvent);
  Serial.println("done!");
}

long counter = 1;

void loop() {
  counter++;
  Serial.println(counter);
  digitalWrite(intPin, HIGH);
  delay(20);
  digitalWrite(intPin, LOW);
}

void requestEvent() {
  // Both the Arduino and RPi are little-endian, no conversion needed...
  Wire.write("securitas");
}
