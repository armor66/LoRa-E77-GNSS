#include "main.h"
#include <string.h>
#include <math.h>
#include <stdio.h>		//for sprintf
#include "ST7735.h"
#include "i2c.h"
#include "compass.h"
//#include "lsm303.h"
//#include "service.h"
#include "settings.h"
#include "gpio.h"
#include "lrns.h"
#include "bno055_stm32.h"

struct acc_data *p_acceleration;
struct mag_data *p_magnetic_field;
struct settings_struct *p_settings;
struct devices_struct **pp_devices_compass;

bno055_calibration_state_t calState;
bno055_calibration_data_t calData;
int8_t calibrateCompassFlag = 0;
//struct settings_struct settings_copy;
//*p_settings = *get_settings();
//uint8_t settings_copy = *p_settings;
//struct gps_num_struct *p_gps_num;			main.h

int16_t heading_deg = 0;
double heading_rad = 0;

int8_t north_ready = 0; //flag is north value ready to readout

uint32_t absv(int32_t value)
{
	if (value < 0) {return (-1 * value);}
	else {return value;}
}

int32_t maxv(int32_t x, int32_t y)
{
	if (x > y) {return x;}
	else {return y;}
}

int32_t minv(int32_t x, int32_t y)
{
	if (x < y) {return x;}
	else {return y;}
}

int32_t limit_to(int32_t value, int32_t pos_lim, int32_t neg_lim)
{
	if (value > pos_lim) {return pos_lim;}
	else if (value < neg_lim) {return neg_lim;}
	else {return value;}
}

void init_compass(void)
{
	p_settings = get_settings();
	//restore calibration from flash
	calData.offset.accel.x = p_settings->accel_offset_x.as_integer;		//buffer, 6
	calData.offset.accel.y = p_settings->accel_offset_y.as_integer;		//buffer, 6
	calData.offset.accel.z = p_settings->accel_offset_z.as_integer;		//buffer, 6

//	calData.offset.mag.x = p_settings->magn_offset_x.as_integer;			//buffer + 6, 6
//	calData.offset.mag.y = p_settings->magn_offset_y.as_integer;			//buffer + 6, 6
//	calData.offset.mag.z = p_settings->magn_offset_z.as_integer;			//buffer + 6, 6

	calData.offset.gyro.x = p_settings->gyro_offset_x.as_integer;		//buffer + 12, 6
	calData.offset.gyro.y = p_settings->gyro_offset_y.as_integer;		//buffer + 12, 6
	calData.offset.gyro.z = p_settings->gyro_offset_z.as_integer;		//buffer + 12, 6

	calData.radius.accel = p_settings->accel_radius.as_integer;		//buffer + 18, 2
//	calData.radius.mag = p_settings->magn_radius.as_integer;			//buffer + 20, 2

	bno055_assignI2C(&hi2c1);
	bno055_setup();

	bno055_writeData(BNO055_AXIS_MAP_CONFIG, 0x21);		//-90° page 25 of "BST_BNO055_DS000_14.pdf"
	bno055_writeData(BNO055_AXIS_MAP_SIGN, 0x07);		//up side down
	bno055_setOperationModeNDOF();
	if(calibrateCompassFlag) calibrate_compass();
	else {
//		bno055_setCalibrationData(calData);
	}
}

