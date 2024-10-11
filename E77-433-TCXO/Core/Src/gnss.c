#include <stdio.h>			//for 'sprintf'
#include <string.h>
#include <math.h>
#include "gnss.h"
#include "settings.h"
#include "lrns.h"
#include "compass.h"
#include "ST7735.h"
#include "gpio.h"
#include "usart.h"
#include "lptim.h"

const double rad_2_deg = 57.29577951308232;        //rad to deg multiplyer
const double deg_2_rad = 0.00000000174532925199433;       //deg to rad multiplyer

double host_lat_lon_rad[2];

//uint32_t distance;
uint32_t min_distance;
//int16_t azimuth_deg_signed;
//fldoublezimuth_rad;
//double azimuth_relative_rad;

volatile int16_t nearest_trekpoint_idx;

struct settings_struct *p_settings_gnss;
struct devices_struct **pp_devices_gnss;

struct trekpoints_struct trekpoints[TREKPOINTS_TOTAL];        //structures array for
struct trekpoints_struct *p_trekpoints[TREKPOINTS_TOTAL];		//structure pointers array

struct trekpoints_struct **get_trekpoints(void)
{
	for (uint8_t idx = 0; idx < TREKPOINTS_TOTAL; idx++)	//TREKPOINTS_TOTAL < 256
	{
		p_trekpoints[idx] = &trekpoints[idx];
	}

	return &p_trekpoints[0];
}
struct trekpoints_struct **pp_trekpoints;

void configure_gps(void);

uint8_t UBLOX_verify_checksum(volatile uint8_t *buffer, uint8_t len)
{
	uint8_t CK_A_real = buffer[len-2];
	uint8_t CK_B_real = buffer[len-1];
	uint8_t CK_A_comp = 0;
	uint8_t CK_B_comp = 0;

	for(uint8_t i = 2; i < len-2; i++)
	{
		CK_A_comp = CK_A_comp + buffer[i];
		CK_B_comp = CK_A_comp + CK_B_comp;
	}
	CK_A_comp = CK_A_comp & 0xFF;
	CK_B_comp = CK_B_comp & 0xFF;

	if(CK_A_real == CK_A_comp && CK_B_real == CK_B_comp) return 1;
	else return 0;
}

#define UBX_HEADER_1		0xB5
#define UBX_HEADER_2		0x62
#define UBX_CLASS_CFG	0x06
#define UBX_CFG_RST		0x04

void send_ubx(uint8_t ubx_class, uint8_t ubx_id, uint8_t payload[], uint8_t len)
{
	uint8_t ubx_message[255 + 8] = {0};
	uint8_t CK_A = 0;
	uint8_t CK_B = 0;

	ubx_message[0] = UBX_HEADER_1;
	ubx_message[1] = UBX_HEADER_2;
	ubx_message[2] = ubx_class;
	ubx_message[3] = ubx_id;
	ubx_message[4] = len;		//length of payload field LSB (expected len < 256)
	ubx_message[5] = 0;			//length of payload field MSB
	ubx_message[6] = 0;			//payload starts from here

	for (uint8_t i = 0; i < len; i++)
	{
		ubx_message[6 + i] = payload[i];	//copy payload
	}

	for (uint8_t m = 2; m < (len + 6); m++)
	{
		CK_A = CK_A + ubx_message[m];		//calc checksums
		CK_B = CK_B + CK_A;
	}

	ubx_message[6 + len] = CK_A;
	ubx_message[6 + len + 1] = CK_B;

//	for (uint8_t n = 0; n < (len + 8); n++)		//8 bytes header & checksum
//	{
//		uart3_tx_byte(ubx_message[n]);			//transmit
//	}
	serialPrint(ubx_message, (len + 8));
}

uint8_t nav_pvt_ram_flag = 0;
uint8_t out_ubx_ram_flag = 0;
uint8_t out_nmea_ram_flag = 0;
uint8_t new_options_flag = 0;

