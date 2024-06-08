#ifndef _DHT11_H
#define _DHT11_H

#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

typedef enum {STARTED, INT_RH, DEC_RH, INT_T, DEC_T, CHECKSUM, ENDED} tDHT11_comState;
extern volatile tDHT11_comState DHT11_comState;
extern volatile uint8_t DHT11_data[5];

void DHT11_start();

#endif