#ifndef TWI_H
#define TWI_H

// i2c SCL frequency
#define F_SCL 100000 // 100khz

//masks the prescaler bits (the last two)
#define TWI_STATUS_REG (TWSR & 0xF8)

//diferent states the TWI_STATUS_REG can have (with the prescaler bits masked)
#define STATUS_START_SENT 0x08
#define STATUS_REPEAT_START_SENT 0x10
#define STATUS_SLA_W_SENT_ACKED 0x18
#define STATUS_DATA_SENT_ACKED 0x28

#define STATUS_SLA_R_SENT_ACKED 0x40
#define STATUS_DATA_RECEIVED_AKED 0x50
#define STATUS_DATA_RECEIVED_NAKED 0x58

void TWI_Init(uint8_t internal_pullups);
void TWI_Write_Read_Transaction_Synchronous(uint8_t slave_addr, uint8_t slave_reg, uint8_t *data, uint8_t len);

#endif // TWI_H