const uint8_t ubx_mon_ver[] = {0xB5, 0x62, 0x0a, 0x04, 0x00, 0x00, 0x0e, 0x34};
//set 38400:
//uint8_t set_baudrate[] = {0xB5, 0x62, 0x06, 0x8A, 0x0C, 0x00, 0x00, 0x07, 0x00, 0x00, 0x01, 0x00, 0x52, 0x40, 0x00, 0x96, 0x00, 0x00, 0xCC, 0x61};
//set 57600
const uint8_t set_baudrate[] = {0xB5, 0x62, 0x06, 0x8A, 0x0C, 0x00, 0x00, 0x07, 0x00, 0x00, 0x01, 0x00, 0x52, 0x40, 0x00, 0xE1, 0x00, 0x00, 0x17, 0x42};
//set 38400
const uint8_t revert_baudrate[] = {0xB5, 0x62, 0x06, 0x8A, 0x0C, 0x00, 0x00, 0x07, 0x00, 0x00, 0x01, 0x00, 0x52, 0x40, 0x00, 0x96, 0x00, 0x00, 0xCC, 0x61};
//save to BBR, RAM
const uint8_t save_to_bbr_flash[] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1D, 0xAB};
//save to FLASH, BBR, RAM
const uint8_t to_ram_bbr_flash[]  = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x21, 0xAF};
//save to BBR, RAM
//const uint8_t revert_to_default[]={0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x03, 0x1B, 0x9A};
//save to FLASH, BBR, RAM
const uint8_t revert_to_default[] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x07, 0x1F, 0x9E};
//set to FLASH, BBR, RAM: NAV-PVT (uart1), pedestrian (3), MEAS (1000), LEN_TP1 (1000), UBX (true), NMEA (false)
//const uint8_t set_crucial_opts[] = {0xB5, 0x62, 0x06, 0x8A, 0x26, 0x00, 0x00, 0x07, 0x00, 0x00, 0x07, 0x00, 0x91, 0x20, 0x01, 0x21, 0x00, 0x11, 0x20, 0x03, 0x01,
//			0x00, 0x21, 0x30, 0xE8, 0x03, 0x04, 0x00, 0x05, 0x40, 0xE8, 0x03, 0x00, 0x00, 0x01, 0x00, 0x74, 0x10, 0x01, 0x02, 0x00, 0x74, 0x10, 0x00, 0x48, 0x34};
//set to FLASH, BBR, RAM: NAV-PVT (uart1), pedestrian (3), MEAS (1000), LEN_TP1 (0), LEN_LOCK_TP1 (100.000), UBX-InOut (true), NMEA-InOut (false)
const uint8_t set_crucial_opts[] = {0xB5, 0x62, 0x06, 0x8A, 0x38, 0x00, 0x00, 0x07, 0x00, 0x00, 0x01, 0x00, 0x73, 0x10, 0x01, 0x02, 0x00, 0x73, 0x10, 0x00, 0x01,
			0x00, 0x74, 0x10, 0x01, 0x02, 0x00, 0x74, 0x10, 0x00, 0x07, 0x00, 0x91, 0x20, 0x01, 0x21, 0x00, 0x11, 0x20, 0x03, 0x01, 0x00, 0x21, 0x30, 0xE8, 0x03,
			0x04, 0x00, 0x05, 0x40, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x40, 0xA0, 0x86, 0x01, 0x00, 0xEA, 0xEC};
