MCU=atmega328p
F_CPU=16000000UL
PORT=/dev/ttyUSB0

CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-mmcu=$(MCU) -DF_CPU=$(F_CPU) -O2 -g -Wall

SRCS=main.c spi.c usart.c

all: main.hex

main.elf: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o main.elf

main.hex: main.elf
	$(OBJCOPY) -O ihex -R .eeprom main.elf main.hex

# Generates readable assembly (.s files per source and a merged .lss disassembly)
asm: main.elf
	$(CC) $(CFLAGS) -S main.c -o main.s
	$(CC) $(CFLAGS) -S spi.c -o spi.s
	$(CC) $(CFLAGS) -S usart.c -o usart.s
	$(OBJDUMP) -d -S main.elf > main.lss

flash: main.hex
	avrdude -c arduino -P $(PORT) -b 57600 -p m328p -U flash:w:main.hex:i

clean:
	rm -f *.elf *.hex *.s *.lss