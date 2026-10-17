#include <avr/io.h>
#include "spi.h"

// Sets up SPI in mode 3
void SPI_Master_Init(void) {
    // //clear the prspi bit in the power reduction register
    // PRR &= ~(1 << PRSPI);

    // eneblae internal pullups on SS and SCK(so they dont get driven low when set as outputs that creates a false spike at startup)
    // it also keeps them high after
    PORTB |= (1 << PORTB2) | (1 << PORTB5);

    // 1. Set MOSI (PB3 / Pin 11), SCK (PB5 / Pin 13), and SS (PB2 / Pin 10) as outputs
    DDRB |= (1 << DDB3) | (1 << DDB5) | (1 << DDB2);
    
    // Set MISO (PB4 / Pin 12) as input
    DDRB &= ~(1 << DDB4);
    
    // 2. Enable SPI, set as Master, clock rate fosc/16
    // Mode 3: Set CPOL=1 (idle clock high) and CPHA=1 (sample on trailing edge)
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA) | (1 << SPR0);
}

uint8_t SPI_Master_Transfer(uint8_t data) {
    // Write data to SPI Data Register to start transmission
    SPDR = data;

    // Wait for transmission complete (SPIF bit set in SPSR)
    while (!(SPSR & (1 << SPIF)));

    // Return received data
    return SPDR;
}

