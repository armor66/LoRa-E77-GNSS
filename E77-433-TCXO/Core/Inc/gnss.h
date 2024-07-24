#ifndef GNSS_HEADER
#define GNSS_HEADER

#include <stdint.h>

#define TREKPOINTS_ON_DISPLAY	(8)

#define TREKPOINTS_TOTAL (1)
//~20m gap
static const float latitude_array []={0};

static const float longitude_array[]={0};

struct trekpoints_struct
{
//RELATIVE COORDINATES
    uint32_t distance;

    int16_t azimuth_deg_signed;
    double azimuth_rad;
    double azimuth_relative_rad;
};

struct trekpoints_struct **get_trekpoints(void);

typedef enum {
	GPS_BAUDRATE_9600 = 0,
	GPS_BAUDRATE_19200,
    GPS_BAUDRATE_38400,
	GPS_BAUDRATE_57600,
	GPS_BAUDRATE_115200,
	GPS_BAUDRATE_230400,
	BAUDRATE_MAX_IND = GPS_BAUDRATE_230400
} gpsBaudRate_e;

void init_gnss(void);

void find_nearest_trekpoint(void);

void manage_trekpoints(uint8_t range_ind);

#endif /*GNSS_HEADER*/
