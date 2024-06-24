#ifndef TWI_H_
#define TWI_H_

#include <avr/io.h>
#include <util/twi.h>

void    TWI_init(void);
void    TWI_stop(void);
void	TWI_start(void);
uint8_t TWI_write(uint8_t byte_data);
uint8_t TWI_readAck(void);
uint8_t TWI_readNack(void);

#endif /* TWI_H_ */