#ifndef MPU6050_H
#define MPU6050_H
#include <stdint.h>

typedef struct {
    int16_t ax, ay, az;  
    int16_t gx, gy, gz;  
} mpu6050_sample_t;

uint8_t mpu6050_init(void);
uint8_t mpu6050_read(mpu6050_sample_t *sample);

uint8_t mpu6050_set_ranges(uint8_t gyro_range, uint8_t accel_range);

#endif