//request from RAM:
//CFG-MSGOUT-UBX_NAVPVT_UART1
const uint8_t req_nav_pvt_ram[] = {0xB5, 0x62, 0x06, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x91, 0x20, 0x51, 0xEF};
//CFG-UART1OUTPROT-UBX
const uint8_t req_out_ubx_ram[] = {0xB5, 0x62, 0x06, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x74, 0x10, 0x1E, 0x8D};
//CFG-UART1OUTPROT-NMEA
const uint8_t req_out_nmea_ram[] = {0xB5, 0x62, 0x06, 0x8B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x74, 0x10, 0x1F, 0x91};
//Power mode:
const uint8_t set_agressive_pm[] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0x6F};
const uint8_t set_balanced_pm[] =  {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x95, 0x61};
//set to RAM only:  CFG-RATE-MEAS=3000, CFG-TP-PERIOD_TP1=3.000.000, CFG-TP-PERIOD_LOCK_TP1=3.000.000
const uint8_t set_three_seconds[] = {0xB5, 0x62, 0x06, 0x8A, 0x1A, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x21, 0x30, 0xB8, 0x0B, 0x02, 0x00, 0x05, 0x40, 0xC0, 0xC6,
		0x2D, 0x00, 0x03, 0x00, 0x05, 0x40, 0xC0, 0xC6, 0x2D, 0x00, 0xB5, 0x38};
//perhaps not need: CFG-RATE-MEAS=1000, CFG-TP-PERIOD_TP1=1.000.000, CFG-TP-PERIOD_LOCK_TP1=1.000.000
const uint8_t set_one_second[]    = {0xB5, 0x62, 0x06, 0x8A, 0x1A, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x21, 0x30, 0xE8, 0x03, 0x02, 0x00, 0x05, 0x40, 0x40, 0x42,
		0x0F, 0x00, 0x03, 0x00, 0x05, 0x40, 0x40, 0x42, 0x0F, 0x00, 0x99, 0x70};

//This performs forced HW restart + coldstart	B5 62 06 04 04 00 FF B9 00 00 C6 8B
uint8_t cfg_rst_cold_restart[] = {0xFF, 0xB9, 0x00, 0x00};

gpsBaudRate_e baudRateInd = 0;
uint16_t baudBRR[BAUDRATE_MAX_IND +1] = {0x1388, 0x9C4, 0x4E2, 0x341, 0x1A0, 0xD0};

void serialPrint(const uint8_t *pBuffer, uint8_t size) {
	for(uint8_t i = 0; i < size; i++) {
//    while (*pBuffer) {
        USART2->TDR = (*pBuffer++ & (uint16_t)0x01FF);
        while(!(USART2->ISR & USART_ISR_TXE_TXFNF));
    }
}

void restart_uart(int8_t baud_rate_index)
{
	uartIdx = 0;
  	USART2->RDR;
   	USART2->CR1 = 0x00000000;
   	USART2->BRR = baudBRR[baud_rate_index];
   	USART2->CR1 = USART_CR1_UE;
   	USART2->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE_RXFNEIE;
}

void init_gnss(void)
{
//	HAL_UART_Transmit(&huart2, ubx_nav_pvt, sizeof(ubx_nav_pvt), 1000);
	p_settings_gnss = get_settings();
	pp_devices_gnss = get_devices();
	pp_trekpoints = get_trekpoints();
#ifndef BEACON
	if(TREKPOINTS_TOTAL > 1)
	{
		pp_devices_gnss[p_settings_gnss->device_number]->flwtrek_flag = 1;
	}
#endif

	if(GPSconfigureFlag)
	{
		configure_gps();
	}
	else
	{
		GPScheckFlag = 1;					//check valid gnss settings: if "nav_pvt_ram_flag" has set
		restart_uart(GPS_BAUDRATE_57600);
		HAL_Delay(100);
		serialPrint(req_nav_pvt_ram, sizeof(req_nav_pvt_ram));
		HAL_Delay(100);
	}
	if(!nav_pvt_ram_flag)
	{
		led_blue_on();
		send_ubx(UBX_CLASS_CFG, UBX_CFG_RST, &cfg_rst_cold_restart[0], sizeof(cfg_rst_cold_restart));
		HAL_Delay(100);
		GPScheckFlag = 1;					//check valid gnss settings: if "nav_pvt_ram_flag" has set
		restart_uart(GPS_BAUDRATE_57600);
		HAL_Delay(100);
		serialPrint(req_nav_pvt_ram, sizeof(req_nav_pvt_ram));
		HAL_Delay(100);
	}
}

