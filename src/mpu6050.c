#include "mpu6050.h"
#include "TWI_290.h"

#define MPU_ADDRESS       0x68u 
#define REG_SMPLRT_DIV    0x19u
#define REG_CONFIG       0x1Au
#define REG_GYRO_CONFIG  0x1Bu
#define REG_ACCEL_CONFIG 0x1Cu
#define REG_ACCEL_XOUT_H 0x3Bu
#define REG_PWR_MGMT_1   0x6Bu
#define REG_WHO_AM_I     0x75u

uint8_t mpu6050_set_ranges(uint8_t gyro_range, uint8_t accel_range)
{
    if (gyro_range > 3u || accel_range > 3u) return 0;
    if (Write_Reg(MPU_ADDRESS, REG_GYRO_CONFIG, gyro_range << 3)) return 0;
    if (Write_Reg(MPU_ADDRESS, REG_ACCEL_CONFIG, accel_range << 3)) return 0;
    return 1;
}

uint8_t mpu6050_init(void)
{
    uint8_t who = 0;
    TWI_init();
    if (Read_Reg(MPU_ADDRESS, REG_WHO_AM_I, &who) || who != 0x68u) return 0;
    if (Write_Reg(MPU_ADDRESS, REG_PWR_MGMT_1, 0x01u)) return 0;
    if (Write_Reg(MPU_ADDRESS, REG_CONFIG, 0x03u)) return 0;
    if (Write_Reg(MPU_ADDRESS, REG_SMPLRT_DIV, 0x09u)) return 0; 
    return mpu6050_set_ranges(0, 0);
}

uint8_t mpu6050_read(mpu6050_sample_t *s)
{
    uint8_t b[14];
    if (!s || Read_Reg_N(MPU_ADDRESS, REG_ACCEL_XOUT_H, 14u, b)) return 0;

    s->ax = (int16_t)(((uint16_t)b[0] << 8) | b[1]);
    s->ay = (int16_t)(((uint16_t)b[2] << 8) | b[3]);
    s->az = (int16_t)(((uint16_t)b[4] << 8) | b[5]);
    s->gx = (int16_t)(((uint16_t)b[8] << 8) | b[9]);
    s->gy = (int16_t)(((uint16_t)b[10] << 8) | b[11]);
    s->gz = (int16_t)(((uint16_t)b[12] << 8) | b[13]);
    return 1;
}