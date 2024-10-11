#include "main.h"
#include <string.h>
#include <math.h>
#include <stdio.h>		//for sprintf
#include "ST7735.h"
//#include "i2c.h"
#include "compass.h"
//#include "lsm303.h"
//#include "service.h"
#include "settings.h"
#include "gpio.h"
#include "lrns.h"
//#include "bno055_stm32.h"

struct acc_data *p_acceleration;
struct mag_data *p_magnetic_field;
struct settings_struct *p_settings;
struct devices_struct **pp_devices_compass;

//bno055_calibration_state_t calState;
//bno055_calibration_data_t calData;
int8_t calibrateCompassFlag = 0;
//struct settings_struct settings_copy;
//*p_settings = *get_settings();
//uint8_t settings_copy = *p_settings;
//struct gps_num_struct *p_gps_num;			main.h

int16_t heading_deg = 0;
double heading_rad = 0;

int8_t north_ready = 0; //flag is north value ready to readout

uint8_t read_north(void)		//subghz_phy_app.c: 345,400
{
	pp_devices_compass = get_devices();
	p_settings = get_settings();
	if(pp_devices_compass[p_settings->device_number]->gps_speed > GPS_SPEED_THRS)		//use GPS course when moving
	{
		heading_deg = pp_devices_compass[p_settings->device_number]->gps_heading;
		if(heading_deg > 180) heading_deg = heading_deg - 360;
		heading_rad = heading_deg * 0.0174532925199433;       //deg to rad multiplyer
		north_ready = 3;
		return 1;						//if is moving
	} else
	{
	}
	north_ready--;
	return 0; //return 0 if nor moving not calibrated
}

uint8_t is_north_ready(void)
{
	if(north_ready < 1) north_ready = 0;
	return north_ready;
}

uint8_t reset_north(void)
{
	north_ready = 0; //clear flag until next read_north()
	return north_ready;	//north;
}