void configure_gps(void)
{
volatile int8_t row;
uint16_t baudRate[BAUDRATE_MAX_IND +2] = {96, 192, 384, 576, 1152, 2304, 0};
//led_red_on();
//led_green_on();
led_blue_off();			//just to off after start
ST7735_SetRotation(0);

HAL_LPTIM_PWM_Start(&hlptim1, 16, brightness);

restart_configuration:

	if(new_options_flag)	//re-init gnss module:
	{
		send_ubx(UBX_CLASS_CFG, UBX_CFG_RST, &cfg_rst_cold_restart[0], sizeof(cfg_rst_cold_restart));
		HAL_Delay(500);
	}

	ubx_hwVersion = 0;
	fillScreen(BLACK);

	for(baudRateInd = 0; baudRateInd < (5 + 1); baudRateInd++)
    {
		restart_uart(baudRateInd);
       	if(HAL_UART_GetState(&huart2) != HAL_UART_STATE_BUSY_RX)	//if gps module does not transmit
       	{
       		serialPrint(ubx_mon_ver, sizeof(ubx_mon_ver));
       		HAL_Delay(50);
       	}
        if(ubx_hwVersion) break;
    }
    //restart IRQ for either serialPrint
	restart_uart(baudRateInd);
//    while(HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_RX)	//if gps module does not transmit
    serialPrint(req_nav_pvt_ram, sizeof(req_nav_pvt_ram));
    HAL_Delay(50);

	restart_uart(baudRateInd);
//    while(HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_RX)	//if gps module does not transmit
    serialPrint(req_out_ubx_ram, sizeof(req_out_ubx_ram));
    HAL_Delay(50);

	restart_uart(baudRateInd);
//    while(HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_RX)	//if gps module does not transmit
    serialPrint(req_out_nmea_ram, sizeof(req_out_nmea_ram));
    HAL_Delay(50);
	while(1)
	{
		if(baudRateInd == 3)
		{
			led_green_on();
			led_red_off();
		}else
		{
			led_red_on();
			led_green_off();
		}
		(nav_pvt_ram_flag && out_ubx_ram_flag && !out_nmea_ram_flag)? led_blue_on(): led_blue_off();
//		sprintf(&Line[0][0], " GPS-");
//		ST7735_WriteString(0, 0, &Line[0][0], Font_11x18, CYAN,BLACK);
//		sprintf(&Line[1][4], "SETTINGS ");
//		ST7735_WriteString(54, 6, &Line[1][4], Font_7x10, CYAN,BLACK);
		row	= 0;

		if(baudRate[baudRateInd])
		{
			sprintf(&Line[row][0], "GNSS version M%d", ubx_hwVersion);
//			sprintf(&Line[row+1][0], "    Current");
			sprintf(&Line[row+1][0], "Baud rate %4d00", baudRate[baudRateInd]);
			if(baudRateInd != GPS_BAUDRATE_57600) ST7735_WriteString(7, (row+1)*14+5, &Line[row+1][0], Font_7x10, RED,BLACK);
		}else
		{
			sprintf(&Line[row+0][0], "  GNSS module");
			sprintf(&Line[row+1][0], "   NOT FOUND");
		}
		for (row; row < 2; row++) {
			ST7735_WriteString(7, row*14+5, &Line[row][0], Font_7x10, YLWGRN,BLACK);
		}
		row	= 2;

		char *boolean[] = {"false", "true"};

		sprintf(&Line[row][0],   "Output NAVPVT%5s", boolean[nav_pvt_ram_flag]);
		if(!nav_pvt_ram_flag) ST7735_WriteString(0, row*14+5, &Line[row][0], Font_7x10, RED,BLACK);

		sprintf(&Line[row+1][0], "Output UBLOX %5s", boolean[out_ubx_ram_flag]); //", baudRate[baudRateInd]);
		if(!out_ubx_ram_flag) ST7735_WriteString(0, (row+1)*14+5, &Line[row+1][0], Font_7x10, RED,BLACK);

		sprintf(&Line[row+2][0], "Output NMEA  %5s", boolean[out_nmea_ram_flag]);
		if(out_nmea_ram_flag) ST7735_WriteString(0, (row+2)*14+5, &Line[row+2][0], Font_7x10, RED,BLACK);

		for (row; row < 5; row++) {
			ST7735_WriteString(0, row*14+5, &Line[row][0], Font_7x10, GREENYELLOW,BLACK);
		}

		row	= 5;	//+=3;
		if(1)
		{
			sprintf(&Line[row][0], "   Press PWR to  ");
			ST7735_WriteString(0, (row)*14+7, &Line[row][0], Font_7x10, YELLOW,BLACK);
			sprintf(&Line[row+=1][0], "  RESTART DEVICE ");	//" REVERT DEFAULTS "
			ST7735_WriteString(0, (row)*14+5, &Line[row][0], Font_7x10, ORANGE,BLACK);

			sprintf(&Line[row+=1][0], "   Press OK to   ");
			ST7735_WriteString(0, (row)*14+7, &Line[row][0], Font_7x10, YELLOW,BLACK);
			sprintf(&Line[row+=1][0], "  SET NEW DATA   ");
			ST7735_WriteString(0, (row)*14+5, &Line[row][0], Font_7x10, ORANGE,BLACK);

			sprintf(&Line[row+=1][0], "   Press ESC to  ");
			ST7735_WriteString(0, (row)*14+7, &Line[row][0], Font_7x10, YELLOW,BLACK);
//			sprintf(&Line[row+=1][0], "     RE-CHECK    ");
			sprintf(&Line[row+=1][0], "  SET BAUD RATE   ");	//" REVERT DEFAULTS "
			ST7735_WriteString(0, (row)*14+5, &Line[row][0], Font_7x10, ORANGE,BLACK);
		}

		if((new_options_flag != 1) && !(GPIOA->IDR & BTN_2_Pin) && !(GPIOA->IDR & BTN_3_Pin))
		{
			new_options_flag = 1;
			serialPrint(revert_to_default, sizeof(revert_to_default));
			HAL_Delay(100);
			serialPrint(revert_baudrate, sizeof(revert_baudrate));
			HAL_Delay(100);
    		goto restart_configuration;
		}
		if ((new_options_flag != 2) && !(GPIOA->IDR & BTN_2_Pin) && (GPIOA->IDR & BTN_3_Pin))	//OK to set values
	    {
			new_options_flag = 2;
//			while(HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_RX)	//if gps module does not transmit
			serialPrint(set_crucial_opts, sizeof(set_crucial_opts));
			HAL_Delay(100);
			serialPrint(set_agressive_pm, sizeof(set_agressive_pm));
			HAL_Delay(100);
			serialPrint(to_ram_bbr_flash, sizeof(to_ram_bbr_flash));
			HAL_Delay(100);
			goto restart_configuration;
	    }
		if ((new_options_flag != 3) && !(GPIOA->IDR & BTN_3_Pin) && (GPIOA->IDR & BTN_2_Pin) && (baudRateInd != GPS_BAUDRATE_57600))
    	{
			new_options_flag = 3;
			serialPrint(set_baudrate, sizeof(set_baudrate));
			HAL_Delay(100);
    		goto restart_configuration;
    	}
		if(!(GPIOA->IDR & BTN_1_Pin))	//PWR for restart
		{
			//re-init gnss module:
			send_ubx(UBX_CLASS_CFG, UBX_CFG_RST, &cfg_rst_cold_restart[0], sizeof(cfg_rst_cold_restart));
			HAL_Delay(500);
			NVIC_SystemReset();
//			goto restart_configuration;
		}
		HAL_Delay(200);
//		led_blue_on();
	}
}

