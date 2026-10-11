#include <avr/io.h>
#include "spi.h"

void SPI_Master_Init(void) {
    // 1. Set MOSI (PB3 / Pin 11), SCK (PB5 / Pin 13), and SS (PB2 / Pin 10) as outputs
    DDRB |= (1 << DDB3) | (1 << DDB5) | (1 << DDB2);
    
    // Set MISO (PB4 / Pin 12) as input
    DDRB &= ~(1 << DDB4);
    
    // Keep SS (PB2) high initially
    PORTB |= (1 << PORTB2);

    // 2. Enable SPI, set as Master, clock rate fosc/16
    // (CPOL=0, CPHA=0 for SPI Mode 0, MSB First)
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

uint8_t SPI_Master_Transfer(uint8_t data) {
    // Write data to SPI Data Register to start transmission
    SPDR = data;

    // Wait for transmission complete (SPIF bit set in SPSR)
    while (!(SPSR & (1 << SPIF)));

    // Return received data
    return SPDR;
}