void calibrate_compass(void)
{
	#include "lptim.h"
//	#include "buttons.h"
//	HAL_LPTIM_PWM_Start(&hlptim1, 16, brightness);
//	disable_buttons_interrupts();
//	EXTI->IMR1 &= ~EXTI_IMR1_IM8;

	struct settings_struct settings_copy;
	p_settings = get_settings();
	settings_copy = *p_settings;

	char Line[24][32];
	int8_t accel_ok;
	int8_t magn_ok;
	int8_t gyro_ok;
	int8_t sys_ok;
	int8_t row;

	restart_cal:

	accel_ok = 0;
	magn_ok = 0;
	gyro_ok = 0;
	sys_ok = 0;

	  led_red_off();
	  led_green_off();
	  led_blue_off();
	ST7735_SetRotation(0);
	fillScreen(BLACK);
	HAL_LPTIM_PWM_Start(&hlptim1, 16, brightness);

	while(1)
	{
		calState = bno055_getCalibrationState();
		row = 0;
		if((calState.accel == 3) && !accel_ok){
			accel_ok = 1;
			sprintf(&Line[row][0],"                  ");
			ST7735_WriteString(0, row*11, &Line[row][0], Font_7x10, BLACK,BLACK);
			sprintf(&Line[row+1][0]," Accelerometer OK");
			ST7735_WriteString(0, (row+1)*11, &Line[row+1][0], Font_7x10, GREEN,BLACK);
			sprintf(&Line[row+1][15],"OK");
			ST7735_WriteString(105, (row+1)*11, &Line[row+1][15], Font_7x10, BLUE,BLACK);
			led_w_on();
			HAL_Delay(10);
			led_w_off();
			led_blue_on();
		}else if(!accel_ok){
			sprintf(&Line[row][0]," Place 6 positions");
			ST7735_WriteString(0, row*11, &Line[row][0], Font_7x10, YELLOW,BLACK);
			sprintf(&Line[row+1][0]," Accelerometer --");
			ST7735_WriteString(0, (row+1)*11, &Line[row+1][0], Font_7x10, CYAN,BLACK);
		}
		row+=2;	//2
		if((calState.gyro == 3) && !magn_ok) {
			magn_ok = 1;
			sprintf(&Line[row][0],"            ");
			ST7735_WriteString(0, row*11, &Line[row][0], Font_7x10, BLACK,BLACK);
			sprintf(&Line[row+1][0]," Gyroscope OK");
			ST7735_WriteString(0, (row+1)*11, &Line[row+1][0], Font_7x10, GREEN,BLACK);
			led_w_on();
			HAL_Delay(10);
			led_w_off();
			led_green_on();
		}else if(!magn_ok) {
			sprintf(&Line[row][0]," Do not move");
			ST7735_WriteString(0, row*11, &Line[row][0], Font_7x10, YELLOW,BLACK);
			sprintf(&Line[row+1][0]," Gyroscope --");
			ST7735_WriteString(0, (row+1)*11, &Line[row+1][0], Font_7x10, CYAN,BLACK);
		}
		row+=2;	//4
		if((calState.mag == 3) && !gyro_ok){
			gyro_ok = 1;
			sprintf(&Line[row][0],"              ");
			ST7735_WriteString(0, row*11, &Line[row][0], Font_7x10, BLACK,BLACK);
			sprintf(&Line[row+1][0]," Magnetometer OK");
			ST7735_WriteString(0, (row+1)*11, &Line[row+1][0], Font_7x10, GREEN,BLACK);
			sprintf(&Line[row+1][14],"OK");
			ST7735_WriteString(98, (row+1)*11, &Line[row+1][14], Font_7x10, RED,BLACK);
			led_w_on();
			HAL_Delay(10);
			led_w_off();
			led_red_on();
		}else if(!gyro_ok) {
			sprintf(&Line[row][0]," Draw figure 8");
			ST7735_WriteString(0, row*11, &Line[row][0], Font_7x10, YELLOW,BLACK);
			sprintf(&Line[row+1][0]," Magnetometer --");
			ST7735_WriteString(0, (row+1)*11, &Line[row+1][0], Font_7x10, CYAN,BLACK);
		}
		row+=3;	//6
		if((calState.sys == 3) && !sys_ok && accel_ok && magn_ok && gyro_ok){
			sys_ok = 1;
			sprintf(&Line[row][0], "Calibrated!");
			ST7735_WriteString(5, row*11, &Line[row][0], Font_11x18, CYAN,BLACK);
			led_w_on();
			HAL_Delay(50);
			led_w_off();

			sprintf(&Line[row+1][0], "   Press OK to");
			ST7735_WriteString(0, (row+2)*11+5, &Line[row+1][0], Font_7x10, YELLOW,BLACK);
			sprintf(&Line[row+2][0], " APPLY AND REBOOT ");
			ST7735_WriteString(0, (row+3)*11+5, &Line[row+2][0], Font_7x10, RED,BLACK);

			sprintf(&Line[row+3][0], "  Press ESC to");
			ST7735_WriteString(0, (row+5)*11, &Line[row+3][0], Font_7x10, YELLOW,BLACK);
			sprintf(&Line[row+4][0], "   TO CLARIFY ");
			ST7735_WriteString(0, (row+6)*11, &Line[row+4][0], Font_7x10, MAGENTA,BLACK);
		}

		if (!(GPIOA->IDR & BTN_2_Pin))	//OK for save
	    {
	    	break;
	    }
		if (!(GPIOA->IDR & BTN_3_Pin))	//ECS for restart
    	{
    		goto restart_cal;
    	}
		HAL_Delay(200);
	}
	calData = bno055_getCalibrationData();
    //save calibration in settings
    settings_copy.accel_offset_x.as_integer = calData.offset.accel.x;		//buffer, 6
    settings_copy.accel_offset_y.as_integer = calData.offset.accel.y;		//buffer, 6
    settings_copy.accel_offset_z.as_integer = calData.offset.accel.z;		//buffer, 6

    settings_copy.magn_offset_x.as_integer  = calData.offset.mag.x;			//buffer + 6, 6
    settings_copy.magn_offset_y.as_integer  = calData.offset.mag.y;			//buffer + 6, 6
    settings_copy.magn_offset_z.as_integer  = calData.offset.mag.z;			//buffer + 6, 6

    settings_copy.gyro_offset_x.as_integer  = calData.offset.gyro.x;		//buffer + 12, 6
    settings_copy.gyro_offset_y.as_integer  = calData.offset.gyro.y;		//buffer + 12, 6
    settings_copy.gyro_offset_z.as_integer  = calData.offset.gyro.z;		//buffer + 12, 6

    settings_copy.accel_radius.as_integer = calData.radius.accel;		//buffer + 18, 2
    settings_copy.magn_radius.as_integer  = calData.radius.mag;			//buffer + 20, 2

    fillScreen(BLACK);
	sprintf(&Line[row-3][1], "  Saving...");
	ST7735_WriteString(0, (row-3)*18, &Line[row-3][1], Font_11x18, YELLOW,BLACK);

	settings_save(&settings_copy);
    HAL_Delay(1000);
    NVIC_SystemReset();

}

int16_t x = 0;
int16_t y = 0;
double comp_x = 0;
double comp_y = 0;

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
		calState = bno055_getCalibrationState();
		if(calState.mag == 3)				//compass is calibrated
		{
			bno055_vector_t v = bno055_getVectorEuler();
			heading_deg = (int16_t)v.x;
			if(heading_deg > 180) heading_deg = heading_deg - 360;
			heading_rad = heading_deg * 0.0174532925199433;       //deg to rad multiplyer
			north_ready = 3;
			return 1; 					//if not moving but calibrated
		}
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
