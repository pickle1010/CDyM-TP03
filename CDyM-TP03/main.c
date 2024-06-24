/*
 * GccApplication4.c
 *
 * Created: 6/6/2024 20:15:10
 * Author : Usuario
 */ 
#include <stdio.h>
#include "dht11.h"
#include "serialPort.h"
#include "rtc.h"

void TIMER0_init();
void MAIN_init();

#define BR9600 (0x67)	// 0x67=103 configura BAUDRATE=9600@16MHz

#define INITIAL_HOURS 15
#define INITIAL_MINUTES 30
#define INITIAL_SECONDS 00

#define INITIAL_YEAR 24
#define INITIAL_MONTH 6
#define INITIAL_DAY 24

char* msg;
char* next_msg;

char startup_msg[] = 
"*   Sistema de Registro de Datos   *\r\n"
"*  Temperatura y Humedad Relativa  *\r\n"
"Desarrollado por Estanislao Carrer, Fernando Ramirez, Lisandro Martinez\r\n"
"Sistema listo para operacion. Presione 's' o 'S' para detener o reanudar el envio de datos\r\n\r\n";

char log_msg[] = "TEMP: __ °C HUM: __% FECHA: __/__/__ HORA: __:__:__\r\n";
char stop_msg[] = "Transmision interrumpida. Presione 's' o 'S' para reanudarla\r\n";
char start_msg[] = "Transmision reanudada\r\n";

uint8_t temp;
uint8_t hum;
rtc_t rtc_info;

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
	MAIN_init();
	
	while (1)
	{
		if(start_dht11)
		{
			start_dht11 = 0;
			read_dht11 = 1;
			DHT11_start();
		}
		if(read_dht11)
		{
			if(DHT11_comState != ENDED)
			{
				DHT11_update();
			}
			else
			{
				read_dht11 = 0;
				if(!STOP){
					DS3232_getDateTime(&rtc_info);

					hum = DHT11_data[0];
					temp = DHT11_data[2];
					
					sprintf(log_msg, "TEMP: %u °C HUM: %u%% FECHA: %02u/%02u/%02u HORA: %02u:%02u:%02u\r\n", temp, hum, rtc_info.date, rtc_info.month, rtc_info.year, rtc_info.hour, rtc_info.min, rtc_info.sec);
					PRINT_send = 1;
					next_msg = log_msg;
				}
			}
		}
		if(PRINT_send && PRINT_done)
		{
			PRINT_send = 0;
			PRINT_done = 0;
			msg = next_msg;
			SerialPort_TX_Interrupt_Enable();
		}
    }
}

void MAIN_init()
{	
	hum = 0;
	temp = 0;
	
	SerialPort_Init(BR9600); 			// Inicializo formato 8N1 y BAUDRATE = 9600bps
	SerialPort_TX_Enable();				// Activo el Transmisor del Puerto Serie
	SerialPort_RX_Enable();				// Activo el Receptor del Puerto Serie
	SerialPort_RX_Interrupt_Enable();	// Activo Interrupción de recepcion
	
	DHT11_init();
	
	DS3232_init();
	
	rtc_info.sec = INITIAL_SECONDS;
	rtc_info.min = INITIAL_MINUTES;
	rtc_info.hour = INITIAL_HOURS;
	rtc_info.date = INITIAL_DAY;
	rtc_info.month = INITIAL_MONTH;
	rtc_info.year = INITIAL_YEAR;
	DS3232_setDateTime(&rtc_info);
	
	next_msg = startup_msg;
	PRINT_send = 1;
	
	TIMER0_init();						// Activo temporizacion para la lectura del sensor
	sei();								// Activo la mascara global de interrupciones (Bit I del SREG en 1)
}

/********************************************************
CONFIGURAR TIMER PARA GENERAR UNA INTERRUPCION CADA 1 ms
UTILIZANDO UNA FREQUENCIA PRESCALADA DE 250 kHz
********************************************************/
void TIMER0_init()
{
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS01) | (1<<CS00);
	TIMSK0 = (1 << OCIE0A);
	OCR0A = 249;
}

/********************************************************
INTERRUPCION DE TIMER 0
*******************************************************/
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
		if(STOP)
		{
			next_msg = stop_msg;
		}
		else
		{
			next_msg = start_msg;
		}
		PRINT_send = 1;
	}
}

ISR(USART_UDRE_vect)
{
	if(msg[tx_index] != '\0')
	{
		SerialPort_Send_Data(msg[tx_index]);
		tx_index++;
	}
	else
	{
		tx_index = 0;
		SerialPort_TX_Interrupt_Disable();
		PRINT_done = 1;
	}
}