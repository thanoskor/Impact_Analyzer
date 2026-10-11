#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void SPI_Master_Init(void);
uint8_t SPI_Master_Transfer(uint8_t data);

// Drive CS low
// Needs to have been activated first via tha SPI_Master_Init
static inline void CS_LOW(void) {
    PORTB &= ~(1 << PORTB2);
}

// Drive CS high
// Needs to have been activated first via tha SPI_Master_Init
static inline void CS_HIGH(void) {
    PORTB |= (1 << PORTB2);
}

#endif // SPI_H