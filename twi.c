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


void TWI_Write_Read_Transaction_Synchronous(uint8_t slave_addr, uint8_t slave_reg, uint8_t *data, uint8_t len) {
    // Guard against 0 length requests
    if (len == 0) {
        USART_Transmit_String("write read transaction cannot service 0 length requests");
        return;
    }

    // Clear TWINT that enables the operation to start
    // And write the TWSTA to send a start condition
    // while keeping the TWEN bit set
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

    // writing the slave addres on the data register along with a 0(W) 
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
    TWDR = slave_reg;

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

    // start again for reading
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    // wait for start to be sent
    while (!(TWCR & (1 << TWINT))) ;

    // checking status reg, it must be set to STATUS_START
    // if not i need to handle the error prob by reseting i2c state (TODO later)
    if (TWI_STATUS_REG != STATUS_REPEAT_START_SENT) {
        // send STOP condition to release the bus
        TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
        USART_Transmit_String("Unexpected TWI STATUS REG (expected STATUS_START_SENT)");
        return;
    }

    // writing the slave addres on the data register along with a 1(R) 
    TWDR = (slave_addr << 1) | 1;

    // starting transmition by clearing TWINT (and keeping TWEN set)
    TWCR = (1 << TWINT) | (1 << TWEN);

    // waiting for the hardware to finish
    while (!(TWCR & (1 << TWINT))) ;

    // verify state
    if (TWI_STATUS_REG != STATUS_SLA_R_SENT_ACKED) {
        // send STOP condition to release the bus
        TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
        USART_Transmit_String("Unexpected TWI STATUS REG (expected STATUS_SLA_W_ACKED)");
        return;
    }

    // We will repeat start and ack for n-1 bytes we want to receive
    for (uint8_t i = 0; i < len-1; i++) {
        // start receiving with ack bit enabled
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
        // waiting for the receive to finish
        while (!(TWCR & (1 << TWINT))) ;

        // verify state
        if (TWI_STATUS_REG != STATUS_DATA_RECEIVED_AKED) {
            // send STOP condition to release the bus
            TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
            USART_Transmit_String("Unexpected TWI STATUS REG (expected STATUS_DATA_SENT_ACKED)");
            return;
        }

        // getting data from the data reg
        data[i] = TWDR;
    }
    // start receiving with ack bit disabled
    TWCR = (1 << TWINT) | (1 << TWEN);

    // waiting for the receive to finish
    while (!(TWCR & (1 << TWINT))) ;

    // verify state
    if (TWI_STATUS_REG != STATUS_DATA_RECEIVED_NAKED) {
        // send STOP condition to release the bus
        TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
        USART_Transmit_String("Unexpected TWI STATUS REG (expected STATUS_DATA_SENT_ACKED)");
        return;
    }

    // getting data from the data reg
    data[len - 1] = TWDR;

    // Send STOP condition to release the bus
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO)); // Wait for STOP condition to clear
}

void TWI_Write_Write_Transaction_Synchronous(uint8_t slave_addr, uint8_t slave_reg, const uint8_t *data, uint8_t len) {
    // Guard against 0 length requests
    if (len == 0) {
        USART_Transmit_String("write read transaction cannot service 0 length requests");
        return;
    }

    // Clear TWINT that enables the operation to start
    // And write the TWSTA to send a start condition
    // while keeping the TWEN bit set
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

    // writing the slave addres on the data register along with a 0(W) 
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
    TWDR = slave_reg;

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

    // start again for writing
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    // wait for start to be sent
    while (!(TWCR & (1 << TWINT))) ;

    // checking status reg, it must be set to STATUS_START
    // if not i need to handle the error prob by reseting i2c state (TODO later)
    if (TWI_STATUS_REG != STATUS_REPEAT_START_SENT) {
        // send STOP condition to release the bus
        TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
        USART_Transmit_String("Unexpected TWI STATUS REG (expected STATUS_START_SENT)");
        return;
    }

    // writing the slave addres on the data register along with a 0(W) 
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

    // We will repeat start and ack for n-1 bytes we want to receive
    for (uint8_t i = 0; i < len-1; i++) {
        // setting data in the data reg
        TWDR = data[i];

        // start transmiting
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

    }

    // setting data in the data reg
    TWDR = data[len - 1];

    // start transmiting
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
    while (TWCR & (1 << TWSTO)); // Wait for STOP condition to clear
}