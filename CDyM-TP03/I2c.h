#ifndef I2C_H_
#define I2C_H_

void    init_i2c(void);
void    i2c_stop(void);
uint8_t i2c_start(uint8_t addr);
uint8_t i2c_write(uint8_t byte_data);
uint8_t i2c_readAck(void);
uint8_t i2c_readNack(void);

#endif /* I2C_H_ */