#include <avr/io.h>
#include "twi.h"
#include "usart.h"

void TWI_Init(uint8_t internal_pullups) {
    // enable internal pull-up resistors on SDA (PC4) and SCL (PC5)
    // needed if external pull-up resistors dont exist
    if (internal_pullups) PORTC |= (1 << PORTC4) | (1 << PORTC5);

    // set Prescaler bits to desired value (TWPS1:0 = 00 -> Prescaler = 1)
    // may need to reconsider if F_SCL changes
    
    _Static_assert(F_SCL == 100000, "if F_SCL changes reconsider the prescaler bits");
    TWSR &= ~((1 << TWPS0) | (1 << TWPS1));

    // set Bit Rate Register value
    TWBR = ((F_CPU / F_SCL) - 16) / 2;

    // enable TWI Peripheral
    TWCR = (1 << TWEN);
}

void TWI_Master_Tx_Synchronous(uint8_t slave_addr, uint8_t data) {
    // Clear TWINT that enables the operation to start
    // And write the TWSTA to send a start condition
    // Keep the TWEN bit set
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    // wait for start to be sent
    while (!(TWCR & (1 << TWINT))) ;

    // checking status reg, it must be set to STATUS_START
    // if not i need to handle the error prob by reseting i2c state (TODO later)
    if (TWI_STATUS_REG != STATUS_START_SENT) {
        // send STOP condition to release the bus
        TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
        USART_Transmit_String("Unexpected TWI STATUS REG (expected STATUS_START_SENT)");
        return;
    }

    // writing what we want to send in the TW data reg, 0 -> W
    TWDR = (slave_addr << 1) | 0;

    // starting transmition by clearing TWINT (and keeping TWEN set)
    TWCR = (1 << TWINT) | (1 << TWEN);

    // waiting for the hardware to finish
    while (!(TWCR & (1 << TWINT))) ;

    // verify state
    if (TWI_STATUS_REG != STATUS_SLA_W_SENT_ACKED) {
        // send STOP condition to release the bus
        TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
        USART_Transmit_String("Unexpected TWI STATUS REG (expected STATUS_SLA_W_ACKED)");
        return;
    }

    // setting data in the data reg
    TWDR = data;
    // starting transmition by clearing TWINT (and keeping TWEN set)
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    // waiting for the hardware to finish
    while (!(TWCR & (1 << TWINT))) ;

    // verify state
    if (TWI_STATUS_REG != STATUS_DATA_SENT_ACKED) {
        // send STOP condition to release the bus
        TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
        USART_Transmit_String("Unexpected TWI STATUS REG (expected STATUS_DATA_SENT_ACKED)");
        return;
    }

    // Send STOP condition to release the bus
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}
