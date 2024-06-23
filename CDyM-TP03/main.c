/*
 * GccApplication4.c
 *
 * Created: 6/6/2024 20:15:10
 * Author : Usuario
 */ 
#define F_CPU 16000000U
#define BR9600 (0x67)	// 0x67=103 configura BAUDRATE=9600@16MHz

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "serialPort.h"
#include "I2c.h"
#include "rtc.h"

char* make_command(void);
void TIMER0_init();

char t[]="Time: ";
char da[]=" Date: ";
char msg[64];

rtc_t today;
rtc_t set_date;

//char stop_msg[] = "Transmision interrumpida";
//char start_msg[] = "Transmision reanudada";

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
	msg = log_msg; //mensaje final, habria que ver como agregarle los datos dht
	uint8_t intRH = 0;
	uint8_t intT = 0;

	
	SerialPort_Init(BR9600); 			// Inicializo formato 8N1 y BAUDRATE = 9600bps
	SerialPort_TX_Enable();				// Activo el Transmisor del Puerto Serie
	SerialPort_RX_Enable();				// Activo el Receptor del Puerto Serie
	SerialPort_RX_Interrupt_Enable();	// Activo Interrupción de recepcion.
	DHT11_init();
	
	TIMER0_init();						// Activo temporizacion para la lectura del sensor
	sei();								// Activo la mascara global de interrupciones (Bit I del SREG en 1)
	// Habilitar transmisor y receptor
								
	init_ds3232();
	
	rtc_t rtc;
	rtc.hour = dec2bcd(15);
	rtc.min =  dec2bcd(53);
	rtc.sec =  dec2bcd(0);	//aca hay que ver como hacemos para poner la hora del sistema que lo inicie y que se una sola vez
	rtc.date = dec2bcd(10);
	rtc.month = dec2bcd(6);
	rtc.year = dec2bcd(24);
	rtc.weekDay = 1;
	ds3232_SetDateTime(&rtc); //seteamos la hora en el ds3232
	
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
					char* m=make_command();
					SerialPort_Wait_For_TX_Buffer_Free();
					SerialPort_Send_String(m);
					SerialPort_TX_Interrupt_Enable();//esto lo deberia dejar para el dht?
				}
			}
		}
	}
}

char* make_command(void)
{
	
	ds3232_GetDateTime(&today);
	char d_y[3];
	char d_m[3];
	char d_d[3];
	char d_h[3];
	char d_mi[3];
	char d_s[3];
	
	uint8_t secu;
	uint8_t mini;
	uint8_t hore;
	
	secu=bcd2bss(today.sec);
	mini=bcd2bss(today.min);
	hore=bcd2bss(today.hour);
	
	itoa(bcd2dec(today.date), d_d, 10);  //day
	itoa(bcd2dec(today.month), d_m, 10); //month
	itoa(bcd2dec(today.year), d_y, 10);  //year

	
	sprintf(msg,"FECHA: %s/%s/%s HORA: %u:%u:%u \r", d_d, d_m, d_y, hore, mini, secu);
	
	return msg;
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
	}
}

ISR(USART_UDRE_vect){
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

/*

int main(void)
{
	// 0x33 corresponde a 9600 bps con 8 MHz
	SerialPort_Init(0x67);

	// Habilitar transmisor y receptor
	SerialPort_TX_Enable();
	SerialPort_RX_Enable();
	
	init_ds3231();
	rtc_t rtc;
	rtc.hour = dec2bcd(15);
	rtc.min =  dec2bcd(53);
	rtc.sec =  dec2bcd(0);
	rtc.date = dec2bcd(10);
	rtc.month = dec2bcd(6);
	rtc.year = dec2bcd(24);
	rtc.weekDay = 1;
	ds3232_SetDateTime(&rtc);

	
	while (1)
	{
		char* m=make_command();
		SerialPort_Wait_For_TX_Buffer_Free();
		SerialPort_Send_String(m);
		_delay_ms(2000);
	}
}



char* make_command(void)
{
	
	ds3232_GetDateTime(&today);
	char d_y[3];
	char d_m[3];
	char d_d[3];
	char d_h[3];
	char d_mi[3];
	char d_s[3];
	
	uint8_t secu;
	uint8_t mini;
	uint8_t hore;
	
	secu=bcd2bss(today.sec);
	mini=bcd2bss(today.min);
	hore=bcd2bss(today.hour);
	
	itoa(bcd2dec(today.date), d_d, 10);  //day
	itoa(bcd2dec(today.month), d_m, 10); //month
	itoa(bcd2dec(today.year), d_y, 10);  //year

	
	sprintf(msg,"FECHA: %s/%s/%s HORA: %u:%u:%u \r", d_d, d_m, d_y, hore, mini, secu);
	
	return msg;
}

