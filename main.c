#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"
#include "usart.h"

int main(void) {
    
    USART_Init(MYUBRR);    
    SPI_Master_Init();

    while (1) {
        SPI_Master_Transfer(0b10101001);
        _delay_ms(100); // software debounce
        
        USART_Transmit_String("Test String, Serial Terminal WORKS YAY");
    }
}
