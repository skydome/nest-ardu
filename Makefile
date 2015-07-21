all: build upload serial

init:
	platformio -f init --board=pro16MHzatmega328 
	wget http://www.airspayce.com/mikem/arduino/RF22/RF22-1.40.zip && unzip RF22-1.40.zip -d lib/ && rm RF22-1.40.zip
	wget http://playground.arduino.cc/uploads/Code/TimedAction-1_6.zip && unzip TimedAction-1_6.zip -d lib/ && rm TimedAction-1_6.zip
	sed -i 's/WProgram.h/Arduino.h/g' lib/TimedAction/TimedAction.h
build: 
	platformio -f run 

upload:
	platformio -f run -t upload --upload-port /dev/ttyUSB$(tty)

clean:
	platformio -f run -t clean

serial:
	platformio serialports monitor -p  /dev/ttyUSB$(tty)

