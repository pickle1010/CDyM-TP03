#include  "rtc.h"
#include  "I2c.h"

void init_ds3232(void)
{
    init_i2c();                             // Inicializa el modulo I2c .
    //i2c_start(0x68);						// inicia comunicacion I2C
	twi_start(); 
    i2c_write(DS3232_WriteMode_U8);          // conectar a ds3232
    i2c_write(DS3232_REG_CONTROL);          // seleccionar el ControlRegister de ds3232 
    i2c_write(0x00);                        // escribe 0x00 al Control register para desactivar SQW-Out
    i2c_stop();                             // termina la comunicacion I2C luego de inicializar ds3232
}

/***************************************************************************************************
                    void ds3232_SetDateTime(rtc_t *rtc)
****************************************************************************************************/

void ds3232_SetDateTime(rtc_t *rtc)
{
    //i2c_start(0x68);                          // inicia comunicacion I2C
	twi_start();
    i2c_write(DS3232_WriteMode_U8);      // conectar a ds3232 enviando su ID al I2c Bus
    i2c_write(DS3232_REG_Seconds);       // Petición de dirección sec RAM a 00H

    i2c_write(rtc->sec);                    // escribe sec desde la direccion RAM 00H
    i2c_write(rtc->min);                    // escribe min desde la direccion RAM 01H
    i2c_write(rtc->hour);                    // escribe hor desde la direccion RAM 02H
    i2c_write(rtc->weekDay);                // escribe weekday desde la direccion RAM 03H
    i2c_write(rtc->date);                    // escribe dia desde la direccion RAM 04H
    i2c_write(rtc->month);                    //escribe mes desde la direccion RAM 05H
    i2c_write(rtc->year);                    // escribe anio desde la direccion RAM 06H
	
    i2c_stop();                              // Detener la comunicación I2C después de ajustar la fecha
}

/***************************************************************************************************
                     void ds3232_GetDateTime(rtc_t *rtc)
****************************************************************************************************/
void ds3232_GetDateTime(rtc_t *rtc)
{
    //i2c_start(0x68);                            // inicia comunicacion I2C
	twi_start(); 
    i2c_write(DS3232_WriteMode_U8);        // conecta a ds3232 enviando su id al I2c Bus
    i2c_write(DS3232_REG_Seconds);         // Solicitar dirección Sec RAM en 00H
    i2c_stop();                            // Detener comunicación I2C después de seleccionar Registro Sec.

    //i2c_start(0x68);                            // inicia comunicacion I2C
	twi_start();
    i2c_write(DS3232_ReadMode_U8);            // conectar al ds3232(Read mode)enviando su ID

    rtc->sec = i2c_readAck();                // leer second y retornar Positive ACK
    rtc->min = i2c_readAck();                 // leer minute y retornar Positive ACK
    rtc->hour= i2c_readAck();               // leer hour y retornar Negative/No ACK
    rtc->weekDay = i2c_readAck();           // leer weekDay y retornar Positive ACK
    rtc->date= i2c_readAck();              // leer Date y retornar Positive ACK
    rtc->month=i2c_readAck();            // leer Month y retornar Positive ACK
    rtc->year =i2c_readNack();             // leer Year y retornar Negative/No ACK

    i2c_stop();                              // Detener la comunicación I2C después de ajustar la fecha
}



uint8_t dec2bcd(char num)
{
	return ((num/10 * 16) + (num % 10));
}

// conversiones decimal - binario con decimal - binario sin signo.

uint8_t bcd2dec(char num)
{
	return ((num/16 * 10) + (num % 16));
}

uint8_t bcd2bss(char num)
{
	return ((num >> 4) * 10 + (num & 0x0F));
}