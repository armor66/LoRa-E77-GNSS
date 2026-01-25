#ifndef DS3231_H_
#define DS3231_H_

//#include <stdio.h>
//#include <stdint.h>
#include <string.h>

typedef struct ds3231_time_s
{
    uint16_t year;                 /**< year */
    uint8_t month;                 /**< month */
    uint8_t week;                  /**< week */
    uint8_t date;                  /**< date */
    uint8_t hour;                  /**< hour */
    uint8_t minute;                /**< minute */
    uint8_t second;                /**< second */
    uint8_t format;		/**< data format "0" for 24H */

} ds3231_time_t;

ds3231_time_t *get_timeData(void);

int8_t init_ds3231(void);

int8_t ds3231_set_time(ds3231_time_t *t);

int8_t ds3231_get_time(ds3231_time_t *t);

void ds3231_get_temperature(int16_t *raw, float *s);

void ds3231_aging_offset_convert_to_register(float offset, int8_t *reg);

#endif  // DS3231_H_