void calc_trekpoint_position(uint16_t idx, double *lat_lon)
{
	//my position
//	double Latitude0 = lat_lon[0];			//((double)pp_devices_gnss[p_settings_gnss->device_number]->latitude.as_integer) * deg_2_rad;		//7 digit
//	double Longitude0 = lat_lon[1];	//((double)pp_devices_gnss[p_settings_gnss->device_number]->longitude.as_integer) * deg_2_rad;	//7 digit

	//position of the trek point to calculate relative position
//	double Latitude1 = ((double)latitude_array[idx]);	// * deg_2_rad;
//	double Longitude1 = ((double)longitude_array[idx]);	// * deg_2_rad;
	//distance in meters to a device
							 //(uint32_t)(6371008 * sqrt(pow((Latitude1 		  - Latitude0) , 2) + pow((cos(Latitude0) *  (Longitude1		   - Longitude0)), 2)))
	trekpoints[idx].distance = (uint32_t)(6371008 * sqrt(pow((latitude_array[idx] - lat_lon[0]), 2) + pow((cos(lat_lon[0]) * (longitude_array[idx] - lat_lon[1])), 2)));
	if(trekpoints[idx].distance)	//if distance != 0
	{			 //cos(Latitude1) 			* sin(Longitude1 		   - Longitude0)
		double X = cos(latitude_array[idx]) * sin(longitude_array[idx] - lat_lon[1]);
				 //cos(Latitude0) *  sin(Latitude1) 		  - sin(Latitude0) *  cos(Latitude1)		   * cos(Longitude1 		  - Longitude0)
		double Y = cos(lat_lon[0]) * sin(latitude_array[idx]) - sin(lat_lon[0]) * cos(latitude_array[idx]) * cos(longitude_array[idx] - lat_lon[1]);
		trekpoints[idx].azimuth_rad = atan2(X,Y);
//int16_t azimuth_deg_signed;       //heading to a device, degrees
		trekpoints[idx].azimuth_deg_signed = (int16_t)(trekpoints[idx].azimuth_rad * rad_2_deg);		//convert to degree

	    if(longitude_array[idx] < lat_lon[1]) {trekpoints[idx].azimuth_rad += 2*M_PI;}
	    trekpoints[idx].azimuth_relative_rad = trekpoints[idx].azimuth_rad - heading_rad;
	}else		//if distance == 0)
	{
	     trekpoints[idx].azimuth_rad = 0;
	     trekpoints[idx].azimuth_deg_signed = 0;
	}
}

