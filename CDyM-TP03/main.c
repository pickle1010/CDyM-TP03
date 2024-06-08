/*
 * CDyM-TP03.c
 *
 * Created: 31/5/2024 21:23:05
 * Author : Estanislao Carrer, Fernando Ramirez, Lisandro Martinez
 */ 

#include "dht11.h"

int main(void)
{	
	uint8_t intRH = 0;
	uint8_t decRH = 0;
	uint8_t intT = 0;
	uint8_t decT = 0;
	uint8_t checksum = 0;
	
	DDRD |= (1 << PORTD0);
	
	DHT11_start();
    while (1)
    {
		if(DHT11_comState == ENDED)
		{
			intRH = DHT11_data[0];
			decRH = DHT11_data[1];
			intT = DHT11_data[2];
			decT = DHT11_data[3];
			checksum = DHT11_data[4];
		}
		if( (PINB & (1 << PORTB0)) == 0)
		{
			PORTD |= (1 << PORTD0);
		}
    }
}