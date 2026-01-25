#include "main.h"
#include <string.h>
#include <math.h>
#include <stdio.h>		//for sprintf
//#include "nv3023.h"
#include "i2c.h"
#include "compass.h"
#include "settings.h"
#include "gpio.h"
#include "lrns.h"
#include "bno055_stm32.h"

#include "lcd_display.h"

struct settings_struct *p_settings;
struct devices_struct **pp_devices_compass;

bno055_calibration_state_t calState;
bno055_calibration_data_t calData;

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
//todo if(!main_flags.display_status) отправлять bno055 в сон
void init_compass(void)
{
	p_settings = get_settings();

	bno055_setup();
	bno055_enableExternalCrystal();
// in Operation Mode Config:
	bno055_writeData(BNO055_AXIS_MAP_CONFIG, 0x21);		//page 25 BST_BNO055_DS000_14.pdf
#ifdef COMPASS_BLUE
	bno055_writeData(BNO055_AXIS_MAP_SIGN, 0x07);		//P6: up side down -90° "blue" board
#else	//COMPASS_PINK
	bno055_writeData(BNO055_AXIS_MAP_SIGN, 0x01);		//P5: up side down +90° "pink" board
#endif
	bno055_setOperationModeNDOF();
//to start calibration immediately
//	if(main_flags.calibrateCompassFlag) calibrate_compass();

	//restore calibration from flash
	p_settings = get_settings();

	calData.offset.gyro.x = p_settings->gyro_offset_x.as_integer;		//buffer + 12, 6
	calData.offset.gyro.y = p_settings->gyro_offset_y.as_integer;		//buffer + 12, 6
	calData.offset.gyro.z = p_settings->gyro_offset_z.as_integer;		//buffer + 12, 6

	calData.offset.accel.x = p_settings->accel_offset_x.as_integer;		//buffer, 6
	calData.offset.accel.y = p_settings->accel_offset_y.as_integer;		//buffer, 6
	calData.offset.accel.z = p_settings->accel_offset_z.as_integer;		//buffer, 6

	calData.offset.mag.x = p_settings->magn_offset_x.as_integer;			//buffer + 6, 6
	calData.offset.mag.y = p_settings->magn_offset_y.as_integer;			//buffer + 6, 6
	calData.offset.mag.z = p_settings->magn_offset_z.as_integer;			//buffer + 6, 6

	calData.radius.accel = p_settings->accel_radius.as_integer;		//buffer + 18, 2
	calData.radius.mag = p_settings->magn_radius.as_integer;			//buffer + 20, 2
//left compass in reset state and turn around in navigation modes
//	bno055_setCalibrationData(calData);
//here to check if calibration values has been saved
	(main_flags.calibrateCompassFlag)? calibrate_compass(): bno055_setCalibrationData(calData);
}