void find_nearest_trekpoint(void)		//in main menu: case M_MAIN_I_NAVIGATION
{
	uint32_t distance;
	min_distance = 0xFFFFFFFF;
	//my position
	double Latitude0 = ((double)pp_devices_gnss[p_settings_gnss->device_number]->latitude.as_integer) * deg_2_rad;		//7 digit
	double Longitude0 = ((double)pp_devices_gnss[p_settings_gnss->device_number]->longitude.as_integer) * deg_2_rad;		//7 digit

	for(uint16_t i = 0; i < TREKPOINTS_TOTAL; i++)	//sizeof(latitude_array)
	{
		//position of the trek point to calculate relative position
//		double Latitude1 = ((double)latitude_array[i]);		// * deg_2_rad;
//		double Longitude1 = ((double)longitude_array[i]);	// * deg_2_rad;
		//distance in meters to trek point
//		distance = (uint32_t)(6371008 * sqrt(pow((Latitude1 		- Latitude0), 2) + pow((cos(Latitude0) * (Longitude1 		 - Longitude0)), 2)));
		distance = (uint32_t)(6371008 * sqrt(pow((latitude_array[i] - Latitude0), 2) + pow((cos(Latitude0) * (longitude_array[i] - Longitude0)), 2)));
		if(distance < min_distance)
		{
			min_distance = distance;
			nearest_trekpoint_idx = i;
		}
	}

	find_nearest_trekpoint_flag = 0;
	shortBeeps(1);								//find_nearest_trekpoint is complete
	__enable_irq();
}

