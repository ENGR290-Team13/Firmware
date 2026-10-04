/* Sharp GP2Y0A21YK0F IR assignment: ATmega328P / Nano, 16 MHz.
 * Sensor output: A6 (ADC6). External reference: 3.09 V on AREF.
 * Sensor supply: 5 V with common ground (3.09 V is only the ADC reference).
 * PWM LED: D3 (PD3); threshold LED: D13 (PB5).
 * Serial Monitor: 9600 baud, 8-N-1.
 * All Files with original source code is defined all code was Provided in complementary files in ENGR 290's moodle page
 */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stdlib.h>
#include <math.h>
#include "init_290.h"
#include <string.h>
#include <util/delay.h>

#define BAUD 9600UL
#define UBRR ((F_CPU) / ((BAUD) * (16UL)) - 1)
#define ADC_sample_max 4

static volatile uint8_t RX_buff, servo_idx, ADC_sample, V_batt;
static volatile uint16_t time, delay_ms, ADC_acc;

static volatile struct
{
    uint8_t ADC0;
    uint8_t ADC1;
    uint8_t ADC2;
    uint8_t ADC3;
    uint8_t ADC6;
    uint8_t ADC7;
} ADC_data;

static const char CRLF[3] = {13, 10};
static volatile uint8_t *msg, TX_buffer1[20], TX_buffer2[20];

volatile struct
{
    uint8_t TX_finished : 1;

} flags;

ISR(USART_TX_vect)
{
    msg++;
    if (*msg)
        UDR0 = *msg;
    else
        flags.TX_finished = 1;
}

void uart_tx_init()
{
    UBRR0H = (uint8_t)((UBRR) >> 8); // Set the UART speed as defined by UBRR
    UBRR0L = (uint8_t)UBRR;
    UCSR0B |= (1 << TXCIE0) | (1 << TXEN0); //(1<<UDRIE0) Enable TX and TX IRQ.
    UCSR0C = (3 << UCSZ00);                 // Asynchronous UART, 8-N-1
} // end UART init

void send_reading(int16_t value, char label[], uint8_t crlf)
{
    while (!flags.TX_finished)
        ; // waiting for other transmission to complete
    flags.TX_finished = 0;
    strcpy(TX_buffer1, label);
    itoa(value, &TX_buffer2[0], 10);
    strcat(TX_buffer1, TX_buffer2);
    if (crlf)
        strcat(TX_buffer1, CRLF); // add CR LF if necesary
    msg = TX_buffer1;
    UDR0 = *msg;
}

void adc_init(uint8_t channel, uint8_t en_IRQ)
{
    // ADC init
    ADMUX = ((1 << ADLAR) | (channel & 0x0F) | (1 << REFS0)); // "left-aligned" result for easy 8-bit reading.
                                                              // AVcc as Aref |(1<<REFS0)
                                                              // Sets ADC to the specified channel. Can be changed later.
    ADCSRA = (1 << ADEN);
    if (en_IRQ)
        ADCSRA |= (1 << ADIE);                            // enable ADC Complete Interrupt. NOTE: the ISR MUST be defined!!!
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // ADC clock prescaler
    ADCSRA |= (1 << ADATE);                               // Continuosly running mode
    ADCSRA |= (1 << ADSC);                                // Start ADC
}

ISR(ADC_vect)
{ // the ADC runs on interrupt and populates ADC_data structure.
    // You can use the ADC readings in the structure.
    //  PORTB^=(1<<PB3); // to check ISR timing
    if (ADC_sample == 0)
    {                // scraping the first reading after the channel was changed
        ADC_acc = 0; // reset the accumulator
        ADC_sample++;
        return;
    }
    if (ADC_sample <= ADC_sample_max)
    { // averaging filter: accumulating readings
        ADC_acc += ADCH;
        ADC_sample++;
        return;
    }
    if (ADC_sample > ADC_sample_max)
    {
        ADC_sample = 0;
        ADC_acc = (ADC_acc / ADC_sample_max); // averaging filter: dividing accumulated readings by the number of readings
        switch (ADMUX & 7)
        { // checking, which ADC channel was read. NOTE: the ADC multiplexer register is used - no need for a variable to keep track.
        case 0:
        {                                  // channel 0
            ADMUX = (ADMUX & 0xF0) | 0x01; // switching to the next channel
            // If your next sensor uses different Aref, you can change it here.
            //		ADMUX=...?
            ADC_data.ADC0 = (uint8_t)(ADC_acc); // storing the filtered reading in ADC_data.
            return;
        }
        case 1:
        {
            ADMUX = (ADMUX & 0xF0) | 0x02;
            ADC_data.ADC1 = (uint8_t)(ADC_acc);
            return;
        }
        case 2:
        {
            ADMUX = (ADMUX & 0xF0) | 0x03;
            ADC_data.ADC2 = (uint8_t)(ADC_acc);
            return;
        }
        case 3:
        {
            ADMUX = (ADMUX & 0xF0) | 0x06;
            ADC_data.ADC3 = (uint8_t)(ADC_acc);
            return;
        }

        case 6:
        {
            ADMUX = (ADMUX & 0xF0) | 0x07;
            ADC_data.ADC6 = (uint8_t)(ADC_acc);
            return;
        }

        case 7:
        {
            ADMUX = (ADMUX & 0xF0) | 0x00;
            ADC_data.ADC7 = (uint8_t)(ADC_acc);
            return;
        }
        default:
        { // if something goes wrong - switching to Channel 0
            ADMUX = (ADMUX & 0xF0) | 0x00;
        }
        }
    }
}

void pin_init()
{

    DDRB |= (1 << PB3); // D3's light set as an ouput
    DDRB |= (1 << PB5); // L pulse light

    PORTB &= ~(1 << PB5); // set L initally OFF
}

void timer2_init()
{ // using Arduino Docs to Fast Mode PWM for the D3 light dimming

    TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS22);

    OCR2A = 0; // initally set this to 0
}

void flashing_L()
{
    PORTB |= (1 << PB5); // LED ON
    _delay_ms(750);

    PORTB &= ~(1 << PB5); // LED OFF
    _delay_ms(750);
}

int main(void)
{
    uart_tx_init();

    adc_init(0, 1);

    flags.TX_finished = 1;

    sei();

    while (1)
    {
        send_reading(ADC_data.ADC6, "ADC: ", 1);

        float Vout = ADC_data.ADC6 * (3.09) / 256; // for 8 bit we use 256

        float distance = 29.988 * pow(Vout, -1.173); // Found Equation from following ReadME file https://github.com/guillaume-rico/SharpIR

        send_reading(distance, "Distance: ", 1);

        if (distance <= 16)
        {
            OCR2A = 255;
            flashing_L();
        }
        else if (distance >= 49)
        {
            OCR2A = 0;
            flashing_L();
        }
        else
        {
            OCR2A = (uint8_t)(255.0f * (49.0f - distance) / 33.0f); // the 33 represents D2 - d1 equations used using a graphing calculator for the logic
        }
    }
}