all:
	arduino-builder -build-path build -fqbn archlinux-arduino:avr:nano:cpu=atmega328 -hardware /usr/share/arduino/hardware -tools /usr/bin rakuchord.ino

flash:
	avrdude -C/etc/avrdude.conf -v -patmega328p -carduino -P/dev/ttyUSB0 -b57600 -D -Uflash:w:build/rakuchord.ino.hex:i