int8_t is_point_ahead(uint16_t idx)
{
	int16_t azimuth_relative_deg;

//	calc_trekpoint_position(idx);
	azimuth_relative_deg = pp_trekpoints[idx]->azimuth_deg_signed - heading_deg;
	if(azimuth_relative_deg > 180) azimuth_relative_deg -= 360;
	if(azimuth_relative_deg < -180) azimuth_relative_deg += 360;
	//get absolute value:
	if((azimuth_relative_deg > -90) && (azimuth_relative_deg < 90))
	{
		return 1;
	}else return 0;
}

int16_t trekpoint_scaled_dist;
int16_t trekpoint_distance_old[10];
uint16_t trekpoint_range = 30;
uint8_t trekpoint_range_scale[] = {1, 2, 4, 8, 16, 32, 64};

double trekpoint_azimuth_relative_rad_old[TREKPOINTS_ON_DISPLAY];

void draw_trekpoints(uint16_t idx, uint8_t trekpoint_range_ind)	//, uint16_t color)
{
	trekpoint_scaled_dist = (uint16_t)(pp_trekpoints[idx]->distance)*2  / trekpoint_range_scale[trekpoint_range_ind];

	erasePosition(63, 97, trekpoint_distance_old[idx], trekpoint_azimuth_relative_rad_old[idx], 3);
	if(trekpoint_scaled_dist > trekpoint_range * 2)
	{
		trekpoint_scaled_dist = trekpoint_range * 2;
		drawTrace(63, 97, trekpoint_scaled_dist, pp_trekpoints[idx]->azimuth_relative_rad, 1, RED);
		drawTrace(63, 97, trekpoint_scaled_dist, pp_trekpoints[idx]->azimuth_relative_rad, 2, RED);
		drawTrace(63, 97, trekpoint_scaled_dist, pp_trekpoints[idx]->azimuth_relative_rad, 3, RED);
	}else drawTrace(63, 97, trekpoint_scaled_dist, pp_trekpoints[idx]->azimuth_relative_rad, 3, CYAN);
	trekpoint_distance_old[idx] = trekpoint_scaled_dist;
	trekpoint_azimuth_relative_rad_old[idx] = pp_trekpoints[idx]->azimuth_relative_rad;
}

volatile double trek_direction_old;

