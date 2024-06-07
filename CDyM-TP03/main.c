/*
 * CDyM-TP03.c
 *
 * Created: 31/5/2024 21:23:05
 * Author : Estanislao Carrer, Fernando Ramirez, Lisandro Martinez
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void DHT11_start();
void codigo_teoria(); //USA POLLING

typedef enum {START, INT_RH, DEC_RH, INT_T, DEC_T, CHECKSUM, END} tDHT11_state;

volatile tDHT11_state state = END;
volatile uint16_t pulse_us = 0;
volatile uint16_t time_ic = 0;
volatile uint16_t pulse_counter = 0;
volatile uint8_t data[5];
volatile uint8_t data_sel;

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
		if(state == END)
		{
			intRH = data[0];
			decRH = data[1];
			intT = data[2];
			decT = data[3];
			checksum = data[4];
		}
		if( (PINB & (1 << PORTB0)) == 0)
		{
			PORTD |= (1 << PORTD0);
		}
    }
}

void DHT11_start()
{
	// Configurar IC del Timer 1
	TCCR1A = 0x00; // Modo Normal
	TCCR1B = (1 << CS11); // Prescale 8, ICSE1 = 0 (LOW)
	
	state = START;
	pulse_counter = 0;
	for (int i = 0; i < 5; i++)
	{
		data[i] = 0;
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
			state = END;
			TIMSK1 = (0 << ICIE1);
		}
		else if (pulse_counter > 3 && pulse_counter % 2 == 0)
		{
			if( (pulse_counter - 4) % 16 == 0 )
			{
				state++;
			}
			data_sel = state - INT_RH;
			data[data_sel] = data[data_sel] << 1;
			if(pulse_us >= 70)
			{
				data[data_sel] += 1;
			}
		}
	}
	TCCR1B ^= (1 << ICES1);
	pulse_counter++;
}