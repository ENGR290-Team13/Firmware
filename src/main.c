#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "mpu6050.h"

#define BAUD 9600UL
#define UBRR_VALUE (F_CPU / (16UL * BAUD) - 1UL)
#define SAMPLE_PERIOD_S 0.020f
#define RAD_TO_DEG 57.2957795f

const uint16_t Servo_angle[256] = {
    //  +/-90 degrees
    85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,                 // 0...15
    108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, // 16...31 0.9ms pulse(108)
    119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119, // 32...47
    131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, // 48...63
    142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, // 64...79
    154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, 154, // 80...95
    165, 165, 165, 165, 165, 165, 165, 165, 165, 165, 165, 165, 165, 165, 165, 165, // 96...111
    177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 177, 188, 188, 188, 188, // center - 1.5ms pulse (188) 112...127
    188, 188, 188, 188, 198, 198, 198, 198, 198, 198, 198, 198, 198, 198, 198, 198, // 128...143
    208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, // 143...159
    218, 218, 218, 218, 218, 218, 218, 218, 218, 218, 218, 218, 218, 218, 218, 218, // 160...175
    228, 228, 228, 228, 228, 228, 228, 228, 228, 228, 228, 228, 228, 228, 228, 228, // 175...191
    238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, // 192...207
    248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, // 208...223
    270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, // 224...239
    290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290  // 240...255
};

static void uart_init(void)
{
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;
    UCSR0B = (1u << TXEN0);
    UCSR0C = (1u << UCSZ01) | (1u << UCSZ00);
}
static void uart_char(char c)
{
    while (!(UCSR0A & (1u << UDRE0)))
    {
    }
    UDR0 = (uint8_t)c;
}
static void uart_text(const char *text)
{
    while (*text)
        uart_char(*text++);
}
static void uart_float(const char *label, float value)
{
    char buf[18];
    uart_text(label);
    dtostrf(value, 7, 2, buf);
    uart_text(buf);
}
static void timer1_init(void)
{
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1A |= (1 << COM1A1);
    TCCR1B |= (1 << WGM13);
    TCCR1B |= (1 << CS11) | (1 << CS10); // Prescaler = 64

    ICR1 = 2500; // 50 Hz PWM

    OCR1A = Servo_angle[127]; // initialize servo angle at middle/ 0 degrees
}
static void timer2_init(void)
{
    // Timer2 Fast PWM on D3
    TCCR2A = _BV(COM2A1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS22);

    OCR2A = 0;
}

static void pins_used_init(void)
{
    DDRB |= (1 << PB1); // Signal Pin used for Servo Motor
    DDRB |= (1 << PB5); // L light
    DDRB |= (1 << PB3); // D3's light

    PORTB &= ~(1 << PB5);
    PORTB |= (1 << PB3);
}

int main(void)
{

    uart_init();
    timer1_init();
    timer2_init();
    pins_used_init();
    _delay_ms(150);
    uart_text("\r\nENGR290 MPU6050 test\r\n");
    if (!mpu6050_init())
    {
        uart_text("ERROR: MPU6050 not detected. Check SDA A4, SCL A5, power and GND.\r\n");
        while (1)
        {
            _delay_ms(500);
        }
    }
    uart_text("MPU6050 detected. Keep stationary during gyro calibration...\r\n");

    int32_t sx = 0, sy = 0, sz = 0;
    mpu6050_sample_t s;
    for (uint16_t i = 0; i < 200; ++i)
    {
        if (!mpu6050_read(&s))
        {
            uart_text("ERROR: I2C read failed during calibration.\r\n");
            while (1)
            {
                _delay_ms(500);
            }
        }
        sx += s.gx;
        sy += s.gy;
        sz += s.gz;
        _delay_ms(10);
    }
    float bias_x = (float)sx / 200.0f;
    float bias_y = (float)sy / 200.0f;
    float bias_z = (float)sz / 200.0f;
    float roll = 0.0f, pitch = 0.0f, yaw = 0.0f;
    uint16_t Servo_Response = 0.0f;
    uint8_t first = 1;
    uint8_t count = 0;
    uint8_t count_IMU = 0;
    float position = 0.0f;
    float velocity = 0.0f;
    uart_text("Calibration done. Output every ~1.5 seconds.\r\n");

    while (1)
    {
        if (!mpu6050_read(&s))
        {
            uart_text("ERROR: MPU6050 read failed.\r\n");
            _delay_ms(100);
            continue;
        }
        float ax = (float)s.ax / 8192.0f;
        float ay = (float)s.ay / 8192.0f;
        float az = (float)s.az / 8192.0f;
        float gx = ((float)s.gx - bias_x) / 131.0f;
        float gy = ((float)s.gy - bias_y) / 131.0f;
        float gz = ((float)s.gz - bias_z) / 131.0f;
        float acc_roll = atan2f(ay, az) * RAD_TO_DEG;
        float acc_pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;
        if (first)
        {
            roll = acc_roll;
            pitch = acc_pitch;
            first = 0;
        }
        else
        {
            roll = 0.98f * (roll + gx * SAMPLE_PERIOD_S) + 0.02f * acc_roll;
            pitch = 0.98f * (pitch + gy * SAMPLE_PERIOD_S) + 0.02f * acc_pitch;
        }
        yaw += gz * SAMPLE_PERIOD_S;

        if (++count >= 75)
        {
            count = 0;
            uart_float("Roll: ", roll);
            uart_float(" Pitch: ", pitch);
            uart_float(" Yaw: ", yaw);
            uart_text(" deg\r\n");
            uart_float("Ax: ", ax);
            uart_float(" Ay: ", ay);
            uart_float(" Az: ", az);
            uart_text(" g\r\n\r\n");
        }

        if (++count_IMU >= 50)
        {
            count_IMU = 0;

            float dt = SAMPLE_PERIOD_S;
            float acceleration = ax * 9.81f; // Preserve direction

            if (fabsf(ax) > 0.13f)
            {
                position += velocity * dt + 0.5f * acceleration * dt * dt;

                velocity += acceleration * dt;
            }
            else
            {
                // Treat small acceleration as stationary
                velocity = 0.0f;
            }

            uart_float("X Position: ", position);
            uart_text(" m\r\n");
        }

        if (yaw <= (-85.0f) || yaw >= 85.0f)
        {
            PORTB |= (1 << PB5);
        }
        else
        {
            PORTB &= ~(1 << PB5);
        }

        if (yaw < 0)
        {
            if (yaw > -80.0f)
            {
                Servo_Response = (uint16_t)((127.0f / 90.0f) * yaw + 127.0f);
            }
            else
            {
                Servo_Response = (uint16_t)((127.0f / 90.0f) * (-80.0f) + 127.0f);
            }
        }
        else if (yaw > 0)
        {
            if (yaw < 80.0f)
            {
                Servo_Response = (uint16_t)((128.0f / 90.0f) * yaw + 127.0f);
            }
            else
            {
                Servo_Response = (uint16_t)((128.0f / 90.0f) * 80.0f + 127.0f);
            }
        }
        else
        {
            Servo_Response = 127;
        }

        if (ax > 1.11f || ax < -1.11f)
        {
            OCR2A = 0;
        }
        else if (ax < 0.11f || ax > -0.11f)
        {
            OCR2A = 255;
        }
        else
        {
            OCR2A = 255 - (uint8_t)(255.0f * (ax - 0.11f));
        }

        OCR1A = Servo_angle[Servo_Response];

        _delay_ms(20);
    }
}
