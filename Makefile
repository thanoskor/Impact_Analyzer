MCU=atmega328p
F_CPU=16000000UL
PORT=/dev/ttyUSB0

CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os

all: main.hex

main.elf: main.c
	$(CC) $(CFLAGS) main.c spi.c usart.c -o main.elf

main.hex: main.elf
	$(OBJCOPY) -O ihex -R .eeprom main.elf main.hex

flash: main.hex
	avrdude -c arduino -P $(PORT) -b 57600 -p m328p -U flash:w:main.hex:i

clean:
	rm -f *.elf *.hex
