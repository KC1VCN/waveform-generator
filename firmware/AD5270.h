#ifndef F_CPU
#define F_CPU 16000000L
#endif

#include <avr/io.h>

void SPI_MasterInit(void);
void SPI_MasterTransmit(char);

void AD5270_init(void);
void AD5270_set_output(unsigned int);

