all: build upload serial

init:
	platformio -f init --board=pro16MHzatmega328 
build: 
	platformio -f run 

upload:
	platformio -f run -t upload --upload-port /dev/ttyUSB$(tty)

clean:
	platformio -f run -t clean

serial:
	platformio serialports monitor -p  /dev/ttyUSB$(tty)

