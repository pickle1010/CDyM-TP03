#ifndef RTC_H_
#define RTC_H_

#include <avr/io.h>
#include  "twi.h"

typedef struct
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t weekDay;
	uint8_t date;
	uint8_t month;
	uint8_t year;
} rtc_t;

#define DS3232_ADDRESS 0x68  // Dirección I2C del DS3232

#define DS3232_ReadMode_U8          (0xD1)
#define DS3232_WriteMode_U8         (0xD0)
#define DS3232_REG_Seconds          (0x00)
#define DS3232_REG_Minutes          (0x01)
#define DS3232_REG_Hour             (0x02)
#define DS3232_REG_Day              (0x03)
#define DS3232_Date                 (0x04)
#define DS3232_Month                (0x05)
#define DS3232_Year                 (0x06)
#define DS3232_REG_ALARM_1          (0x07)
#define DS3232_REG_ALARM_2          (0x0B)
#define DS3232_REG_CONTROL          (0x0E)
#define DS3232_REG_STATUS           (0x0F)
#define DS3232_REG_TEMPERATURE      (0x11)

void    DS3232_init(void);
void    DS3232_setDateTime(rtc_t *rtc);
void    DS3232_getDateTime(rtc_t *rtc);

uint8_t DS3232_bin2bcd(uint8_t val);
uint8_t DS3232_bcd2bin(uint8_t val);
/**************************************************************************************************/

#endif /* RTC_H_ */