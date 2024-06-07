#include "dht11.h"

volatile tDHT11_state DHT11_state = END;
volatile uint8_t DHT11_data[5];

static volatile uint16_t pulse_us = 0;
static volatile uint16_t time_ic = 0;
static volatile uint16_t pulse_counter = 0;
static volatile uint8_t data_sel;

void DHT11_start()
{
	// Configurar IC del Timer 1
	TCCR1A = 0x00; // Modo Normal
	TCCR1B = (1 << CS11); // Prescale 8, ICSE1 = 0 (LOW)
	
	DHT11_state = START;
	pulse_counter = 0;
	for (int i = 0; i < 5; i++)
	{
		DHT11_data[i] = 0;
	}
	
	// Enviar señal de start al modulo DHT11
	PORTB &= ~(1<<PORTB0);
	DDRB |= (1<<PORTB0);
	_delay_ms(18);
	DDRB &= ~(1<<PORTB0);
	
	TIMSK1 = (1 << ICIE1); // Habilitar interrupción de IC
	sei();
}

ISR (TIMER1_CAPT_vect)
{
	if(pulse_counter == 0)
	{
		TCNT1 = 0x0000;
		pulse_us = 0;
		time_ic = 0;
	}
	else
	{
		pulse_us = (ICR1 - time_ic) / 2;
		time_ic = ICR1;
		if (pulse_counter >= 83)
		{
			// Fin de la comunicacion
			DHT11_state = END;
			TIMSK1 = (0 << ICIE1);
		}
		else if (pulse_counter > 3 && pulse_counter % 2 == 0)
		{
			if( (pulse_counter - 4) % 16 == 0 )
			{
				DHT11_state++;
			}
			data_sel = DHT11_state - INT_RH;
			DHT11_data[data_sel] = DHT11_data[data_sel] << 1;
			if(pulse_us >= 70)
			{
				DHT11_data[data_sel] += 1;
			}
		}
	}
	TCCR1B ^= (1 << ICES1);
	pulse_counter++;
}