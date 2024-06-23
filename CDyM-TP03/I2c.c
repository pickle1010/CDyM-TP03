#include <avr/io.h>
#include <util/twi.h> //TWII=I2C en atmega328p
#include "I2c.h"
#define F_CPU 16000000U
#define SCL_CLOCK  100000 //100 kHz
/*************************************************************************
 Inicialización de la interfaz del bus I2C. Necesita ser llamado sólo una vez
*************************************************************************/
void init_i2c(void)
{
  /* se inicializa TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
  
  TWSR = 0;                         /* no prescaler */
  //TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  /* debe ser > 10 para un funcionamiento estable */
  TWBR=152;
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
}/* i2c_init */

void twi_start(void){
	TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
	while (!(TWCR & (1 << TWINT)));
}


uint8_t i2c_start(uint8_t addr) //se envia la direccion del esclavo
{
    uint8_t   twst;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));                    // esperar hasta que se complete la transmision
	twst = TW_STATUS & 0xF8;                    // check valor del registro de estados TWI. Mascara de bits de prescaler.
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;
	TWDR = addr;                             // se envia la direccion del esclavo al dispositivo
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));            // Esperar hasta que la transmisión se complete y se reciba ACK/NACK
	twst = TW_STATUS & 0xF8;               // Leer y enmascarar el registro de estado TWI.
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
	return 0;
}/* i2c_start */


void i2c_stop(void)
{
    /* enviar condicion de stop */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	// esperar hasta que se ejecute la condición de parada y se libere el bus	
	while(TWCR & (1<<TWSTO));
}/* i2c_stop */


/*************************************************************************
  Enviar un byte al dispositivo I2C
  Entrada: byte a transferir
  Retorno:   0 escritura correcta
			 1 escritura fallida
*************************************************************************/
uint8_t i2c_write(uint8_t byte_data)
{	
    uint8_t   twst;
    
	// enviar datos al dispositivo previamente direccionado
	TWDR = byte_data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// esperar el fin de la transfer.
	while(!(TWCR & (1<<TWINT)));

	// comprueba el valor del Registro de Estado TWI. Enmascarar bits de prescaler
	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;
}/* i2c_write */

/*************************************************************************
 Leer un byte del dispositivo I2C, solicitar más datos del dispositivo
 
 Retorno: byte leído del dispositivo I2C
*************************************************************************/
uint8_t i2c_readAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));    

    return TWDR;
}/* i2c_readAck */

/*************************************************************************
 Lee un byte del dispositivo I2C, la lectura va seguida de una condición de stop
 
 Retorno: byte leído del dispositivo I2C
*************************************************************************/
uint8_t i2c_readNack(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	
    return TWDR;
}/* i2c_readNak */