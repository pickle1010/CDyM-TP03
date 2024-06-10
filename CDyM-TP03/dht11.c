#include "dht11.h"

tDHT11_comState DHT11_comState = ENDED;
uint8_t DHT11_data[5];

static uint16_t pulse_us = 0;
static uint16_t pulse_counter = 0;
static uint8_t data_sel;

static uint16_t prev_time;
static uint16_t curr_time;
static uint8_t prev_edge;
static uint8_t curr_edge;

void DHT11_init()
{
	// Configurar Timer 1
	TCCR1A = 0x00;			// Modo Normal
	TCCR1B = (1 << CS11);	// Prescale 8
}

void DHT11_start()
{	
	DHT11_comState = STARTED;
	pulse_counter = 0;
	for (int i = 0; i < 5; i++)
	{
		DHT11_data[i] = 0;
	}
	prev_time = 0;
	curr_time = 0;
	prev_edge = (1 << DHT11_PIN_NUM);
	
	// Enviar señal de start al modulo DHT11
	DHT11_PORT &= ~(1<<DHT11_PIN_NUM);
	DHT11_DDR |= (1<<DHT11_PIN_NUM);
	_delay_ms(18);
	DHT11_DDR &= ~(1<<DHT11_PIN_NUM);
}

void DHT11_update()
{
	curr_time = TCNT1;
	curr_edge = DHT11_PIN & (1 << DHT11_PIN_NUM);
	
	if(prev_edge != curr_edge)
	{	
		pulse_us = (curr_time - prev_time) / 2;
		prev_time = curr_time;
		if (pulse_counter >= 83)
		{
			DHT11_comState = ENDED; // Fin de la comunicacion
		}
		else if (pulse_counter > 3 && pulse_counter % 2 == 0)
		{
			if( (pulse_counter - 4) % 16 == 0 )
			{
				DHT11_comState++;
			}
			data_sel = DHT11_comState - INT_RH;
			DHT11_data[data_sel] = DHT11_data[data_sel] << 1;
			if(pulse_us >= 60)
			{
				DHT11_data[data_sel] += 1;
			}
		}
		
		pulse_counter++;
		prev_edge = curr_edge;
	}
}