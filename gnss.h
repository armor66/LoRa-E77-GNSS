#ifndef GNSS_HEADER
#define GNSS_HEADER

#include <stdint.h>

#define TREKPOINTS_ON_DISPLAY	(8)

#define TREKPOINTS_TOTAL (1)
//~20m gap
static const float latitude_array[]={0}

static const float longitude_array[]={0}

struct trekpoints_struct
{
//RELATIVE COORDINATES
    uint32_t distance;

    int16_t azimuth_deg_signed;
    double azimuth_rad;
    double azimuth_relative_rad;
};

struct trekpoints_struct **get_trekpoints(void);

//set FLASH, BBR, RAM: CFG-TP-PERIOD_TP1 =1000 000, CFG-TP-PERIOD_LOCK_TP1	=1000 000
static const uint8_t set_persecond_period[] = {0xB5, 0x62, 0x06, 0x8A, 0x14, 0x00, 0x00, 0x07, 0x00, 0x00, 0x02, 0x00,
		0x05, 0x40, 0x40, 0x42, 0x0F, 0x00, 0x03, 0x00, 0x05, 0x40, 0x40, 0x42, 0x0F, 0x00, 0x5C, 0x9F};
//+CFG-RATE-MEAS =1000: B5 62 06 8A 1A 00 00 07 00 00 02 00 05 40 40 42 0F 00 03 00 05 40 40 42 0F 00 01 00 21 30 E8 03 9F 5C
//set FLASH, BBR, RAM: CFG-TP-PERIOD_TP1 =3000 000, CFG-TP-PERIOD_LOCK_TP1	=3000 000
static const uint8_t set_pertriple_period[] = {0xB5, 0x62, 0x06, 0x8A, 0x14, 0x00, 0x00, 0x07, 0x00, 0x00, 0x02, 0x00,
		0x05, 0x40, 0xC0, 0xC6, 0x2D, 0x00, 0x03, 0x00, 0x05, 0x40, 0xC0, 0xC6, 0x2D, 0x00, 0xA0, 0x3F};
//+CFG-RATE-MEAS =3000: B5 62 06 8A 1A 00 00 07 00 00 02 00 05 40 C0 C6 2D 00 03 00 05 40 C0 C6 2D 00 01 00 21 30 B8 0B BB 3C
void serialPrint(const uint8_t *pBuffer, uint8_t size);

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
