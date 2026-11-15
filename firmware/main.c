// Copyright (C) 2026 Noyan Kinayman, KC1VCN
//
// This file is part of the KC1VCN Waveform Generator project.
//
// This source describes Open Hardware and is licensed under the
// CERN Open Hardware Licence Version 2 - Weakly Reciprocal
// (CERN-OHL-W-2.0).
//
// You may redistribute and modify this source and make products
// using it under the terms of CERN-OHL-W-2.0.
//
// This source is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY,
// INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS
// FOR A PARTICULAR PURPOSE. Please see the CERN-OHL-W-2.0 licence
// for applicable conditions.
//
// SPDX-License-Identifier: CERN-OHL-W-2.0
//
#ifndef F_CPU
#define F_CPU 16000000L
#endif

#define CLOCK_TRIM 8.5

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "UART.h"
#include "AD5452.h"
#include "AD9833.h"
#include "AD5270.h"

ISR (INT0_vect)
{
    static long int freq = 1E3;
    static long int freq_step = 100;
    static long int freq_max = 20E3;
    static long int freq_min = 20;
 
    if ((((PIND >> PD4) & 1) ^ ((PIND >> PD2) & 1)) == 0)
    {
        freq = freq-freq_step;
        if (freq < freq_min) freq = freq_min;
    }
    else
    {
        freq = freq+freq_step;
        if (freq > freq_max) freq = freq_max;
    }

//
    AD9833_set_freq_hz(freq, 0, CLOCK_TRIM);
}

ISR (INT1_vect)
{
    static long int gain = 500;
    static long int gain_step = 100;
    static long int gain_max = 4095;
    static long int gain_min = 0;
 
    if ((((PIND >> PD5) & 1) ^ ((PIND >> PD3) & 1)) == 0)
    {
        gain = gain-gain_step;
        if (gain < gain_min) gain = gain_min;
    }
    else
    {
        gain = gain+gain_step;
        if (gain > gain_max) gain = gain_max;
    }

//
    AD5452_set_output(gain);
}

void SPI_MasterInit(void)
{
//
// DDB0: 1, ~SYNC for AD5270
// DDB1: 1, ~SYNC for AD5452
// DDB2: 1, ~SYNC for AD9833
// DDB3: 1, SPI data
// DDB5: 1, SPI clock
//
    DDRB = (1 << DDB3) | (1 << DDB5) | (1 << DDB2) | (1 << DDB1) | (1 << DDB0);
    PORTB = PORTB | (1 << DDB2) | (1 << DDB1) | (1 << DDB0);

//
// SPE:  1, SPI enable
// MSTR: 1, host/master mode
// CPOL: 1, clock idle high
// CPHA: 0, sample on the leading clock edge
// SPR:  1, F_CPU/16 = 1 MHz SPI clock
//
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << CPOL) | (0 << CPHA);
}

void SPI_MasterTransmit(char cData)
{
    SPDR = cData;

//
// SPIF: 1, serial data transfer finishes
//
    while (!(SPSR & (1 << SPIF)));
}

void ADC_init(void)
{
//
// REFS: 1, AVCC reference voltage
// MUX:  0, ADC0 input
//
    ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);

//
// ADEN: 1, ADC enable
// ADPS: 3, F_CPU/128 = 125 kHz ADC clock
//
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(unsigned int channel)
{
    unsigned long int adc = 0;
    unsigned int count = 0;
    unsigned int adc_max = 0;

//
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    while (count<30)
    {
//
// ADSC: 1, start ADC conversion
//
        ADCSRA |= (1 << ADSC);

        while (ADCSRA & (1 << ADSC));

        adc = adc+ADC;

        count = count+1;
    }

//
    return adc_max/count;
}

void send_data_FSK(unsigned long int count, char *payload)
{
    unsigned int i = 0;
    unsigned int n = strlen(payload);
    char reg = 0;
    char preamble[4] = {0b10101010, 0b10101010, 0b00000111, 0b00010010};
    char message[64];

    memcpy(message, preamble, 4);
    memcpy(message+4, payload, n);

    AD9833_set_phase_degrees(0, 0);
    AD9833_set_phase_degrees(0, 1);

    AD9833_set_freq_hz( 999000, 1, CLOCK_TRIM);
    AD9833_set_freq_hz(1001000, 0, CLOCK_TRIM);

//
    while (count--)
    {
        i = 0;

        while (i<8*(n+4))
        {
            reg = (message[i >> 3] >> (7-(i & 7))) & 1;

            AD9833_select_output(reg, reg);

            i = i+1;

// 1  ksps: 972.35 us delay
// 3  ksps: 306.30 us delay
// 10 ksps:  73.24 us delay

            _delay_us(306.30);
        }
    }

//
    AD9833_set_freq_hz(0, 1, CLOCK_TRIM);
    AD9833_set_freq_hz(0, 0, CLOCK_TRIM);
}

void send_data_PSK(unsigned long int count, char *payload)
{
    unsigned int i = 0;
    unsigned int n = strlen(payload);
    char reg = 0;
    char preamble[4] = {0b10101010, 0b10101010, 0b00000111, 0b00010010};
    char message[64];

    memcpy(message, preamble, 4);
    memcpy(message+4, payload, n);

    AD9833_set_phase_degrees(180, 0);
    AD9833_set_phase_degrees(  0, 1);

    AD9833_set_freq_hz(1000000, 1, CLOCK_TRIM);
    AD9833_set_freq_hz(1000000, 0, CLOCK_TRIM);

//
    while (count--)
    {
        i = 0;

        while (i<8*(n+4))
        {
            reg = (message[i >> 3] >> (7-(i & 7))) & 1;

            AD9833_select_output(reg, reg);

            i = i+1;

// 1  ksps: 972.35 us delay
// 3  ksps: 306.30 us delay
// 10 ksps:  73.24 us delay

            _delay_us(306.30);
        }
   }

//
    AD9833_set_freq_hz(0, 1, CLOCK_TRIM);
    AD9833_set_freq_hz(0, 0, CLOCK_TRIM);
}

