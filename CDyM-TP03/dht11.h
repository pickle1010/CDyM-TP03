#ifndef _DHT11_H
#define _DHT11_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

typedef enum {STARTED, INT_RH, DEC_RH, INT_T, DEC_T, CHECKSUM, ENDED} tDHT11_comState;
extern tDHT11_comState DHT11_comState;
extern uint8_t DHT11_data[5];

#define DHT11_PIN_NUM 0
#define DHT11_PORT PORTC
#define DHT11_DDR DDRC
#define DHT11_PIN PINC

void DHT11_init();
void DHT11_start();
void DHT11_update();

#endif