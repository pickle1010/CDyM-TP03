/*
 * CDyM-TP03.c
 *
 * Created: 31/5/2024 21:23:05
 * Author : Estanislao Carrer, Fernando Ramirez, Lisandro Martinez
 */ 

#include "dht11.h"
#include "serialPort.h"

void TIMER0_init();

#define BR9600 (0x67)	// 0x67=103 configura BAUDRATE=9600@16MHz

char log_msg[] = "TEMP: __ °C HUM: __% FECHA: __/__/__ HORA: __:__:__\r\n";

volatile uint16_t counter = 0;
volatile uint8_t start_dht11= 0;
volatile uint8_t read_dht11= 0;
volatile uint8_t PRINT_send = 0;
volatile uint8_t PRINT_done = 1;
volatile uint8_t STOP = 0;

volatile char RX_Buffer=0;
volatile uint8_t tx_index = 0;

int main(void)
{	
	uint8_t intRH = 0;
	uint8_t intT = 0;
	
	SerialPort_Init(BR9600); 			// Inicializo formato 8N1 y BAUDRATE = 9600bps
	SerialPort_TX_Enable();				// Activo el Transmisor del Puerto Serie
	SerialPort_RX_Enable();				// Activo el Receptor del Puerto Serie
	SerialPort_RX_Interrupt_Enable();	// Activo Interrupción de recepcion.
	TIMER0_init();						// Activo temporizacion para la lectura del sensor
	sei();								// Activo la mascara global de interrupciones (Bit I del SREG en 1)
	
	DHT11_start();
    while (1)
    {
		if(start_dht11)
		{
			start_dht11 = 0;
			read_dht11 = 1;
			DHT11_start();
		}
		if(read_dht11 && DHT11_comState == ENDED)
		{
			intRH = DHT11_data[0];
			intT = DHT11_data[2];
			
			log_msg[6] = '0' + intT / 10;
			log_msg[7] = '0' + intT % 10;
			log_msg[17] = '0' + intRH / 10;
			log_msg[18] = '0' + intRH % 10;
			
			read_dht11 = 0;
			PRINT_send = !STOP;
		}
		if(!STOP && PRINT_send && PRINT_done)
		{
			PRINT_send = 0;
			SerialPort_TX_Interrupt_Enable();
			PRINT_done = 0;
		}
    }
}

/********************************************************
CONFIGURAR TIMER PARA GENERAR UNA INTERRUPCION CADA 1 ms
UTILIZANDO UNA FREQUENCIA PRESCALADA DE 250 kHz
********************************************************/
void TIMER0_init(){
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS01) | (1<<CS00);
	TIMSK0 = (1 << OCIE0A);
	OCR0A = 249;
}

/********************************************************
INTERRUPCION DE TIMER 0
********************************************************/
ISR (TIMER0_COMPA_vect)
{
	// ejecutar cada 1 ms
	if (++counter == 2000) {
		//ejecutar cada 2 s
		start_dht11 = 1;
		counter = 0;
	}
}

// Rutina de Servicio de Interrupción de Byte Recibido
ISR(USART_RX_vect)
{
	RX_Buffer = UDR0; //la lectura del UDR borra flag RXC
	if(RX_Buffer == 's' || RX_Buffer ==  'S')
	{
		STOP = !STOP;
	}
}

ISR(USART_UDRE_vect){
	if(log_msg[tx_index] != '\0')
	{
		SerialPort_Send_Data(log_msg[tx_index]);
		tx_index++;
	}
	else
	{
		tx_index = 0;
		SerialPort_TX_Interrupt_Disable();
		PRINT_done = 1;
	}
} 