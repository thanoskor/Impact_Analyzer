#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"
#include "usart.h"
#include <stdio.h>


int main(void) {
    
    

    USART_Init(MYUBRR);    
    SPI_Master_Init();

    _delay_ms(10); // wait for the ibm160 to be ready

    uint8_t chip_id = 1; //from the ibm160 datasheet reading reg 0x00 i should get 209

    CS_LOW();
    CS_HIGH();

    CS_LOW();
    SPI_Master_Transfer(0x07 | 0x80); // Dummy read register
    chip_id = SPI_Master_Transfer(0x00);
    CS_HIGH();

    _delay_ms(2);
  

    USART_Transmit(chip_id);
    while (1) {
        CS_LOW();
        SPI_Master_Transfer(0x00 | 0x80); // Read bit (0x80) + Register 0x00
        chip_id = SPI_Master_Transfer(0x00);
        CS_HIGH();
        USART_Transmit(chip_id);
        _delay_ms(100);
    }
}