int main(void) 
{
    char data[128];
    char *command, *value;
    unsigned long int freq[2] = {0, 0};
    unsigned long int phase[2] = {0, 0};
    unsigned long int freq_hz = 0;
    unsigned long int phase_degrees = 0;
    unsigned long int gain = 0;
    unsigned long int dc_offset = 0;
    unsigned long int count = 0;
    unsigned int freq_select = 0;
    unsigned int phase_select = 0;

//
    UART_Init();
    ADC_init();
    SPI_MasterInit();

//    
    AD9833_init();
    AD9833_select_output(0, 0);
    AD9833_set_waveform(1);

    AD9833_set_freq((long) 0, 0);
    AD9833_set_freq((long) 0, 1);

    AD9833_set_phase(0, 0);
    AD9833_set_phase(0, 1);

//
    AD5452_init();
    AD5452_set_output(0);

//    
    AD5270_init();
    AD5270_set_output(0);

//
    DDRD  &= ~((1 << PD5) | (1 << PD4) | (1 << PD3) | (1 << PD2));
    PORTD |= (1 << PD5) | (1 << PD4) | (1 << PD3) | (1 << PD2);

//
// ISC00: 1, any change on INT0 generates an interrupt
// ISC10: 1, any change on INT1 generates an interrupt
//
    EICRA = (1 << ISC10) | (1 << ISC00);
//
// INT0: 1, INT0 enable
// INT1: 1, INT1 enable
//
    EIMSK = (1 << INT1) | (1 << INT0);
    
    sei();
    
//
    AD9833_set_freq_hz(1000, 0, CLOCK_TRIM);
    AD5270_set_output(335);

//
    AD5452_set_output(2048);
    _delay_ms(500);
    AD5452_set_output(0);
    _delay_ms(500);
    AD5452_set_output(2048);

//
    while (1)
    {
        UART_Transmit_Array((unsigned char *) "READY\n");
        
        UART_Receive_Array((unsigned char *) data);
        
        command = strtok(data, ":");

//
        if (strcmp(command, "output") == 0)
        {
            value = strtok(NULL, ":");
            freq_select = atoi(value);
            value = strtok(NULL, ":");
            phase_select = atoi(value);

            AD9833_select_output(freq_select, phase_select);
        }
        else if (strcmp(command, "freq") == 0)
        {
            value = strtok(NULL, ":");
            freq[freq_select] = atol(value);

            AD9833_set_freq(freq[freq_select], freq_select);
        }
        else if (strcmp(command, "freq-hz") == 0)
        {
            value = strtok(NULL, ":");
            freq_hz = atol(value);

            freq[freq_select] = AD9833_set_freq_hz(freq_hz, freq_select, CLOCK_TRIM);
        }
        else if (strcmp(command, "freq?") == 0)
        {
            sprintf(data, "%ld\n", freq[freq_select]);

            UART_Transmit_Array((unsigned char *) data);
        }
        else if (strcmp(command, "phase") == 0)
        {
            value = strtok(NULL, ":");
            phase[phase_select] = atol(value);

            AD9833_set_phase(phase[phase_select], phase_select);
        }
        else if (strcmp(command, "phase-deg") == 0)
        {
            value = strtok(NULL, ":");
            phase_degrees = atol(value);

            phase[phase_select] = AD9833_set_phase_degrees(phase_degrees, phase_select);
        }
        else if (strcmp(command, "phase?") == 0)
        {
            sprintf(data, "%ld\n", phase[phase_select]);

            UART_Transmit_Array((unsigned char *) data);
        }
        else if (strcmp(command, "gain") == 0)
        {
            value = strtok(NULL, ":");
            gain = atol(value);

            AD5452_set_output(gain);
        }
        else if (strcmp(command, "gain?") == 0)
        {
            sprintf(data, "%ld\n", gain);

            UART_Transmit_Array((unsigned char *) data);
        }
        else if (strcmp(command, "offset") == 0)
        {
            value = strtok(NULL, ":");
            dc_offset = atol(value);

            AD5270_set_output(dc_offset);
        }
        else if (strcmp(command, "offset?") == 0)
        {
            sprintf(data, "%ld\n", dc_offset);

            UART_Transmit_Array((unsigned char *) data);
        }
        else if (strcmp(command, "wave") == 0)
        {
            value = strtok(NULL, ":");

            if (strcmp(value, "sin") == 0) AD9833_set_waveform(1);
            else if (strcmp(value, "tri") == 0)  AD9833_set_waveform(2);
            else if (strcmp(value, "sqr1") == 0) AD9833_set_waveform(3);
            else if (strcmp(value, "sqr2") == 0) AD9833_set_waveform(4);
            else AD9833_set_waveform(0);
        }
        else if (strcmp(command, "send-fsk") == 0)
        {
            value = strtok(NULL, ":");
            count = atol(value);
            value = strtok(NULL, ":");

            send_data_FSK(count, value);
        }
        else if (strcmp(command, "send-psk") == 0)
        {
            value = strtok(NULL, ":");
            count = atol(value);
            value = strtok(NULL, ":");

            send_data_PSK(count, value);
        }
        else
        {
            UART_Transmit_Array((unsigned char *) "ERROR\n");

            continue;
        }

        UART_Transmit_Array((unsigned char *) "DONE\n");
    }

    return 0;
}


