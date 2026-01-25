#include "iic_stm32.h"
#include "ds3231.h"
#include "main.h"

#define DS3231_REG_SECOND               0x00        /**< second register */
#define DS3231_REG_MINUTE               0x01        /**< minute register */
#define DS3231_REG_HOUR                 0x02        /**< hour register */
#define DS3231_REG_WEEK                 0x03        /**< week register */
#define DS3231_REG_DATE                 0x04        /**< date register */
#define DS3231_REG_MONTH                0x05        /**< month register */
#define DS3231_REG_YEAR                 0x06        /**< year register */
//#define DS3231_REG_ALARM1_SECOND        0x07        /**< alarm1 second register */
//#define DS3231_REG_ALARM1_MINUTE        0x08        /**< alarm1 minute register */
//#define DS3231_REG_ALARM1_HOUR          0x09        /**< alarm1 hour register */
//#define DS3231_REG_ALARM1_WEEK          0x0A        /**< alarm1 week register */
//#define DS3231_REG_ALARM2_MINUTE        0x0B        /**< alarm2 minute register */
//#define DS3231_REG_ALARM2_HOUR          0x0C        /**< alarm2 hour register */
//#define DS3231_REG_ALARM2_WEEK          0x0D        /**< alarm2 week register */
#define DS3231_REG_CONTROL              0x0E        /**< control register */
#define DS3231_REG_STATUS               0x0F        /**< status register */
#define DS3231_REG_XTAL                 0x10        /**< xtal register */
#define DS3231_REG_TEMPERATUREH         0x11        /**< temperature high register */
#define DS3231_REG_TEMPERATUREL         0x12        /**< temperature low register */

#define DS3231_ADDRESS        0x68        /**0xD0 < iic device address */

ds3231_time_t timeData;
ds3231_time_t *get_timeData(void)
{
	return &timeData;
}

static inline uint8_t ds3231_hex2bcd(uint8_t val)
{
    uint8_t i, j, k;

    i = val / 10;            /* get tens place */
    j = val % 10;            /* get ones place */
    k = j + (i << 4);        /* set bcd */

    return k;                /* return bcd */
}

static inline uint8_t ds3231_bcd2hex(uint8_t val)
{
    uint8_t temp;

    temp = val & 0x0F;              /* get ones place */
    val = (val >> 4) & 0x0F;        /* get tens place */
    val = val * 10;                 /* set tens place */
    temp = temp + val;              /* get hex */

    return temp;                    /* return hex */
}

static inline uint8_t ds3231_get_status(void)
{
	uint8_t statusReg;
	iic_readData(DS3231_ADDRESS, DS3231_REG_STATUS, &statusReg, 1);
	if((statusReg >> 2) & 0x01)
	{
		return 1;	//busy
	}
	return 0;
}

int8_t ds3231_set_time(ds3231_time_t *t)	//if(main_flags.rtc_enabled)
{
	if(ds3231_get_status()) return 1;

    uint8_t century = 0;				//ignored

    if(iic_writeData(DS3231_ADDRESS, DS3231_REG_SECOND, ds3231_hex2bcd(t->second))) return 1;

    if(iic_writeData(DS3231_ADDRESS, DS3231_REG_MINUTE, ds3231_hex2bcd(t->minute))) return 1;

    if (t->format)                       						/* if 12H set hour in 24H */
    {
    	if(iic_writeData(DS3231_ADDRESS, DS3231_REG_HOUR, ((0 << 6) | ds3231_hex2bcd(t->hour)))) return 1;
    }
    else if(iic_writeData(DS3231_ADDRESS, DS3231_REG_HOUR, ds3231_hex2bcd(t->hour))) return 1;

    if(iic_writeData(DS3231_ADDRESS, DS3231_REG_WEEK, ds3231_hex2bcd(t->week))) return 1;

    if(iic_writeData(DS3231_ADDRESS, DS3231_REG_DATE, ds3231_hex2bcd(t->date))) return 1;

    if(iic_writeData(DS3231_ADDRESS, DS3231_REG_MONTH, ds3231_hex2bcd(t->month) | (century << 7))) return 1;

    if(iic_writeData(DS3231_ADDRESS, DS3231_REG_YEAR, ds3231_hex2bcd((uint8_t)(t->year - 2000)))) return 1;

    return 0;
}

int8_t ds3231_get_time(ds3231_time_t *t)		//if(main_flags.rtc_enabled)
{
    uint8_t buf[7];

    memset(buf, 0, sizeof(uint8_t) * 7);                                                  /* clear the buffer */

    if(iic_readData(DS3231_ADDRESS, DS3231_REG_SECOND, (uint8_t *)buf, 7)) return 1;       /* multiple_read */

    t->year = ds3231_bcd2hex(buf[6]) + 2000 + ((buf[5] >> 7) & 0x01) * 100;             /* get year */
    t->month = ds3231_bcd2hex(buf[5]&0x1F);                                             /* get month */
    t->week = ds3231_bcd2hex(buf[3]);                                                   /* get week */
    t->date = ds3231_bcd2hex(buf[4]);                                                   /* get date */

    t->format = ((buf[2] >> 6) & 0x01);                                  /* get format */
    if (t->format)                                                   /* if 12H */
    {
        t->hour = ds3231_bcd2hex(buf[2] & 0x1F);                                        /* get hour */
    }
    else t->hour = ds3231_bcd2hex(buf[2] & 0x3F);                                        /* get hour */

    t->minute = ds3231_bcd2hex(buf[1]);                                                 /* get minute */
    t->second = ds3231_bcd2hex(buf[0]);                                                 /* get second */

    return 0;
}

int8_t init_ds3231(void)
{
	if(iic_check(DS3231_ADDRESS))
	{
		main_flags.rtc_enabled = 0;
		return 1;
	}

	uint8_t controlReg = 0x00;		//set defaults and 1Hz SQW
	uint8_t statusReg;

	if(iic_readData(DS3231_ADDRESS, DS3231_REG_STATUS, &statusReg, 1))
	{
		main_flags.rtc_enabled = 0;
		return 1;
	}

	if(statusReg)	//then reset Oscillator Stop Flag and disable 32kHz Output
	{
		if(iic_writeData(DS3231_ADDRESS, DS3231_REG_STATUS, controlReg) ||
				iic_writeData(DS3231_ADDRESS, DS3231_REG_CONTROL, controlReg))
		{
			main_flags.rtc_enabled = 0;
			return 1;
		}
	}

	ds3231_get_time(&timeData);
	main_flags.rtc_enabled = 1;
    return 0;
}


