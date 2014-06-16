#include <TimedAction.h>
#include <RF22ReliableDatagram.h>
#include <RF22.h>
#include <SPI.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 0

// Singleton instance of the radio
RF22ReliableDatagram manager(CLIENT_ADDRESS);

struct Pin {
  const int PIN;
  int state;
  byte counter[2];
  byte failed[2];
  byte fixed[2];
};

Pin working = {8, LOW, {0, 0}, {    }, {    }};
Pin pulse =   {7, LOW, {0, 0}, {    }, {    }};
Pin warp =    {6, LOW, {0, 0}, {6, 0}, {6, 1}};
Pin weft =    {5, LOW, {0, 0}, {5, 0}, {5, 1}};
Pin rope =    {4, LOW, {0, 0}, {4, 0}, {4, 1}};
Pin unknown = {3, LOW, {0, 0}, {3, 0}, {3, 1}};

// every 10 sec, send pulse count to nest
const int pulseCountInterval = 10000;
// check health every 1 sec
const int healthCheckInterval = 1000;

uint8_t buf[RF22_MAX_MESSAGE_LEN];

TimedAction pulseCalculator = TimedAction(pulseCountInterval, calculatePulse);
TimedAction healthCheckCalculator = TimedAction(healthCheckInterval, healthCheck);
void setup() {
  pinMode(working.PIN, INPUT); // sets the pin as output
  pinMode(pulse.PIN, INPUT); // sets the pin as output
  pinMode(warp.PIN, INPUT); // sets the pin as output
  pinMode(weft.PIN, INPUT); // sets the pin as output
  pinMode(rope.PIN, INPUT); // sets the pin as output
  pinMode(unknown.PIN, INPUT);
  Serial.begin(9600);
  if (manager.init())
    Serial.println("init success");
  else
    Serial.println("init failed");
}

void loop() {
  increasePulse();
  healthCheckCalculator.check();
  pulseCalculator.check();
}

void healthCheck() {
  checkError(warp);
  checkError(weft);
  checkError(rope);
  checkError(unknown);
  if (digitalRead(working.PIN) == HIGH) {
    warp.state = LOW;
    weft.state = LOW;
    rope.state = LOW;
    unknown.state = LOW;
  }
}

void increasePulse() {
  int tempPulseState =  digitalRead(pulse.PIN);
  if (tempPulseState == HIGH && pulse.state != tempPulseState) {
    pulse.counter[1]++;
  }
  pulse.state = tempPulseState;
}

void calculatePulse() {
  Serial.print("sending pulse count : ");
  Serial.println(pulse.counter[1]);
  sendReliableMessage(pulse.counter);
  pulse.counter[1] = 0;
}

void checkError(struct Pin &pin) {
  int tempState =  digitalRead(pin.PIN);
  if (tempState == HIGH && pin.state != tempState) {
    Serial.print("error : ");
    Serial.println(pin.PIN);
    sendReliableMessage(pin.failed);
  } else if (tempState == LOW && pin.state != tempState) {
    Serial.print("fixed : ");
    Serial.println(pin.PIN);
    sendReliableMessage(pin.fixed);
  }
  pin.state = tempState;
}

void sendReliableMessage ( byte data[]) {

  if (manager.sendtoWait(data, 2, SERVER_ADDRESS))  {
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("No reply, is rf22_reliable_datagram_server running?");
    }
  }
  else
    Serial.println("sendtoWait failed");
}


