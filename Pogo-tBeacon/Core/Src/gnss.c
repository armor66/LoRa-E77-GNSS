#include <stdio.h>			//for 'sprintf'
#include <string.h>
#include <math.h>
#include "gnss.h"
#include "settings.h"
#include "lrns.h"
//#include "compass.h"
#include "gpio.h"
#include "usart.h"
//#include "lptim.h"
#include "lcd_display.h"

const double rad_2_deg = 57.29577951308232;        //rad to deg multiplyer
const double deg_2_rad = 0.00000000174532925199433;       //deg to rad multiplyer

double host_lat_lon_rad[2];

struct settings_struct *p_settings_gnss;
struct devices_struct **pp_devices_gnss;

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

//uint8_t nav_pvt_ram_flag = 0;
//uint8_t out_ubx_ram_flag = 0;
//uint8_t out_nmea_ram_flag = 0;
uint8_t new_options_flag = 0;

const uint8_t ubx_mon_ver[] = {0xB5, 0x62, 0x0a, 0x04, 0x00, 0x00, 0x0e, 0x34};
//set 38400:
//uint8_t set_baudrate[] = {0xB5, 0x62, 0x06, 0x8A, 0x0C, 0x00, 0x00, 0x07, 0x00, 0x00, 0x01, 0x00, 0x52, 0x40, 0x00, 0x96, 0x00, 0x00, 0xCC, 0x61};
//set 57600
const uint8_t set_baudrate[] =    {0xB5, 0x62, 0x06, 0x8A, 0x0C, 0x00, 0x00, 0x07, 0x00, 0x00, 0x01, 0x00, 0x52, 0x40, 0x00, 0xE1, 0x00, 0x00, 0x17, 0x42};
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
const uint8_t set_aggressive_pm[] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0x6F};
const uint8_t set_balanced_pm[]  =  {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x95, 0x61};

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
	main_flags.uartIdx = 0;
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

	if(main_flags.GPSconfigureFlag)
	{
		configure_gps();
	}
	else
	{
		main_flags.GPScheckFlag = 1;					//check valid gnss settings: if "nav_pvt_ram_flag" has set
		restart_uart(GPS_BAUDRATE_38400);
		HAL_Delay(100);
		serialPrint(req_nav_pvt_ram, sizeof(req_nav_pvt_ram));
		HAL_Delay(400);
	}
	if(!main_flags.nav_pvt_ram_flag)
	{
		led_blue_on();
//		send_ubx(UBX_CLASS_CFG, UBX_CFG_RST, &cfg_rst_cold_restart[0], sizeof(cfg_rst_cold_restart));
//		HAL_Delay(100);
		main_flags.GPScold_restarted = 1;
		main_flags.GPScheckFlag = 1;					//check valid gnss settings: if "nav_pvt_ram_flag" has set
		restart_uart(GPS_BAUDRATE_38400);
		HAL_Delay(100);
		serialPrint(req_nav_pvt_ram, sizeof(req_nav_pvt_ram));
		HAL_Delay(400);
	}
/*set aggressive 1Hz power mode to RAM:*/
//	serialPrint(set_aggressive_pm, sizeof(set_aggressive_pm));
//	serialPrint(to_ram_bbr_flash, sizeof(to_ram_bbr_flash));
/*	set CFG-TP-PERIOD_LOCK_TP1=3.000.000 to manage ADC and UART only ones on period*/
//	if(p_settings_gnss->spreading_factor == 12) serialPrint(set_three_seconds, sizeof(set_three_seconds));
}

