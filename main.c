#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"
#include "usart.h"
#include "twi.h"
#include <stdio.h>


int main(void) {

    USART_Init(MYUBRR);
    TWI_Init(1);
    _delay_ms(100);
    uint8_t c = 0;
    while (1) {
        TWI_Master_Tx_Synchronous(0b01010101, c);
        c++;
        _delay_ms(50);
    }
    
}
