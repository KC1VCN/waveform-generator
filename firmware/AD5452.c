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
#include <util/delay.h>
#include <avr/interrupt.h>

#include "AD5452.h"

//
void AD5452_init(void)
{
    unsigned int C1 = 0, C0 = 0;
    unsigned int reg = 0;
 
//  
    reg = (C1 << 15) | (C0 << 14) | 0x000;
 
//  
    PORTB = PORTB & ~(1 << DDB1);
    SPI_MasterTransmit(((reg & 0xFF00) >> 8));
    SPI_MasterTransmit(reg & 0xFF);
    PORTB = PORTB | (1 << DDB1);
}

//
void AD5452_set_output(unsigned int data)
{
    unsigned int C1 = 0, C0 = 0;
    unsigned int reg = 0;

//    
    reg = (C1 << 15) | (C0 << 14) | ((data & 0xFFF) << 2);

//
    PORTB = PORTB & ~(1 << DDB1);
    SPI_MasterTransmit(((reg & 0xFF00) >> 8));
    SPI_MasterTransmit(reg & 0xFF);
    PORTB = PORTB | (1 << DDB1);
}
