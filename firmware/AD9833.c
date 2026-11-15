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

#include "AD9833.h"

//
void AD9833_init(void)
{
    unsigned long int reg = 0;
    
    char B28 = 1;
    char HLB = 0;
    char FSELECT = 0;
    char PSELECT = 0;
    char RESET = 1;
    char SLEEP1 = 0;
    char SLEEP12 = 0;
    char OBITEN = 0;
    char DIV2 = 0;
    char MODE = 0;

//
    reg = (B28 << 13) | (HLB << 12) | (FSELECT << 11) | (PSELECT << 10) | (RESET << 8);
    reg = reg | (SLEEP1 << 7) | (SLEEP12 << 6) | (OBITEN << 5) | (DIV2 << 3) | (MODE << 1);

//
    PORTB = PORTB & ~(1 << DDB2);
    SPI_MasterTransmit(((reg & 0xFF00) >> 8));
    SPI_MasterTransmit(reg & 0xFF);
    PORTB = PORTB | (1 << DDB2);

//
    REG = reg;
}

//
void AD9833_set_waveform(char waveform)
{
    unsigned long int reg = REG;

    char MODE = 0;
    char DIV2 = 0;
    char OPBITEN = 0;
    char RESET = 0;

//
    reg = reg & (~(1 << 8 | 1 << 5 | 1 << 3 | 1 << 1));

//
    if (waveform == 1)
    {
        MODE = 0;
        OPBITEN = 0;
        DIV2 = 0;
    }
    else if (waveform == 2)
    {
        MODE = 1;
        OPBITEN = 0;
        DIV2 = 0;
    }
    else if (waveform == 3)
    {
        MODE = 0;
        OPBITEN = 1;
        DIV2 = 0;
    }
    else if (waveform == 4)
    {
        MODE = 0;
        OPBITEN = 1;
        DIV2 = 1;
    }
    else
    {
        RESET = 1;
    }
    
//
    reg = reg | (RESET << 8) | (OPBITEN << 5) | (DIV2 << 3) | (MODE << 1);

//
    PORTB = PORTB & ~(1 << DDB2);
    SPI_MasterTransmit(((reg & 0xFF00) >> 8));
    SPI_MasterTransmit(reg & 0xFF);
    PORTB = PORTB | (1 << DDB2);

//
    REG = reg;
}

//
void AD9833_select_output(char freq_sel, char phase_sel)
{
    unsigned long int reg = REG;

    char FSELECT = freq_sel;
    char PSELECT = phase_sel;
    char RESET = 0;

//
    reg = reg & (~(1 << 8 | 1 << 11 | 1 << 10));
    reg = reg | (FSELECT << 11) | (PSELECT << 10) | (RESET << 8);

    PORTB = PORTB & ~(1 << DDB2);
    SPI_MasterTransmit(((reg & 0xFF00) >> 8));
    SPI_MasterTransmit(reg & 0xFF);
    PORTB = PORTB | (1 << DDB2);

//
    REG = reg;
}

//
void AD9833_set_freq(unsigned long freq, char sel)
{
    unsigned long int reg = 0;

//
    reg = ((sel+1) << 14) | (freq & 0x3FFF);
    
    PORTB = PORTB & ~(1 << DDB2);
    SPI_MasterTransmit(((reg & 0xFF00) >> 8));
    SPI_MasterTransmit(reg & 0xFF);
    PORTB = PORTB | (1 << DDB2);

//
    reg = ((sel+1) << 14) | ((freq & 0xFFFC000) >> 14);
    
    PORTB = PORTB & ~(1 << DDB2);
    SPI_MasterTransmit(((reg & 0xFF00) >> 8));
    SPI_MasterTransmit(reg & 0xFF);
    PORTB = PORTB | (1 << DDB2);
}

//
void AD9833_set_phase(unsigned long phase, char sel)
{
    unsigned long int reg = 0;
 
//
    reg = (1 << 15) | (1 << 14) | (sel << 13) | (phase & 0xFFF);
    
    PORTB = PORTB & ~(1 << DDB2);
    SPI_MasterTransmit(((reg & 0xFF00) >> 8));
    SPI_MasterTransmit(reg & 0xFF);
    PORTB = PORTB | (1 << DDB2);
}

//
unsigned long AD9833_set_freq_hz(float freq_hz, char sel, float clock_trim)
{
    unsigned long int freq = 0;
 
//
    freq = freq_hz*((float) ((unsigned long int) 1 << 28))/(F_MCLK+clock_trim*1000);

//
    AD9833_set_freq(freq, sel);

//
    return freq;
}

//
unsigned long AD9833_set_phase_degrees(float phase_degrees, char sel)
{
    unsigned long int phase = 0;

//
    phase = (phase_degrees*4096)/360;

//
    AD9833_set_phase(phase, sel);

//
    return phase;
}

