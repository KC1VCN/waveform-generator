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

#include <avr/io.h>
#include <string.h>

#include "UART.h" 

//
void UART_Init(void)
{
    UBRR0H = (unsigned char) (UBRR>>8);
    UBRR0L = (unsigned char) (UBRR);
    
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
    UCSR0C = (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UPM01) | (0<<UPM00);
}

//
unsigned char UART_Transmit(unsigned char *data)
{
    unsigned char error = 0;
    
    while (!(UCSR0A & (1<<UDRE0)));
    
    UDR0 = *data;

//
    return error;
}

//
unsigned char UART_Receive(unsigned char *data)
{
    unsigned char error = 0;
    
    while (!(UCSR0A & (1<<RXC0)));
    
    *data = UDR0;
    
    error = UCSR0A & (FRAME_ERROR | DATA_OVERRUN | PARITY_ERROR);
    if (error != 0) UCSR0A = UCSR0A ^ (FRAME_ERROR | DATA_OVERRUN | PARITY_ERROR);

//
    return error;
}

//
unsigned char UART_Transmit_Array(unsigned char *data)
{
    unsigned char error = 0;
    int i = 0;
    
    while (i<128)
    {
        error = UART_Transmit(data+i);
        
        if (error != 0 || *(data+i) == 10) break;
        
        i = i+1;
    }

//
    return error;
}

unsigned char UART_Receive_Array(unsigned char *data)
{
    unsigned char error = 0;
    int i = 0;
    
    while (i<128)
    {
        error = UART_Receive(data+i);
        
        if (error != 0 || *(data+i) == 10) break;
        
        i = i+1;
    }

//
    return error;
}

