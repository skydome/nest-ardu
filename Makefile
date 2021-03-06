all: build upload serial

tty = 0
baud = 9600

init:
	platformio -f init --board=pro16MHzatmega328 
build: 
	platformio -f run 

upload:
	platformio -f run -t upload --upload-port /dev/ttyUSB$(tty)

clean:
	platformio -f run -t clean

serial:
	PYTHONIOENCODING=utf-8	platformio serialports monitor -p  /dev/ttyUSB$(tty) -b $(baud)