void calibrate_compass(void)
{
	#include "lptim.h"

	struct settings_struct settings_copy;
	settings_copy = *p_settings;

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
	fill_screen(BLACK);
	lptim1_start(16, 11);	//main_flags.brightness
	mute_off();
	while(1)
	{
		calState = bno055_getCalibrationState();
		row = 0;
		if((calState.gyro == 3) && !gyro_ok)
		{
			gyro_ok = 1;
			draw_str_by_rows(0, row*11, "            ", &Font_7x9, BLACK,BLACK);
			draw_str_by_rows(0, (row+1)*11, " Gyroscope OK", &Font_7x9, GREEN,BLACK);
			draw_str_by_rows(78, (row+1)*11, "OK", &Font_7x9, CYANB,BLACK);
			led_w_on();
			HAL_Delay(10);
			led_w_off();
			led_blue_on();
		}else if(!gyro_ok) {
			draw_str_by_rows(0, row*11, " Do not move", &Font_7x9, YELLOW,BLACK);
			draw_str_by_rows(0, (row+1)*11, " Gyroscope --", &Font_7x9, CYAN,BLACK);
		}
		row+=2;	//2
		if((calState.accel == 3) && !accel_ok)
		{
			accel_ok = 1;
			draw_str_by_rows(0, row*11, "                  ", &Font_7x9, BLACK,BLACK);
			draw_str_by_rows(0, (row+1)*11, " Accelerometer OK", &Font_7x9, GREEN,BLACK);
			led_w_on();
			HAL_Delay(10);
			led_w_off();
			led_green_on();
		}else if(!accel_ok){
			draw_str_by_rows(0, row*11, " Place 6 positions", &Font_7x9, YELLOW,BLACK);
			draw_str_by_rows(0, (row+1)*11, " Accelerometer --", &Font_7x9, CYAN,BLACK);
		}
		row+=2;	//4
		if((calState.mag == 3) && !magn_ok)
		{
			magn_ok = 1;
			draw_str_by_rows(0, row*11, "              ", &Font_7x9, BLACK,BLACK);
			draw_str_by_rows(0, (row+1)*11, " Magnetometer OK", &Font_7x9, GREEN,BLACK);
			draw_str_by_rows(98, (row+1)*11, "OK", &Font_7x9, RED,BLACK);
			led_w_on();
			HAL_Delay(10);
			led_w_off();
			led_red_on();
		}else if(!magn_ok) {
			draw_str_by_rows(0, row*11, " Draw figure 8", &Font_7x9, YELLOW,BLACK);
			draw_str_by_rows(0, (row+1)*11, " Magnetometer --", &Font_7x9, CYAN,BLACK);
		}
		row+=3;	//6
		if(!sys_ok && !(GPIOA->IDR & BTN_2_Pin) && (GPIOA->IDR & BTN_3_Pin) && accel_ok && magn_ok && gyro_ok)
		{
			fill_rectgl(0, 70, 128, 160, BLACK);
			sys_ok = 1;
//			sprintf(&string_buffer[row][0], "Calibrated!");
			draw_str_by_rows(5, row*11, "Calibrated!", &Font_11x18, CYAN,BLACK);
			led_w_on();
			HAL_Delay(50);
			led_w_off();

			draw_str_by_rows(0, (row+2)*11+5, "   Press OK to", &Font_7x9, YELLOW,BLACK);
			draw_str_by_rows(0, (row+3)*11+5, " APPLY AND REBOOT ", &Font_7x9, RED,BLACK);

			draw_str_by_rows(0, (row+5)*11, "  Press ESC to", &Font_7x9, YELLOW,BLACK);
			draw_str_by_rows(0, (row+6)*11, "   TO CLARIFY ", &Font_7x9, MAGENTA,BLACK);
		}

		HAL_Delay(300);

		if(!(GPIOA->IDR & BTN_1_Pin) && (GPIOA->IDR & BTN_2_Pin) && (GPIOA->IDR & BTN_3_Pin))
		{
		    fill_screen(BLACK);
			draw_str_by_rows(0, row*11, " Just Restarting", &Font_7x9, YELLOW,BLACK);
		    HAL_Delay(1000);
		    NVIC_SystemReset();
		}
		if (!(GPIOA->IDR & BTN_2_Pin) && sys_ok == 1)	//OK for save
	    {
	    	break;
	    }
		if (!(GPIOA->IDR & BTN_3_Pin) && (GPIOA->IDR & BTN_2_Pin) && accel_ok && magn_ok && gyro_ok)	//ECS for restart && sys_ok == 1
    	{
			bno055_setup();
			bno055_setOperationModeNDOF();
    		goto restart_cal;
    	}

		calData = bno055_getCalibrationData();
		if(!sys_ok)
		{
			row = 7;
			(calState.sys == 3)? sprintf(&string_buffer[row][0]," system calibr OK"):
								 sprintf(&string_buffer[row][0]," system calibr %d", calState.sys);
			draw_str_by_rows(0, (row)*11-3, &string_buffer[row][0], &Font_7x9, CYAN,BLACK);
			row++;
			draw_str_by_rows(0, (row)*11, "off", &Font_7x9, YELLOW,BLACK);
			draw_str_by_rows(28, (row)*11, "GYR  ACC   MAG", &Font_7x9, CYAN,BLACK);
			row++;
			sprintf(&string_buffer[row][0],"X%5d %5d %5d", calData.offset.gyro.x, calData.offset.accel.x, calData.offset.mag.x);		//buffer, 6
			draw_str_by_rows(0, (row)*11, &string_buffer[row][0], &Font_7x9, YELLOW,BLACK);
			row++;
			sprintf(&string_buffer[row][0],"Y%5d %5d %5d", calData.offset.gyro.y, calData.offset.accel.y, calData.offset.mag.y);		//buffer, 6
			draw_str_by_rows(0, (row)*11, &string_buffer[row][0], &Font_7x9, YELLOW,BLACK);
			row++;
			sprintf(&string_buffer[row][0],"Z%5d %5d %5d", calData.offset.gyro.z, calData.offset.accel.z, calData.offset.mag.z);
			draw_str_by_rows(0, (row)*11, &string_buffer[row][0], &Font_7x9, YELLOW,BLACK);
			row++;
			if(accel_ok && magn_ok && gyro_ok) draw_str_by_rows(0, row*11+3, "Press ESC ifZeroes", &Font_7x9, RED,BLACK);
			row++;
			if(accel_ok && magn_ok && gyro_ok) draw_str_by_rows(0, row*11+7, " Press OK if Not", &Font_7x9, RED,BLACK);
	//		row++;
	//		sprintf(&string_buffer[row][0],"Radius     %4d %5d", calData.radius.accel, calData.radius.mag);		//buffer + 18, 2
	//		draw_str_by_rows(0, (row)*11, &string_buffer[row][0], &Font_7x9, MAGENTA,BLACK);
	//		row++;
	//		sprintf(&string_buffer[row][0],"State   %d    %d     %d", calState.gyro, calState.accel, calState.mag);
	//		draw_str_by_rows(0, (row)*11, &string_buffer[row][0], &Font_7x9, MAGENTA,BLACK);
		}
	}

    //save calibration in settings
    settings_copy.gyro_offset_x.as_integer  = calData.offset.gyro.x;		//buffer + 12, 6
    settings_copy.gyro_offset_y.as_integer  = calData.offset.gyro.y;		//buffer + 12, 6
    settings_copy.gyro_offset_z.as_integer  = calData.offset.gyro.z;		//buffer + 12, 6

    settings_copy.accel_offset_x.as_integer = calData.offset.accel.x;		//buffer, 6
    settings_copy.accel_offset_y.as_integer = calData.offset.accel.y;		//buffer, 6
    settings_copy.accel_offset_z.as_integer = calData.offset.accel.z;		//buffer, 6

    settings_copy.magn_offset_x.as_integer  = calData.offset.mag.x;			//buffer + 6, 6
    settings_copy.magn_offset_y.as_integer  = calData.offset.mag.y;			//buffer + 6, 6
    settings_copy.magn_offset_z.as_integer  = calData.offset.mag.z;			//buffer + 6, 6

    settings_copy.accel_radius.as_integer = calData.radius.accel;		//buffer + 18, 2
    settings_copy.magn_radius.as_integer  = calData.radius.mag;			//buffer + 20, 2

    fill_screen(BLACK);
	draw_str_by_rows(0, (row-3)*18, "  Saving...", &Font_11x18, YELLOW,BLACK);
	settings_save(&settings_copy);
    HAL_Delay(1000);
    NVIC_SystemReset();
}

int16_t x = 0;
int16_t y = 0;
double comp_x = 0;
double comp_y = 0;

uint8_t read_north(void)		//case 3 when TIM1 IRQ
{
	pp_devices_compass = get_devices();
	p_settings = get_settings();
	if(main_flags.fix_valid &&
			(pp_devices_compass[p_settings->device_number]->gps_speed > GPS_SPEED_THRS))	//use GPS course when moving
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
