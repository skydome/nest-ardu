#include "Arduino.h"
#include <Wire.h>
#include <RHReliableDatagram.h>
#include <RH_RF22.h>
#include <SPI.h>

#define RF_SERVER_ADDRESS 0

static const char intPin = 9;

void receiveEvent(int bytes);
void requestEvent();

RH_RF22 driver;
RHReliableDatagram manager(driver, RF_SERVER_ADDRESS);

void setup() {
  pinMode(intPin, OUTPUT);

  Serial.begin(9600);
  
  Wire.begin(0x03); // start I2C on 0x03
  Wire.onRequest(requestEvent);

  if (manager.init()){
	  Serial.println("init succeeded!"); 
  }
  else {
	  Serial.println("init failed!"); 
  }
}

long counter = 1;

void loop() {
  counter++;
 // Serial.println(counter);
  digitalWrite(intPin, HIGH);
  delay(20);
  digitalWrite(intPin, LOW);
}

void requestEvent() {
  // Both the Arduino and RPi are little-endian, no conversion needed...
  Wire.write("securitas");
}
