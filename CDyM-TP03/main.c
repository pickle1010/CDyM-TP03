/*
 * CDyM-TP03.c
 *
 * Created: 31/5/2024 21:23:05
 * Author : Estanislao Carrer, Fernando Ramirez, Lisandro Martinez
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "serialPort.h"

#define DS3232_ADDRESS 0x68  // Dirección I2C del DS3232

void TWI_Init(void);
void TWI_Start(void);
void TWI_WriteAddress(uint8_t address);
void TWI_WriteByte(uint8_t data);
uint8_t TWI_ReadByte_ACK(void);
uint8_t TWI_ReadByte_NACK(void);
void TWI_Stop(void);

int main(void) {
	TWI_Init();
	SerialPort_Init(0x67);
	SerialPort_TX_Enable();

	while (1) {
		
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
		
		TWI_Stop();
		
		seconds = (seconds >> 4) * 10 + (seconds & 0x0F);
		minutes = (minutes >> 4) * 10 + (minutes & 0x0F);
		hours = (hours >> 4) * 10 + (hours & 0x0F);

		//Imprimir segundos leidos
		char buffer[100];
		sprintf(buffer, "hours: %02d minutes: %02d Seconds: %02d\r", hours,minutes,seconds);
		
		
		
		SerialPort_Wait_For_TX_Buffer_Free();
		SerialPort_Send_String(buffer);
		
		_delay_ms(10000);
		
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
