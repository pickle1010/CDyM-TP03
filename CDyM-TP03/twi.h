#ifndef TWI_H_
#define TWI_H_

#include <avr/io.h>
#include <util/twi.h> //TWII=I2C en atmega328p

void    TWI_init(void);
void    TWI_stop(void);
void	TWI_Start(void);
uint8_t TWI_start(uint8_t addr);
uint8_t TWI_write(uint8_t byte_data);
uint8_t TWI_readAck(void);
uint8_t TWI_readNack(void);

//void TWI_Init(void);
//void TWI_Start(void);
//void TWI_WriteAddress(uint8_t address);
//void TWI_WriteByte(uint8_t data);
//uint8_t TWI_ReadByte_ACK(void);
//uint8_t TWI_ReadByte_NACK(void);
//void TWI_Stop(void);

#endif /* TWI_H_ */