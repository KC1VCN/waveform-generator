#ifndef F_CPU
#define F_CPU 16000000L
#endif

#ifndef F_MCLK
#define F_MCLK 25000000L
#endif

#include <avr/io.h>

unsigned long int REG;

void SPI_MasterInit(void);
void SPI_MasterTransmit(char);

void AD9833_init(void);
void AD9833_set_waveform(char);
void AD9833_select_output(char, char);
void AD9833_set_freq(unsigned long, char);
void AD9833_set_phase(unsigned long, char);
unsigned long AD9833_set_freq_hz(float, char, float);
unsigned long AD9833_set_phase_degrees(float, char);