void configure_gps(void)
{
	volatile int8_t row;
	uint16_t baudRate[BAUDRATE_MAX_IND +2] = {96, 192, 384, 576, 1152, 2304, 0};

	led_blue_off();	//just to off after start
	lcd_on();		//lptim1_start(16, main_flags.brightness);	//HAL_LPTIM_PWM_Start(&hlptim1, 16, main_flags.brightness);

restart_configuration:

	if(new_options_flag)	//re-init gnss module:
	{
		send_ubx(UBX_CLASS_CFG, UBX_CFG_RST, &cfg_rst_cold_restart[0], sizeof(cfg_rst_cold_restart));
		HAL_Delay(500);
	}

	main_flags.ubx_hwVersion = 0;
	fill_screen(BLACK);

	for(baudRateInd = 0; baudRateInd < (5 + 1); baudRateInd++)
    {
		restart_uart(baudRateInd);
       	if(HAL_UART_GetState(&huart2) != HAL_UART_STATE_BUSY_RX)	//if gps module does not transmit
       	{
       		serialPrint(ubx_mon_ver, sizeof(ubx_mon_ver));
       		HAL_Delay(200);
       	}
        if(main_flags.ubx_hwVersion) break;
    }
    //restart IRQ for either serialPrint
	restart_uart(baudRateInd);
//    while(HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_RX)	//if gps module does not transmit
    serialPrint(req_nav_pvt_ram, sizeof(req_nav_pvt_ram));
    HAL_Delay(75);

	restart_uart(baudRateInd);
//    while(HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_RX)	//if gps module does not transmit
    serialPrint(req_out_ubx_ram, sizeof(req_out_ubx_ram));
    HAL_Delay(75);

	restart_uart(baudRateInd);
//    while(HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_RX)	//if gps module does not transmit
    serialPrint(req_out_nmea_ram, sizeof(req_out_nmea_ram));
    HAL_Delay(75);
	while(1)
	{
		if(baudRateInd == GPS_BAUDRATE_38400)
		{
			led_green_on();
			led_red_off();
		}else
		{
			led_red_on();
			led_green_off();
		}
		(main_flags.nav_pvt_ram_flag && main_flags.out_ubx_ram_flag && !main_flags.out_nmea_ram_flag)? led_blue_on(): led_blue_off();

		row	= 0;

		if(baudRate[baudRateInd])
		{
			sprintf(&string_buffer[row][0], "GNSS version M%d", main_flags.ubx_hwVersion);
			sprintf(&string_buffer[row+1][0], "Baud rate %4d00", baudRate[baudRateInd]);
			if(baudRateInd != GPS_BAUDRATE_38400) draw_str_by_rows(7, (row+1)*14+5, &string_buffer[row+1][0], &Font_7x9, RED,BLACK);
		}else
		{
			sprintf(&string_buffer[row+0][0], "  GNSS module");
			sprintf(&string_buffer[row+1][0], "   NOT FOUND");
		}
		for (; row < 2; row++) {
			draw_str_by_rows(7, row*14+5, &string_buffer[row][0], &Font_7x9, YLWGRN,BLACK);
		}

		row	= 2;
		char *boolean[] = {"false", "true"};

		sprintf(&string_buffer[row][0],   "Output NAVPVT%5s", boolean[main_flags.nav_pvt_ram_flag]);
		(!main_flags.nav_pvt_ram_flag)? draw_str_by_rows(0, row*14+5, &string_buffer[row][0], &Font_7x9, RED,BLACK):
		draw_str_by_rows(0, row*14+5, &string_buffer[row][0], &Font_7x9, GREEN,BLACK);

		sprintf(&string_buffer[row+1][0], "Output UBLOX %5s", boolean[main_flags.out_ubx_ram_flag]); //", baudRate[baudRateInd]);
		(!main_flags.out_ubx_ram_flag)? draw_str_by_rows(0, (row+1)*14+5, &string_buffer[row+1][0], &Font_7x9, RED,BLACK):
		draw_str_by_rows(0, (row+1)*14+5, &string_buffer[row+1][0], &Font_7x9, GREEN,BLACK);

		sprintf(&string_buffer[row+2][0], "Output NMEA  %5s", boolean[main_flags.out_nmea_ram_flag]);
		(main_flags.out_nmea_ram_flag)? draw_str_by_rows(0, (row+2)*14+5, &string_buffer[row+2][0], &Font_7x9, RED,BLACK):
		draw_str_by_rows(0, (row+2)*14+5, &string_buffer[row+2][0], &Font_7x9, GREEN,BLACK);


		row	= 5;	//+=3;
		if(1)
		{
			draw_str_by_rows(0, (row)*14+7, "   Press PWR to  ", &Font_7x9, YELLOW,BLACK);
			draw_str_by_rows(0, (row+=1)*14+5, "  RESTART DEVICE ", &Font_7x9, ORANGE,BLACK);
			draw_str_by_rows(0, (row+=1)*14+7, "   Press OK to   ", &Font_7x9, YELLOW,BLACK);
			draw_str_by_rows(0, (row+=1)*14+5, "  SET NEW DATA   ", &Font_7x9, ORANGE,BLACK);
			draw_str_by_rows(0, (row+=1)*14+7, "   Press ESC to  ", &Font_7x9, YELLOW,BLACK);
			draw_str_by_rows(0, (row+=1)*14+5, "  SET BAUD RATE   ", &Font_7x9, ORANGE,BLACK);
		}
		//both OK and ESC pressed
		if((new_options_flag != 1) && !(GPIOA->IDR & BTN_2_Pin) && !(GPIOA->IDR & BTN_3_Pin))
		{
			new_options_flag = 1;
			serialPrint(revert_to_default, sizeof(revert_to_default));
			HAL_Delay(100);
			//not need if default 38400 used
//			serialPrint(revert_baudrate, sizeof(revert_baudrate));
//			HAL_Delay(100);
    		goto restart_configuration;
		}
		if ((new_options_flag != 2) && !(GPIOA->IDR & BTN_2_Pin) && (GPIOA->IDR & BTN_3_Pin))	//OK to set values
	    {
			new_options_flag = 2;
//			while(HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_RX)	//if gps module does not transmit
			serialPrint(set_crucial_opts, sizeof(set_crucial_opts));
			HAL_Delay(100);
//			serialPrint(set_aggressive_pm, sizeof(set_agressive_pm));
//			serialPrint(set_balanced_pm, sizeof(set_balanced_pm));	//not need if aggressive was not saved
//			HAL_Delay(100);
			serialPrint(to_ram_bbr_flash, sizeof(to_ram_bbr_flash));
			HAL_Delay(100);
			goto restart_configuration;
	    }
		if ((new_options_flag != 3) && !(GPIOA->IDR & BTN_3_Pin) && (GPIOA->IDR & BTN_2_Pin) && (baudRateInd != GPS_BAUDRATE_38400))
    	{
			new_options_flag = 3;
			serialPrint(revert_baudrate, sizeof(revert_baudrate));
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
	}
}

