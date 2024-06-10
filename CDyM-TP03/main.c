/*
 * CDyM-TP03.c
 *
 * Created: 31/5/2024 21:23:05
 * Author : Estanislao Carrer, Fernando Ramirez, Lisandro Martinez
 */ 

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

#define DS3232_ADDRESS 0x68  // Dirección I2C del DS3232
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
	
	TWI_Init();
	SerialPort_Init(BR9600); 			// Inicializo formato 8N1 y BAUDRATE = 9600bps
	SerialPort_TX_Enable();				// Activo el Transmisor del Puerto Serie
	SerialPort_RX_Enable();				// Activo el Receptor del Puerto Serie
	SerialPort_RX_Interrupt_Enable();	// Activo Interrupción de recepcion.
	DHT11_init();
	TIMER0_init();						// Activo temporizacion para la lectura del sensor
	sei();								// Activo la mascara global de interrupciones (Bit I del SREG en 1)
	
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
					//segundos
					// Leer el registro de segundos del DS3232 (dirección 0x00)
					TWI_Start();
					TWI_WriteAddress(DS3232_ADDRESS << 1);  // Dirección + bit de escritura (0)
					TWI_WriteByte(0x00);  // Dirección del registro de segundos
					TWI_Start();
					TWI_WriteAddress((DS3232_ADDRESS << 1) | 1);  // Dirección + bit de lectura (1)
					uint8_t seconds = TWI_ReadByte_NACK();
					
					//minutos
					TWI_WriteByte(0x01);  // Dirección del registro de segundos
					TWI_Start();
					TWI_WriteAddress((DS3232_ADDRESS << 1) | 1);  // Dirección + bit de lectura (1)
					uint8_t minutes = TWI_ReadByte_NACK();
					
					//horas
					TWI_WriteByte(0x02);  // Dirección del registro de segundos
					TWI_Start();
					TWI_WriteAddress((DS3232_ADDRESS << 1) | 1);  // Dirección + bit de lectura (1)
					uint8_t hours = TWI_ReadByte_NACK();

					//dia
					TWI_WriteByte(0x04);  // Dirección del registro de segundos
					TWI_Start();
					TWI_WriteAddress((DS3232_ADDRESS << 1) | 1);  // Dirección + bit de lectura (1)
					uint8_t day = TWI_ReadByte_NACK();
					
					//mes
					TWI_WriteByte(0x05);  // Dirección del registro de segundos
					TWI_Start();
					TWI_WriteAddress((DS3232_ADDRESS << 1) | 1);  // Dirección + bit de lectura (1)
					uint8_t month = TWI_ReadByte_NACK();
					
					//year
					TWI_WriteByte(0x06);  // Dirección del registro de segundos
					TWI_Start();
					TWI_WriteAddress((DS3232_ADDRESS << 1) | 1);  // Dirección + bit de lectura (1)
					uint8_t year = TWI_ReadByte_NACK();
					
					TWI_Stop();
					
					seconds = (seconds >> 4) * 10 + (seconds & 0x0F);
					minutes = (minutes >> 4) * 10 + (minutes & 0x0F);
					hours = (hours >> 4) * 10 + (hours & 0x0F);
					day = (day >> 4) * 10 + (day & 0x0F);
					month = (month >> 4) * 10 + (month & 0x0F);
					year = (year >> 4) * 10 + (year & 0x0F);					

					intRH = DHT11_data[0];
					intT = DHT11_data[2];
					
					log_msg[6] = '0' + intT / 10;
					log_msg[7] = '0' + intT % 10;
					log_msg[17] = '0' + intRH / 10;
					log_msg[18] = '0' + intRH % 10;
					
					log_msg[43] = '0' + hours / 10;
					log_msg[44] = '0' + hours % 10;
					log_msg[46] = '0' + minutes / 10;
					log_msg[47] = '0' + minutes % 10;
					log_msg[49] = '0' + seconds / 10;
					log_msg[50] = '0' + seconds % 10;
					
					log_msg[28] = '0' +  day / 10;
					log_msg[29] = '0' +  day % 10;

					log_msg[31] = '0' + month / 10;
					log_msg[32] = '0' + month % 10;

					log_msg[34] = '0' + year / 10;
					log_msg[35] = '0' + year % 10;
					
					SerialPort_TX_Interrupt_Enable();	
				}
			}
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
