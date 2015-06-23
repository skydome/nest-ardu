all: clean build

build: 
	 ano build -m pro --cpu 16MHzatmega328

upload:
	ano upload -m pro --cpu 16MHzatmega328 -p /dev/ttyUSB$(tty)

clean:
	ano clean

serial:
	ano serial -p /dev/ttyUSB$(tty)

