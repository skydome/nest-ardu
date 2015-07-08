#include <TimedAction.h>
#include <RF22ReliableDatagram.h>
#include <RF22.h>
#include <SPI.h>
#include "DHT.h"

#define DHTPIN 9
#define DHTTYPE DHT11

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 0

RF22ReliableDatagram manager(CLIENT_ADDRESS);
DHT dht(DHTPIN, DHTTYPE);

struct Pin {
  const int PIN;
  int state;
  byte counter[5];
  byte failed[2];
  byte fixed[2];
};

Pin working =    {8, LOW, {0, 0, 0, 0, 0}, {8, 0}, {8, 1}};
Pin pulse =      {7, LOW, {7, 0, 0, 0, 0}, {    }, {    }};
Pin weft =       {6, LOW, {0, 0, 0, 0, 0}, {6, 0}, {6, 1}};
Pin warp =       {5, LOW, {0, 0, 0, 0, 0}, {5, 0}, {5, 1}};
Pin manualStop = {4, LOW, {0, 0, 0, 0, 0}, {4, 0}, {4, 1}};

// every 30 sec, send pulse count to nest
const int pulseCountInterval = 5000;
// check health every 1 sec
const int healthCheckInterval = 1000;

const int reportInterval = 1000;

bool reportMe = false;

uint8_t buf[RF22_MAX_MESSAGE_LEN];

TimedAction pulseCalculator = TimedAction(pulseCountInterval, calculatePulse);
TimedAction healthCheckCalculator = TimedAction(healthCheckInterval, healthCheck);
TimedAction reporter = TimedAction(reportInterval, report);

void setup() {
  dht.begin();
  pinMode(working.PIN, INPUT);
  pinMode(pulse.PIN, INPUT);
  pinMode(weft.PIN, INPUT);
  pinMode(warp.PIN, INPUT);
  pinMode(manualStop.PIN, INPUT);
  Serial.begin(9600);
  Serial.println("Starting ant...");
  if (manager.init())
    Serial.println("init success for ant");
  else
    Serial.println("init failed for ant");
}

void loop() {
  increasePulse();
  healthCheckCalculator.check();
  pulseCalculator.check();
  if (reportMe)
    reporter.check();
}

void healthCheck() {
  checkError(weft, HIGH, LOW);
  checkError(warp, HIGH, LOW);
  checkError(manualStop, HIGH, LOW);
  checkError(working, LOW, HIGH);
  if (digitalRead(working.PIN) == HIGH) {
    weft.state = LOW;
    warp.state = LOW;
    manualStop.state = LOW;
  }

}

void report() {
  Serial.print("Working : ");
  Serial.println(digitalRead(working.PIN));
  Serial.print("pulse : ");
  Serial.println(digitalRead(pulse.PIN));
  Serial.print("weft : ");
  Serial.println(digitalRead(weft.PIN));
}

void increasePulse() {
  if (digitalRead(working.PIN) == HIGH) {
    int tempPulseState =  digitalRead(pulse.PIN);
    if (tempPulseState == LOW && pulse.state != tempPulseState) {
      pulse.counter[1]++;
    }
    pulse.state = tempPulseState;
  }
}

void calculatePulse() {
  Serial.print("sending pulse count : ");
  Serial.println(pulse.counter[1]);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  float hic = dht.computeHeatIndex(t, h, false);
  pulse.counter[2] = h;
  pulse.counter[3] = t;
  pulse.counter[4] = hic;
  sendReliableMessage(pulse.counter, true);
  pulse.counter[2] = 0;
  pulse.counter[3] = 0;
  pulse.counter[4] = 0;
  pulse.counter[1] = 0;
}

void checkError(struct Pin &pin, int high, int low) {
  int tempState =  digitalRead(pin.PIN);
  if (tempState == high && pin.state != tempState) {
    Serial.print("error : ");
    Serial.println(pin.PIN);

    sendReliableMessage(pin.failed, false);
  } else if (tempState == low && pin.state != tempState) {
    Serial.print("fixed : ");
    Serial.println(pin.PIN);
    sendReliableMessage(pin.fixed, false);
  }
  pin.state = tempState;
}

void sendReliableMessage ( byte data[], bool pulse) {

  Serial.print("sending data[0]:");
  Serial.println(data[0]);
  Serial.print("sending data[1]:");
  Serial.println(data[1]);

  int length = 2;

  if(pulse){
    length = 5;
    Serial.print("sending data[2]:");
    Serial.println(data[2]);
    Serial.print("sending data[3]:");
    Serial.println(data[3]);
    Serial.print("sending data[4]:");
    Serial.println(data[4]);
  }

  if (manager.sendtoWait(data, length, SERVER_ADDRESS))  {
  //  uint8_t len = sizeof(buf);
  //  uint8_t from;
  //  if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
  //  {
  //    Serial.print("got reply from : 0x");
  //    Serial.print(from, HEX);
  //    Serial.print(": ");
  //    Serial.println((char*)buf);
  //  }
  //  else
  //  {
  //    Serial.println("No reply, is rf22_reliable_datagram_server running?");
  //  }
  }
  else
    Serial.println("sendtoWait failed");
}
