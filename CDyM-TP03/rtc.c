#include  "rtc.h"

void DS3231_init(void)
{
    TWI_init();		// Inicializa el modulo I2C.
	TWI_start();
    TWI_write(DS3231_WriteMode_U8);		// Conectar a DS3231
    TWI_write(DS3231_REG_CONTROL);		// Seleccionar el ControlRegister de DS3231
    TWI_write(0x00);	// Escribe 0x00 al Control register para desactivar SQW-Out
    TWI_stop();			// Termina la comunicacion I2C luego de inicializar DS3231
}

/***************************************************************************************************
                    void DS3231_SetDateTime(rtc_t *rtc)
****************************************************************************************************/
void DS3231_setDateTime(rtc_t *rtc)
{
	rtc->sec = DS3231_bin2bcd(rtc->sec);
	rtc->min = DS3231_bin2bcd(rtc->min);
	rtc->hour = DS3231_bin2bcd(rtc->hour);
	rtc->weekDay = DS3231_bin2bcd(rtc->weekDay);
	rtc->date = DS3231_bin2bcd(rtc->date);
	rtc->month = DS3231_bin2bcd(rtc->month);
	rtc->year = DS3231_bin2bcd(rtc->year);
	
	TWI_start();
    TWI_write(DS3231_WriteMode_U8);	// conectar a DS3231 enviando su ID al I2c Bus
    TWI_write(DS3231_REG_Seconds);	// Petición de dirección sec RAM a 00H

    TWI_write(rtc->sec);		// escribe sec desde la direccion RAM 00H
    TWI_write(rtc->min);		// escribe min desde la direccion RAM 01H
    TWI_write(rtc->hour);		// escribe hor desde la direccion RAM 02H
    TWI_write(rtc->weekDay);	// escribe weekday desde la direccion RAM 03H
    TWI_write(rtc->date);		// escribe dia desde la direccion RAM 04H
    TWI_write(rtc->month);		//escribe mes desde la direccion RAM 05H
    TWI_write(rtc->year);		// escribe anio desde la direccion RAM 06H
	
    TWI_stop();	// Detener la comunicación I2C después de ajustar la fecha
}

/***************************************************************************************************
                     void DS3231_GetDateTime(rtc_t *rtc)
****************************************************************************************************/
void DS3231_getDateTime(rtc_t *rtc)
{
	TWI_start(); 
    TWI_write(DS3231_WriteMode_U8);		// conecta a DS3231 enviando su id al I2c Bus
    TWI_write(DS3231_REG_Seconds);		// Solicitar dirección Sec RAM en 00H
    TWI_stop();		// Detener comunicación I2C después de seleccionar Registro Sec.

	TWI_start();
    TWI_write(DS3231_ReadMode_U8);	// conectar al DS3231(Read mode)enviando su ID

    rtc->sec = TWI_readAck();		// leer second y retornar Positive ACK
    rtc->min = TWI_readAck();		// leer minute y retornar Positive ACK
    rtc->hour= TWI_readAck();		// leer hour y retornar Negative/No ACK
    rtc->weekDay = TWI_readAck();	// leer weekDay y retornar Positive ACK
    rtc->date = TWI_readAck();		// leer Date y retornar Positive ACK
    rtc->month = TWI_readAck();		// leer Month y retornar Positive ACK
    rtc->year = TWI_readNack();		// leer Year y retornar Negative/No ACK

    TWI_stop();		// Detener la comunicación I2C después de ajustar la fecha
	
	rtc->sec = DS3231_bcd2bin(rtc->sec);
	rtc->min = DS3231_bcd2bin(rtc->min);
	rtc->hour = DS3231_bcd2bin(rtc->hour);
	rtc->weekDay = DS3231_bcd2bin(rtc->weekDay);
	rtc->date = DS3231_bcd2bin(rtc->date);
	rtc->month = DS3231_bcd2bin(rtc->month);
	rtc->year = DS3231_bcd2bin(rtc->year);
}

/********************************************************
FUNCIÓN PARA CONVERTIR UN VALOR BINARIO A BCD
********************************************************/
uint8_t DS3231_bin2bcd(uint8_t val)
{
	return ((val / 10) << 4) | (val % 10);
}

/********************************************************
FUNCIÓN PARA CONVERTIR UN VALOR BCD A BINARIO
********************************************************/
uint8_t DS3231_bcd2bin(uint8_t val)
{
	return ((val >> 4) * 10) + (val & 0x0F);
}