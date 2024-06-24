#include "twi.h"
//#define F_CPU 16000000U
//#define SCL_CLOCK 100000 //100 kHz

/*************************************************************************
 Inicialización de la interfaz del bus I2C. Necesita ser llamado sólo una vez
*************************************************************************/
void TWI_init(void)
{
  /* se inicializa TWI clock: 50 kHz clock, TWPS = 0 => prescaler = 1 */
  
  TWSR = 0x00;	// Prescaler value = 1
  //TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  /* debe ser > 10 para un funcionamiento estable */
  TWBR = 152; // Set bit rate register for 50 kHz SCL frequency
  //TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
  TWCR = (1 << TWEN);  // Habilitar TWI
}/* TWI_init */

void TWI_start(void) {
	TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
	while (!(TWCR & (1 << TWINT)));  // Esperar hasta que se complete el inicio
}/* TWI_start */

void TWI_stop(void)
{
    /* enviar condicion de stop */
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	// esperar hasta que se ejecute la condición de parada y se libere el bus	
	while (TWCR & (1 << TWSTO));
}/* TWI_stop */

/*************************************************************************
  Enviar un byte al dispositivo I2C
  Entrada: byte a transferir
  Retorno:   0 escritura correcta
			 1 escritura fallida
*************************************************************************/
uint8_t TWI_write(uint8_t byte_data)
{	
    uint8_t twst;
    
	// Enviar datos al dispositivo previamente direccionado
	TWDR = byte_data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// Esperar el fin de la transfer.
	while(!(TWCR & (1<<TWINT)));

	// Comprueba el valor del Registro de Estado TWI. Enmascarar bits de prescaler
	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;
}/* TWI_write */

/*************************************************************************
 Leer un byte del dispositivo I2C, solicitar más datos del dispositivo
 
 Retorno: byte leído del dispositivo I2C
*************************************************************************/
uint8_t TWI_readAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));
    return TWDR;
}/* TWI_readAck */

/*************************************************************************
 Lee un byte del dispositivo I2C, la lectura va seguida de una condición de stop
 
 Retorno: byte leído del dispositivo I2C
*************************************************************************/
uint8_t TWI_readNack(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	
    return TWDR;
}/* TWI_readNak */