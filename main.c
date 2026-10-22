#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"
#include "usart.h"
#include "twi.h"
#include <stdio.h>

#define BMI160_SLA_ADDR 0x69

int main(void) {

    USART_Init(MYUBRR);
    TWI_Init(1);
    _delay_ms(100);
    uint8_t d = 0;
    while (1) {
        TWI_Write_Read_Transaction_Synchronous(BMI160_SLA_ADDR, 0x00, &d, 1);
        if (d != 0) {
            USART_Transmit(d);
        }
        _delay_ms(1000);
    }
    
}
