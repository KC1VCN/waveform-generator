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

#include "AD5270.h"

//
void AD5270_init(void)
{
    unsigned int C3 = 0, C2 = 1, C1 = 1, C0 = 1;
    unsigned int reg = 0;

//
    reg = (C3 << 13) | (C2 << 12) | (C1 << 11) | (C0 << 10) | 0x002;

//
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (0 << CPOL) | (1 << CPHA);
    
//
    PORTB = PORTB & ~(1 << DDB0);
    SPI_MasterTransmit(((reg & 0xFF00) >> 8));
    SPI_MasterTransmit(reg & 0xFF);
    PORTB = PORTB | (1 << DDB0);
    
//
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << CPOL) | (0 << CPHA);
}

//
void AD5270_set_output(unsigned int data)
{
    unsigned int C3 = 0, C2 = 0, C1 = 0, C0 = 1;
    unsigned int reg = 0;
 
//   
    reg = (C3 << 13) | (C2 << 12) | (C1 << 11) | (C0 << 10) | (data & 0x3FF);

//
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (0 << CPOL) | (1 << CPHA);
    
//
    PORTB = PORTB & ~(1 << DDB0);
    SPI_MasterTransmit(((reg & 0xFF00) >> 8));
    SPI_MasterTransmit(reg & 0xFF);
    PORTB = PORTB | (1 << DDB0);
    
//
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << CPOL) | (0 << CPHA);
}
