#include "TWI_290.h"
#include <avr/io.h>
#include <util/twi.h>

#define TWI_CLOCK_HZ 100000UL
#define TWI_WAIT_LIMIT 60000UL

static uint8_t twi_wait(void)
{
    uint32_t limit = TWI_WAIT_LIMIT;
    while (!(TWCR & (1u << TWINT))) {
        if (--limit == 0) return 1;
    }
    return 0;
}

void TWI_init(void)
{
    TWSR = 0;
    TWBR = (uint8_t)((F_CPU / TWI_CLOCK_HZ - 16UL) / 2UL);
    TWCR = (1u << TWEN);
}

uint8_t TWI_start(uint8_t twi_addr, uint8_t read_write)
{
    TWCR = (1u << TWINT) | (1u << TWSTA) | (1u << TWEN);
    if (twi_wait()) return 1;
    uint8_t status = TWSR & 0xF8u;
    if (status != TW_START && status != TW_REP_START) return 2;

    TWDR = (uint8_t)((twi_addr << 1) | (read_write & 1u));
    TWCR = (1u << TWINT) | (1u << TWEN);
    if (twi_wait()) return 3;
    status = TWSR & 0xF8u;
    if (status != (read_write ? TW_MR_SLA_ACK : TW_MT_SLA_ACK)) return 4;
    return 0;
}

void TWI_stop(void)
{
    TWCR = (1u << TWINT) | (1u << TWSTO) | (1u << TWEN);
}

uint8_t TWI_write(uint8_t tx_data)
{
    TWDR = tx_data;
    TWCR = (1u << TWINT) | (1u << TWEN);
    if (twi_wait()) return 1;
    return (TWSR & 0xF8u) == TW_MT_DATA_ACK ? 0 : 2;
}

uint8_t TWI_ack_read(uint8_t *data)
{
    if (!data) return 1;
    TWCR = (1u << TWINT) | (1u << TWEN) | (1u << TWEA);
    if (twi_wait()) return 2;
    if ((TWSR & 0xF8u) != TW_MR_DATA_ACK) return 3;
    *data = TWDR;
    return 0;
}

uint8_t TWI_nack_read(uint8_t *data)
{
    if (!data) return 1;
    TWCR = (1u << TWINT) | (1u << TWEN);
    if (twi_wait()) return 2;
    if ((TWSR & 0xF8u) != TW_MR_DATA_NACK) return 3;
    *data = TWDR;
    return 0;
}

uint8_t Read_Reg_N(uint8_t twi_addr, uint8_t reg_addr,
                   uint8_t bytes, uint8_t *data)
{
    uint8_t result;
    if (!data || !bytes) return 1;
    result = TWI_start(twi_addr, TW_WRITE);
    if (result) goto done;
    result = TWI_write(reg_addr);
    if (result) goto done;
    result = TWI_start(twi_addr, TW_READ);
    if (result) goto done;
    for (uint8_t i = 0; i < bytes; ++i) {
        result = (i + 1u == bytes) ? TWI_nack_read(&data[i])
                                   : TWI_ack_read(&data[i]);
        if (result) goto done;
    }
done:
    TWI_stop();
    return result;
}

uint8_t Read_Reg(uint8_t twi_addr, uint8_t reg_addr, uint8_t *data)
{
    return Read_Reg_N(twi_addr, reg_addr, 1u, data);
}

uint8_t Write_Reg(uint8_t twi_addr, uint8_t reg_addr, uint8_t value)
{
    uint8_t result = TWI_start(twi_addr, TW_WRITE);
    if (result) goto done;
    result = TWI_write(reg_addr);
    if (result) goto done;
    result = TWI_write(value);
done:
    TWI_stop();
    return result;
}