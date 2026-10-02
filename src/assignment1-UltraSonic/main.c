#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include "init_290.h"

#define BAUD 9600UL
#define UBRR ((F_CPU) / ((BAUD) * (16UL)) - 1)

volatile struct
{
    uint8_t TX_finished : 1;
    uint8_t sample : 1;
    uint8_t mode : 1;
    uint8_t stop : 1;
    uint8_t T1_ovf0 : 2;
    uint8_t T1_ovf1 : 2;
} flags;

static volatile struct
{
    uint16_t pulse0;
    uint16_t pulse1;
    uint16_t t_start0;
    uint16_t t_end0;
    uint16_t t_start1;
    uint16_t t_end1;
    uint16_t finished;
} PULSE_data;

static volatile uint8_t RX_buff, servo_idx, ADC_sample, V_batt;
static volatile uint16_t time, delay_ms, ADC_acc;

ISR(TIMER1_CAPT_vect)
{ // system tick: 50Hz, 20ms
    time++;
    delay_ms += 20;
    flags.sample = 1; // starts IMU sampling
    flags.T1_ovf0++;
    flags.T1_ovf1++;
}

static const char CRLF[3] = {13, 10};
static volatile uint8_t *msg, TX_buffer1[20], TX_buffer2[20];

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

ISR(INT0_vect)
    { // timing capture for US sensor
        if (PIND & (1 << PD2))
        { // raising edge IRQ - start of pulse
            flags.T1_ovf0 = 0;
            PULSE_data.t_start0 = TCNT1; // There is no need to disable global interrupts as they are already disabled while and ISR is being executed.
        }
        if (!(PIND & (1 << PD2)))
        { // falling edge IRQ - end of pulse
            PULSE_data.t_end0 = TCNT1;
            if (flags.T1_ovf0 == 1)
                PULSE_data.pulse0 = PWM_TOP - PULSE_data.t_start0 + PULSE_data.t_end0; // one ovf
            else if (flags.T1_ovf0 == 2)
                PULSE_data.pulse0 = PWM_TOP - PULSE_data.t_start0 + PULSE_data.t_end0 + PWM_TOP; // two ovfs
            else if (flags.T1_ovf0 == 0)
                PULSE_data.pulse0 = PULSE_data.t_end0 - PULSE_data.t_start0; // no ovf
            else
                PULSE_data.pulse0 = 0xFFFF; // something went wrong
        }
        PULSE_data.finished = 1;
}

void pin_init(){

    DDRD &= ~(1 << PD2); // Echo
    DDRB |= (1 << PB3); // Trig
    DDRD |= (1 << PD3); // D3's light
    DDRB |= (1 << PB5); //L pulse light

    PORTB &= ~(1 << PB3); // set trig initial Low
    PORTB &= ~(1 << PB5); // set L initally OFF
}

void int0_init(){
    EICRA |= (1<<ISC00);                              
    EIMSK |= (1 << INT0); 
                                             
}

void timer1_init()
{                                            
    TCCR1B |= ((1 << CS11) | (1 << CS10)); // timer prescaler set to 64
    
}

void timer2_init(){ // using Arduino Docs to Fast Mode PWM for the D3 light dimming

    TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS22);

    OCR2B = 0; // initally set this to 0
}

void flashing_L()
{
    PORTB |= (1 << PB5); // LED ON
    _delay_ms(750);

    PORTB &= ~(1 << PB5); // LED OFF
    _delay_ms(750);
}

int main(void){

    uart_tx_init();
    pin_init();
    int0_init();
    timer1_init();
    timer2_init();
    sei();

    
    flags.TX_finished = 1;

    while(1){

        PULSE_data.finished = 0;

        PORTB |= (1 << PB3); // set trigger high

        _delay_us(10);

        PORTB &= ~(1 << PB3);

        while(!(PULSE_data.finished)){
        }

        unsigned long int pulse_length = PULSE_data.pulse0 * 4;

        float distance = ((float)pulse_length / 1000000.0f) * 34300.0f / 2.0f;

        send_reading((int16_t)distance, "Distance", 1);

        if(distance <= 16){
            OCR2B = 255;
            flashing_L();
        } else if(distance >= 49){
            OCR2B = 0;
            flashing_L();
        } else {
            OCR2B = (uint8_t)(255.0f * (49.0f - distance) / 33.0f);
        }
        
    }
}