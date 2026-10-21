#ifndef TWI_290_H
#define TWI_290_H

#include <stdint.h>

void TWI_init(void);

uint8_t TWI_start(uint8_t twi_addr, uint8_t read_write);
void TWI_stop(void);
uint8_t TWI_write(uint8_t tx_data);

uint8_t TWI_ack_read(uint8_t *data);
uint8_t TWI_nack_read(uint8_t *data);

uint8_t Read_Reg(uint8_t twi_addr, uint8_t reg_addr, uint8_t *data);
uint8_t Read_Reg_N(uint8_t twi_addr, uint8_t reg_addr,
                   uint8_t bytes, uint8_t *data);
uint8_t Write_Reg(uint8_t twi_addr, uint8_t reg_addr, uint8_t value);
#endif
