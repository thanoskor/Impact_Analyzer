#ifndef USART_H
#define USART_H

#include <stdint.h>

#define BAUD 9600
#define MYUBRR (((F_CPU + (BAUD * 8UL)) / (16UL * BAUD)) - 1)

void USART_Init(uint16_t ubrr);
void USART_Transmit( uint8_t data );
void USART_Transmit_String(const char *str);

#endif // USART_H