void manage_trekpoints(uint8_t range_ind)	//	if(pp_trekpoints[nearest_trekpoint_idx]->distance < 2000)
{
	host_lat_lon_rad[0] = ((double)pp_devices_gnss[p_settings_gnss->device_number]->latitude.as_integer) * deg_2_rad;
	host_lat_lon_rad[1] = ((double)pp_devices_gnss[p_settings_gnss->device_number]->longitude.as_integer) * deg_2_rad;

	calc_trekpoint_position(nearest_trekpoint_idx, host_lat_lon_rad);

	if(trek_direction_old)		//erase line and distance
	{
		drawDirection(63, 97, 60, trek_direction_old, BLACK);
		ST7735_WriteString(31, 7*18, &Line[7][3], Font_11x18, BLACK,BLACK);
	}

	if(pp_trekpoints[nearest_trekpoint_idx]->distance < 1900)
	{
		trek_direction_old = 0;
		int8_t index_increased;
		int16_t trekpoint_idx;
		int16_t idx;

		(nearest_trekpoint_idx < TREKPOINTS_TOTAL)? (trekpoint_idx = nearest_trekpoint_idx + 1): (trekpoint_idx = TREKPOINTS_TOTAL);
		calc_trekpoint_position(trekpoint_idx, host_lat_lon_rad);

		if(is_point_ahead(trekpoint_idx))				//варианты 1 и 2
		{
			index_increased = 1;
			if(is_point_ahead(nearest_trekpoint_idx))	//вариант 2
			{	//draw line to nearest_trekpoint_idx
				trekpoint_idx = nearest_trekpoint_idx;
			}else										//вариант 1
			{	//draw line to trekpoint_idx
				nearest_trekpoint_idx = trekpoint_idx;	//nearest_trekpoint_idx++
			}
			//увеличивать индекс
			for(idx = trekpoint_idx; idx < (trekpoint_idx + TREKPOINTS_ON_DISPLAY); idx++)
			{
				if(idx <= TREKPOINTS_TOTAL)
				{
					if(idx > trekpoint_idx) calc_trekpoint_position(idx, host_lat_lon_rad);			//first already calculated
					if(pp_trekpoints[idx]->distance < 1900) draw_trekpoints(idx, range_ind);	//, CYAN);	//draw '+'
				} else idx = (trekpoint_idx + TREKPOINTS_ON_DISPLAY);				//exit cycle 'for'
			}
		}else											//варианты 3 и 4
		{
			index_increased = 0;
			if(is_point_ahead(nearest_trekpoint_idx))	//вариант 4
			{	//draw line to nearest_trekpoint_idx
				trekpoint_idx = nearest_trekpoint_idx;
			}else										//вариант 3
			{	//draw line to trekpoint_idx
				(nearest_trekpoint_idx > 0)? (trekpoint_idx = nearest_trekpoint_idx - 1): (trekpoint_idx = 0);
				calc_trekpoint_position(trekpoint_idx, host_lat_lon_rad);
				if(is_point_ahead(trekpoint_idx))
				{
					nearest_trekpoint_idx = trekpoint_idx;
				}
			}
			//уменьшать индекс
			for(idx = trekpoint_idx; idx > (trekpoint_idx - TREKPOINTS_ON_DISPLAY); idx--)
			{
				if(idx >= 0)		//idx should not be negative
				{
					if(idx < trekpoint_idx) calc_trekpoint_position(idx, host_lat_lon_rad);			//first already calculated
					if(pp_trekpoints[idx]->distance < 1900) draw_trekpoints(idx, range_ind);	//, BLUE);	//draw '-'
				}else idx = (trekpoint_idx - TREKPOINTS_ON_DISPLAY);				//exit cycle 'for'
			}
		}
		sprintf(&Line[6][4], "%3dm ", (uint16_t)pp_trekpoints[nearest_trekpoint_idx]->distance);
		ST7735_WriteString(48, 6*19, &Line[6][4], Font_11x18, WHITE,BLACK);

		if(pp_trekpoints[nearest_trekpoint_idx]->distance > (8 * trekpoint_range_scale[range_ind]))
		{
			drawDirection(63, 97, 60, pp_trekpoints[nearest_trekpoint_idx]->azimuth_relative_rad, WHITE);
			trek_direction_old = pp_trekpoints[nearest_trekpoint_idx]->azimuth_relative_rad;
		}else
		{
		if(index_increased)
			{
				drawDirection(63, 97, 60, pp_trekpoints[nearest_trekpoint_idx + 1]->azimuth_relative_rad, WHITE);
				trek_direction_old = pp_trekpoints[nearest_trekpoint_idx + 1]->azimuth_relative_rad;
			}else
			{
				drawDirection(63, 97, 60, pp_trekpoints[nearest_trekpoint_idx - 1]->azimuth_relative_rad, WHITE);
				trek_direction_old = pp_trekpoints[nearest_trekpoint_idx - 1]->azimuth_relative_rad;
			}
		}

	}else	//or if(pp_trekpoints[nearest_trekpoint_idx]->distance > 1900)
	{
		drawDirection(63, 97, 60, pp_trekpoints[nearest_trekpoint_idx]->azimuth_relative_rad, WHITE);
		trek_direction_old = pp_trekpoints[nearest_trekpoint_idx]->azimuth_relative_rad;

		sprintf(&Line[6][2], "%4dm ", (uint16_t)pp_trekpoints[nearest_trekpoint_idx]->distance);
		ST7735_WriteString(26, 6*19, &Line[6][2], Font_11x18, WHITE,BLACK);
	}

	sprintf(&Line[7][4], "%d", nearest_trekpoint_idx);		//"%dm", (uint16_t)pp_trekpoints[nearest_trekpoint_idx]->distance);
	ST7735_WriteString(54, 7*19, &Line[7][4], Font_11x18, WHITE,BLACK);
}
