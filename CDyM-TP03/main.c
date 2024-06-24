/*
 * CDyM-TP03.c
 *
 * Created: 31/5/2024 21:23:05
 * Author : Estanislao Carrer, Fernando Ramirez, Lisandro Martinez
 */ 

#include <stdio.h>
#include "dht11.h"
#include "serialPort.h"

void TWI_Init(void);
void TWI_Start(void);
void TWI_WriteAddress(uint8_t address);
void TWI_WriteByte(uint8_t data);
uint8_t TWI_ReadByte_ACK(void);
uint8_t TWI_ReadByte_NACK(void);
void TWI_Stop(void);

void TIMER0_init();
uint8_t bin_to_bcd(uint8_t);
uint8_t bcd_to_bin(uint8_t);
void MAIN_init();

#define DS3232_ADDRESS 0x68  // Dirección I2C del DS3232
#define BR9600 (0x67)	// 0x67=103 configura BAUDRATE=9600@16MHz

#define INITIAL_HOURS 15
#define INITIAL_MINUTES 30
#define INITIAL_SECONDS 00

#define INITIAL_YEAR 24
#define INITIAL_MONTH 6
#define INITIAL_DAY 24

typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
} Date;

typedef struct
{
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} Time;

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
Time time_var;
Date date;

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
					TWI_Start();
					TWI_WriteAddress(DS3232_ADDRESS << 1);  // Dirección + bit de escritura (0)
					TWI_WriteByte(0x00);  // Dirección del registro de segundosuint8_t seconds = TWI_ReadByte_ACK();
					TWI_Stop();
					
					TWI_Start();
					TWI_WriteAddress((DS3232_ADDRESS << 1) | 1);  // Dirección + bit de lectura (1)
					time_var.seconds = TWI_ReadByte_ACK();
					time_var.minutes = TWI_ReadByte_ACK();
					time_var.hours = TWI_ReadByte_ACK();
					TWI_ReadByte_ACK();
					date.day = TWI_ReadByte_ACK();
					date.month = TWI_ReadByte_ACK();
					date.year = TWI_ReadByte_NACK();
					TWI_Stop();
					
					time_var.hours = bcd_to_bin(time_var.hours);
					time_var.minutes = bcd_to_bin(time_var.minutes);
					time_var.seconds = bcd_to_bin(time_var.seconds);
					
					date.day = bcd_to_bin(date.day);
					date.month = bcd_to_bin(date.month);
					date.year = bcd_to_bin(date.year);

					hum = DHT11_data[0];
					temp = DHT11_data[2];
					
					sprintf(log_msg, "TEMP: %u °C HUM: %u%% FECHA: %02u/%02u/%02u HORA: %02u:%02u:%02u\r\n", temp, hum, date.day, date.month, date.year, time_var.hours, time_var.minutes, time_var.seconds);
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
	time_var.hours = bin_to_bcd(INITIAL_HOURS);
	time_var.minutes = bin_to_bcd(INITIAL_MINUTES);
	time_var.seconds = bin_to_bcd(INITIAL_SECONDS);
	
	date.year = bin_to_bcd(INITIAL_YEAR);
	date.month = bin_to_bcd(INITIAL_MONTH); 
	date.day = bin_to_bcd(INITIAL_DAY);
	
	TWI_Init();
	SerialPort_Init(BR9600); 			// Inicializo formato 8N1 y BAUDRATE = 9600bps
	SerialPort_TX_Enable();				// Activo el Transmisor del Puerto Serie
	SerialPort_RX_Enable();				// Activo el Receptor del Puerto Serie
	SerialPort_RX_Interrupt_Enable();	// Activo Interrupción de recepcion
	DHT11_init();
	
	TWI_Start();
	TWI_WriteAddress(DS3232_ADDRESS << 1);  // Dirección + bit de escritura (0)
	TWI_WriteByte(0x00);  // Dirección del registro de segundosuint8_t seconds = TWI_ReadByte_ACK();
	TWI_WriteByte(time_var.seconds);
	TWI_WriteByte(time_var.minutes);
	TWI_WriteByte(time_var.hours);
	TWI_WriteByte(0);
	TWI_WriteByte(date.day);
	TWI_WriteByte(date.month);
	TWI_WriteByte(date.year);
	TWI_Stop();
	
	next_msg = startup_msg;
	PRINT_send = 1;
	
	TIMER0_init();						// Activo temporizacion para la lectura del sensor
	sei();								// Activo la mascara global de interrupciones (Bit I del SREG en 1)
}

/********************************************************
FUNCIÓN PARA CONVERTIR UN VALOR BINARIO A BCD
********************************************************/
uint8_t bin_to_bcd(uint8_t val)
{
	return ((val / 10) << 4) | (val % 10);
}

/********************************************************
FUNCIÓN PARA CONVERTIR UN VALOR BCD A BINARIO
********************************************************/
uint8_t bcd_to_bin(uint8_t val)
{
	return ((val >> 4) * 10) + (val & 0x0F);
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

void TWI_Init(void) {
	TWSR = 0x00;  // Prescaler value = 1
	TWBR = 152;  // Set bit rate register for 50 kHz SCL frequency
	TWCR = (1 << TWEN);  // Habilitar TWI
}

void TWI_Start(void) {
	TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
	while (!(TWCR & (1 << TWINT)));  // Esperar hasta que se complete el inicio
}

void TWI_WriteAddress(uint8_t address) {
	TWDR = address;
	TWCR = (1 << TWEN) | (1 << TWINT);
	while (!(TWCR & (1 << TWINT)));  // Esperar hasta que se complete la transmisión
}

void TWI_WriteByte(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWEN) | (1 << TWINT);
	while (!(TWCR & (1 << TWINT)));  // Esperar hasta que se complete la transmisión
}

uint8_t TWI_ReadByte_ACK(void) {
	TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);  // Habilitar ACK
	while (!(TWCR & (1 << TWINT)));  // Esperar hasta que se complete la recepción
	return TWDR;
}

uint8_t TWI_ReadByte_NACK(void) {
	TWCR = (1 << TWEN) | (1 << TWINT);  // Deshabilitar ACK
	while (!(TWCR & (1 << TWINT)));  // Esperar hasta que se complete la recepción
	return TWDR;
}

void TWI_Stop(void) {
	TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);
	while (TWCR & (1 << TWSTO));  // Esperar hasta que se complete la parada
}
