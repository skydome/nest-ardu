
#include <RHReliableDatagram.h>
#include <RH_RF24.h>
#include <SPI.h>
#include <Wire.h>

#define SLAVE_ADDRESS 0x60
#define RPI_ADDRESS 0x10
#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

RH_RF24 driver;
RHReliableDatagram manager(driver, SERVER_ADDRESS);

void requestEvent();

struct Message {
	uint8_t data[10];
};

const int indexMax = 10;

enum QueueSelect {
	BLUE,
	GREEN
};

QueueSelect queueSelect = BLUE;

struct Messages {
	Message queue[indexMax];
	int index = 0;
	const QueueSelect type;
	bool ready2write = true;
	Messages(QueueSelect type): type(type) {}
};

Messages messagesBlue = Messages { BLUE };

Messages messagesGreen = Messages { GREEN};

void setup()
{
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);  
  
  if (manager.init())
    Serial.println("Nest init success!");
  else
    Serial.println("Nest init failed!");

}

uint8_t data[] = "Ack";
uint8_t buf[10];

void requestEvent() {

  Messages *messages;	
  
  if(!messagesGreen.ready2write && messagesGreen.index > 0) {
	messages = &messagesGreen;
  }else if(!messagesBlue.ready2write && messagesBlue.index > 0){
	messages = &messagesBlue;
  } 

  Wire.write(messages->index);

  for(int i=0; i<messages->index; i++)
  	Wire.write((char*)messages->queue[i].data);
}	

void loop()
{
  if (manager.available())
  {
    Messages *messages;
    switch(queueSelect){
	    case BLUE:
		    messages = &messagesBlue;
		    break;
	    case GREEN:
		    messages = &messagesGreen;
		    break;
    }

    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      Serial.print("got request from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);

      if(messages->index >= indexMax) {
		
	      QueueSelect type = messages->type;
	      messages-> ready2write = false;
	      messages-> index = 0;		
	      switch(type){
		      case BLUE:
			      messages = &messagesGreen;
			      queueSelect = GREEN;
			      break;
		      case GREEN:
			      messages = &messagesBlue;
			      queueSelect = BLUE;
			      break;
	      }
      }

      messages->queue[messages->index] = Message {&buf}; 
      messages->index = messages->index++;


      if (!manager.sendtoWait(data, sizeof(data), from))
        Serial.println("sendtoWait failed");
    }
  }
}
