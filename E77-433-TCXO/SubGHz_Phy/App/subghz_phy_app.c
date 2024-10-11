/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz_phy_app.c
  * @author  MCD Application Team
  * @brief   Application of the SubGHz_Phy Middleware
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "sys_app.h"
#include "subghz_phy_app.h"
#include "radio.h"

/* USER CODE BEGIN Includes */
#include "iwdg.h"
#include "stm32_systime.h"
#include "stm32_timer.h"
#include "stm32_seq.h"
#include "utilities_def.h"
#include "app_version.h"
#include "adc_if.h"
//#include "ublox.h"
#include "tim.h"
#include "lptim.h"
#include "buttons.h"
#include <stdlib.h>
#include "settings.h"
#include "spi.h"
#include "stdio.h"
#include "lrns.h"
#include "compass.h"
#include "ST7735.h"

#include "gnss.h"
#include "gpio.h"		//#include "main.h"		//***main_flags_struct
#include "radio_driver.h"

struct main_flags_struct main_flags = {0};
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t pps_counter = 0;
uint8_t pps_flag = 0;
uint8_t time_slot = 0;
uint8_t transmit_iq_inverted_flag = 0;
uint8_t long_beep_ones = 0;
//uint8_t time_slot_timer_ovf = 0;		//added to main flags
//uint8_t gps_speed = 0;
//int16_t gps_heading = 0;
uint16_t no_PPS_gap1 = 2705;			// RX1 to PPS gap = 677after receive NODE_ID1 779mS = 30mS + (5slots + Processing) * 150mS
uint16_t no_PPS_gap2 = 1705;			//368
uint16_t no_PPS_gap3 = 705;			//68
uint16_t endRX_2_TX = 0;

//uint8_t *p_update_interval_values;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Radio events function pointer */
static RadioEvents_t RadioEvents;
/* USER CODE BEGIN PV */
//uint8_t BufferAirSize = BUFFER_AIR_SIZE;
//uint8_t BufferRxSize = BUFFER_RX;
uint8_t bufferTx[BUFFER_AIR_SIZE];
uint8_t bufferRx[BUFFER_RX];

uint8_t bufNode[NODES][BUFFER_RX];
//uint8_t validFixFlag[4] = {0};

#define PVTsize 100
uint8_t PVTbuffer[PVTsize] = {0};
#define UBX_HW_VER_SIZE 40				//for UBX_MON_VER: Header(2)+Class(1)+ID(1)+Length(2)+swVersion(30)+hwVersion(10)
#define UBX_CFG_SIZE 14					//answer for CFG-MSGOUT-UBX_NAVPVT_UART1, CFG-UART1OUTPROT-UBX(NMEA)
uint8_t GNSSbuffer[UBX_HW_VER_SIZE] = {0};
#define UBX_CFG_FLAG 10				//hwVersion(10)
uint8_t ubx_hwVersion = 0;
//uint8_t uartByte = 0;
uint8_t uartIdx = 0;

static States_t State = RX_START;		//на первые 30 секунд ??? RX_DONE ???

//static uint32_t WatchDogRx = WATCHDOG_RX_PERIOD;
bool isChannelFree = true;

uint8_t button_code = 0;
uint8_t processing_button = 0;

uint8_t *p_tx_power_values_phy;

char Lines[24][32];
int8_t rssi_by_channel[2][FREQ_CHANNEL_LAST - FREQ_CHANNEL_FIRST + 1];
int8_t scanRadioFlag = 0;
int8_t channel_ind = 0;
void scan_channels(void);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/*!
 * @brief Function to be executed on Radio Tx Done event
 */
static void OnTxDone(void);

/**
  * @brief Function to be executed on Radio Rx Done event
  * @param  payload ptr of buffer received
  * @param  size buffer size
  * @param  rssi
  * @param  LoraSnr_FskCfo
  */
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo);

/**
  * @brief Function executed on Radio Tx Timeout event
  */
static void OnTxTimeout(void);

/**
  * @brief Function executed on Radio Rx Timeout event
  */
static void OnRxTimeout(void);

/**
  * @brief Function executed on Radio Rx Error event
  */
static void OnRxError(void);

/* USER CODE BEGIN PFP */
struct settings_struct *p_settings_phy;
struct devices_struct **pp_devices_phy;
/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SubghzApp_Init(void)
{
  /* USER CODE BEGIN SubghzApp_Init_1 */
	p_tx_power_values_phy = get_tx_power_values();
	p_settings_phy = get_settings();
	pp_devices_phy = get_devices();

	SYS_InitMeasurement();
  /* USER CODE END SubghzApp_Init_1 */

  /* Radio initialization */
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);

  /* USER CODE BEGIN SubghzApp_Init_2 */
  srand(Radio.Random());

      Radio.SetTxConfig(MODEM_LORA, p_tx_power_values_phy[p_settings_phy->tx_power_opt], 0, LORA_BANDWIDTH,			//MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
    		  p_settings_phy->spreading_factor, p_settings_phy->coding_rate_opt,				//LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                        true, 0, 0, LORA_IQ_NORMAL, TX_TIMEOUT_VALUE);

      Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, p_settings_phy->spreading_factor,		//MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
    		  p_settings_phy->coding_rate_opt, 0, LORA_PREAMBLE_LENGTH,
                        LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
						BUFFER_AIR_SIZE, true, 0, 0, LORA_IQ_NORMAL, true);				//0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

      Radio.SetMaxPayloadLength(MODEM_LORA, BUFFER_AIR_SIZE);

      Radio.SetChannel((433000 + 50 + p_settings_phy->freq_channel * 25) * 1000);	//(RF_FREQUENCY);
//    Radio.SetChannel((800 + p_freq_region_values_phy[p_settings_phy->freq_region_opt]) * 1000000);	//(RF_FREQUENCY);
//      Radio.Rx(RX_TIMEOUT_VALUE);
	    HAL_Delay(Radio.GetWakeupTime() + TCXO_WORKAROUND_TIME_MARGIN);

	    if(scanRadioFlag) scan_channels();
  /* USER CODE END SubghzApp_Init_2 */
}

/* USER CODE BEGIN EF */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
    case  BTN_1_Pin:					//PC0		PWR button	void EXTI0_IRQHandler(void)
    	pps_counter = 0;//    	pp_devices_phy[p_settings_phy->device_number]->lcd_timeout = 0;
    	disable_buttons_interrupts();
    	//EXTI->PR = EXTI_PR_PR5;			//clear interrupt
    	processing_button = BUTTON_PWR;
    	__HAL_TIM_CLEAR_FLAG(&htim2, TIM_SR_UIF);		// очищаем флаг
    	HAL_TIM_Base_Start_IT(&htim2);
    	break;

    case  BTN_2_Pin:					//PB4		UP/OK button		void EXTI4_IRQHandler(void)
    	pps_counter = 0;//    	pp_devices_phy[p_settings_phy->device_number]->lcd_timeout = 0;
    	disable_buttons_interrupts();
    	//EXTI->PR = EXTI_PR_PR4;			//clear interrupt
    	processing_button = BUTTON_UP_OK;
    	__HAL_TIM_CLEAR_FLAG(&htim2, TIM_SR_UIF);		// очищаем флаг
    	HAL_TIM_Base_Start_IT(&htim2);
        break;

    case  BTN_3_Pin:					//PB3		DOWN/ESC button	void EXTI3_IRQHandler(void)
    	pps_counter = 0;//    	pp_devices_phy[p_settings_phy->device_number]->lcd_timeout = 0;
    	disable_buttons_interrupts();
    	//EXTI->PR = EXTI_PR_PR3;			//clear interrupt
    	processing_button = BUTTON_DOWN_ESC;
    	__HAL_TIM_CLEAR_FLAG(&htim2, TIM_SR_UIF);		// очищаем флаг
    	HAL_TIM_Base_Start_IT(&htim2);
        break;

	case  PPS_Pin:				// interrupt on PPS front - the same as on TIM16 interrupt	void EXTI1_IRQHandler(void)
		getADC_sensors(p_settings_phy->device_number);								//lrns.c
		if(pps_counter++ > 60) {													//if 60sec no buttons activity
			pps_counter = 60;
			HAL_LPTIM_PWM_Stop(&hlptim1);											//lcd_off();
			pp_devices_phy[p_settings_phy->device_number]->display_status = 0;		//for TPS7330 Vthresold=2.64V
			if(pp_devices_phy[p_settings_phy->device_number]->batt_voltage < 30) {	//for TPS7333 Vthresold=2.87V(287-270=17) 0==270(2.70V) (actually ~2.95V)
				longBeepsBlocking(1);												//long beep to prevent silent "RESET"
 				HAL_Delay(50);
				release_power();
			}
		}

	if(PVTbuffer[16]%p_settings_phy->devices_on_air == 0)// || (p_settings_phy->spreading_factor == 12))	//start timer ones of 3, 4 or 5 seconds
	{
		HAL_TIM_Base_Stop_IT(&htim1);
		pps_flag = 1;			//todo if fix valid	(PVTbuffer[21+6] & 0x01)
		time_slot = 0;			// from TIM1_IRQ case: 2
		main_flags.time_slot_timer_ovf = 0;
		__HAL_TIM_SET_COUNTER(&htim1, 0);
		__HAL_TIM_CLEAR_FLAG(&htim1, TIM_SR_UIF);		// очищаем флаг
		HAL_TIM_Base_Start_IT(&htim1);					// start time slot timer right after PPS
		if(p_settings_phy->spreading_factor == 12) led_blue_on();
	}

	uartIdx = 0;
	USART2->RDR;				//!!!очистка регистра чтением!!!иначе прерывание сработает сразу
	USART2->CR1 = 0x00000000;	//иначе работает только после resetа
	USART2->CR1 = USART_CR1_UE;
	USART2->CR1 |= USART_CR1_RE | USART_CR1_RXNEIE_RXFNEIE;	//1: USART interrupt generated whenever ORE = 1 or RXFNE = 1 in the USART_ISR register

//	if((p_settings_phy->spreading_factor < 12) || !time_slot)
//	{
	if(p_settings_phy->device_number == (time_slot + 1)) clear_fix_data(time_slot + 1);	//before uart handling finished
	if(p_settings_phy->spreading_factor == 12 && p_settings_phy->device_number == 2) clear_fix_data(2);	//for beacon №2 only to transmit
	if(p_settings_phy->spreading_factor != 12) led_blue_on();	//PPS received
		led_green_on();
//	}
//avoid extra beeps
	led_w_off();
	main_flags.short_beeps = 0;
//	HAL_IWDG_Refresh(&hiwdg);	moved to main.c while(1)
		break;

    default:
    	break;
  }
}

void USART2_IRQHandler(void)			//GNSS_StateHandle *GNSS An interrupt is generated if RXNEIE = 1 in the USART_CR1 register
{										//USART interrupt generated whenever ORE = 1 or RXNE = 1 in the USART_ISR register
    if (USART2->ISR & USART_ISR_RXNE_RXFNE)	//сброс аппаратный чтением регистра
    {
    	if(GPSconfigureFlag || GPScheckFlag)	//GPSconfigureFlag or GPScheckFlag has set
		{
			if(uartIdx > UBX_HW_VER_SIZE) uartIdx = 0;		//secure check
			GNSSbuffer[uartIdx] = USART2->RDR;
			//if header is not valid
			if(uartIdx == 1 && (GNSSbuffer[0] != 0xB5 || GNSSbuffer[1] != 0x62))	// || GNSSbuffer[2] != 0x0A || GNSSbuffer[3] != 0x04))
	   		{
	   			USART2->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;	//0: Interrupt inhibited
//	   			memset(&GNSSbuffer, 0, UBX_HW_VER_SIZE);
				GPScheckFlag = 0;	//after init_gnss one check only
	   		}
			//if it is answer for UBX-CFG-UART1OUT
			else if((uartIdx == UBX_CFG_SIZE) && GNSSbuffer[2] == 0x06 && GNSSbuffer[3] == 0x8B && GNSSbuffer[4] == 0x09)
			{
				if(GNSSbuffer[UBX_CFG_FLAG] == 0x07)
				{
					nav_pvt_ram_flag = GNSSbuffer[UBX_CFG_SIZE];
				}
				else if(GNSSbuffer[UBX_CFG_FLAG] == 0x01)
				{
					out_ubx_ram_flag = GNSSbuffer[UBX_CFG_SIZE];
				}
				else if(GNSSbuffer[UBX_CFG_FLAG] == 0x02)
				{
					out_nmea_ram_flag= GNSSbuffer[UBX_CFG_SIZE];
				}
	   			USART2->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;	//0: Interrupt inhibited
		   		memset(&GNSSbuffer, 0, UBX_CFG_SIZE);
				GPScheckFlag = 0;	//after init_gnss one check only
			}
			//if it is answer for UBX_HV_VER
	   		else if((uartIdx == UBX_HW_VER_SIZE) && GNSSbuffer[2] == 0x0A && GNSSbuffer[3] == 0x04)
	   		{
	   			USART2->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;	//0: Interrupt inhibited
	   			(GNSSbuffer[UBX_HW_VER_SIZE-1] == 0x41)? (ubx_hwVersion = (GNSSbuffer[UBX_HW_VER_SIZE-1] - 0x37)): (ubx_hwVersion = (GNSSbuffer[UBX_HW_VER_SIZE-1] - 0x30));
	   			memset(&GNSSbuffer, 0, UBX_HW_VER_SIZE);
	   		}
	   		else uartIdx++;	//do not increment index in case 1 or 2
		}
    	else		//normal operation if(!GPSconfigureFlag && !GPScheckFlag)
	    {
	    	PVTbuffer[uartIdx] = USART2->RDR;	   		//PVTbuffer[uartIdx] = uartByte;
	   		if(uartIdx == 3 && (PVTbuffer[0] != 0xB5 || PVTbuffer[1] != 0x62 || PVTbuffer[2] != 0x01 || PVTbuffer[3] != 0x07))
	   		{
	   			USART2->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;
//	   			uartIdx = 0;
	   			memset(&PVTbuffer, 0, PVTsize);			//for(uint8_t i = 0; i < PVTsize; i++) { PVTbuffer[i] = 0; }
	   		}
	   		else if (uartIdx >= 83)	//83 = 77(pDOP) + 6 (uartIdx >= PVTsize)
	   		{
	   			USART2->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;	//0: Interrupt inhibited
//	   			uartIdx = 0;
	   			ublox_to_this_device(p_settings_phy->device_number);
	   			led_green_off();
	   		} else uartIdx++;	//do not increment index in case 1 or 2
		}
	}
}
uint8_t pattern_index;
const uint8_t timeslot_pattern[2][103] =
//	| PVT|Slot1 OnRxDone time <642mS		+358mS|Slot2 OnTxDone time <627mS		  | PVT|Slot3					   | draw menu|
//	|----|-------------------------|---------|----|-------------------------|---------|----|-------------------------|---------|--
//	0 50 100mS	                             1000 1100mS							  2000 2100								 3000
{{0,0,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,7,4,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,7,//4,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,7,4,6 };
//	| PVT|Slot3					   | draw menu|PVT|Slot4					| draw menu|PVT|Slot5					 | draw menu|
//	|----|-------------------------|---------|----|-------------------------|---------|----|-------------------------|---------|--
//	2000 2100mS								 3000 3100mS							  4000 4100mS							   5000mS
	4,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,7,4,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,7,4,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,7,4,6 },
//	SF = 12, CR1(4/5), Packet duration in air = 1155.1
//  | PVT|Slot1 		OnRxDone time		<1200mS		  |draw menu| |Slot2 			OnRxDone time		<1200mS	   |draw menu|
//	|----|-----------------------------------|------------|---------|-----------------|------------------------------|---------|
//	0 50 100mS	                             1000		 1300mS						  2000				    	  	2750	   3000
 {0,0,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,3,4,5,4,7,1,2,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,7,6 }};
//	|----|---------------------------|--------------------|-----------|--------------------|---------------------------|-------|
//	0 50 100mS	                    +700     			+1200mS	     1600				  	  +700					 +1200	   3000
//{0,0,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,3,4,5,4,1,2,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,3,4,5,4,7,4,6 }};
// {0,1, 2,0,0,0,0,0,0,0,0,0,0,0,0,0,8,0,0,0,0,0, 0,0,0,0,3,4,7,4,1,2,0,0,0,0,0,0,0,0,0,0, 8,0,0,0,0,0,0,0,0,0,0,0,0,3,4,7,4,0,0,6 };
//in case 8 (end of rx packet from device №3, IQ inverted)
//might do long_beep 500mS or some stuff, depends on received flags
//todo add option to choose what beacon to address, therefore in what slot to transmit (4 flags, 2 for each beacon)
//todo inverse LORA_IQ_INVERSION in Radio.SetTxConfig and Radio.SetRxConfig for module and beacons respectively
//todo for beacons set Radio.SetRxConfig BUFFER_AIR_SIZE = 3

int8_t find_nearest_trekpoint_flag = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1) 	//check if the interrupt comes from TIM1
	{
	  if(!scanRadioFlag)
	  {
		  main_flags.time_slot_timer_ovf++;	//0...59

//		if (pps_flag)
//		{
			(p_settings_phy->spreading_factor == 12)? (pattern_index = 1): (pattern_index = 0);
			switch (timeslot_pattern[pattern_index][main_flags.time_slot_timer_ovf])
			{
			case 0:			//do nothing
				break;

			case 1:			//50mS
				time_slot++;
//clear what should be received or not in this slot after draw menu has finished
				if(p_settings_phy->device_number != time_slot) clear_fix_data(time_slot);
//set receive LORA_IQ_NORMAL in other time slot ones if transmitting with LORA_IQ_INVERTED has occurred (for [p_settings_phy->device_number == 3] only)
				if(transmit_iq_inverted_flag)	//(p_settings_phy->spreading_factor == 12) && (pp_devices_phy[3]->beeper_flag != time_slot)
				{
					transmit_iq_inverted_flag = 0;

					Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, p_settings_phy->spreading_factor,
					p_settings_phy->coding_rate_opt, 0, LORA_PREAMBLE_LENGTH, LORA_SYMBOL_TIMEOUT,
					LORA_FIX_LENGTH_PAYLOAD_ON,	BUFFER_AIR_SIZE, true, 0, 0, LORA_IQ_NORMAL, true);
				}
				break;

			case 2:			//100mS
				if(time_slot == p_settings_phy->device_number)
				{	//if this device doesn't get gnss fix via uart 100mS after PPS, delay for full pattern time
					if(!pp_devices_phy[p_settings_phy->device_number]->valid_fix_flag)	main_flags.fix_valid--;
					if(main_flags.fix_valid)	//do not transmit if no GNSS FIX
					{
						if(pp_devices_phy[p_settings_phy->device_number]->display_status)
						{
							endRX_2_TX = __HAL_TIM_GET_COUNTER(&htim17);				//save interval from RX-end to TX-start
							HAL_TIM_Base_Stop(&htim17);
						}
						main_flags.permit_actions = 0;
						transmit_data();		//State = TX_START;
					}else main_flags.fix_valid = 0;		//just to avoid negative values
				}
				else//if(time_slot != p_settings_phy->device_number)
				{	//transmit on demand if beeper_flag has set(choose witch beacon to send) with LORA_IQ_INVERTED
					if((p_settings_phy->spreading_factor == 12) && (pp_devices_phy[3]->beeper_flag == time_slot))	//for [p_settings_phy->device_number == 3] only
					{	//beeper_flags inverted to transmit in the slot, beacon can get: slot1 for beacon2, slot2 for beacon1
							main_flags.permit_actions = 0;
							transmit_iq_inverted_flag = 1;
							//set transmit LORA_IQ_INVERTED
							Radio.SetTxConfig(MODEM_LORA, p_tx_power_values_phy[p_settings_phy->tx_power_opt], 0,
								LORA_BANDWIDTH,	p_settings_phy->spreading_factor, p_settings_phy->coding_rate_opt,
								LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON, true, 0, 0, LORA_IQ_INVERTED, TX_TIMEOUT_VALUE);
							transmit_data();		//State = TX_START;
					}
					else//(p_settings_phy->spreading_factor != 12) or !(pp_devices_phy[3]->beeper_flag == time_slot)
					{	//manage (pp_devices_phy[time_slot]->beeper_flag) on it own slot only, if beeper_flag received previously in this slot)
						if(pp_devices_phy[time_slot]->beeper_flag)
						{
							led_w_on();
							pp_devices_phy[time_slot]->beeper_flag = 0;
							long_beep_ones = 1;	//set here to finish on case 3 after starts here only
						}
						Radio.Rx(0);			//start to receive if SF != 12 or no beeper_flag has set on SF ==12
					}
				}
				led_blue_off();		//occurrence case 2 after PPS
				break;

			case 3:			//check if remote devices on the air, draw menu items
				if(time_slot != p_settings_phy->device_number)	//for receiver slot only
				{
					if(pp_devices_phy[p_settings_phy->device_number]->valid_fix_flag &&	//host gps data is valid (implemented in rx_to_devices())
					pp_devices_phy[time_slot]->beacon_traced &&							//timeout_threshold is set & valid data has received once
					!pp_devices_phy[time_slot]->valid_fix_flag)							//not valid data received now
					{	//do decrement beacon_traced in lrns.c, except when beep flag sent to one of two beacon in beacon mode (SF12)
						if(!((p_settings_phy->spreading_factor == 12) && (pp_devices_phy[3]->beeper_flag == time_slot))) check_traced(time_slot);
						//if(pp_devices_phy[time_slot]->beacon_flag && !!! do it not for beacon only:
						if(pp_devices_phy[time_slot]->beacon_traced * (p_settings_phy->devices_on_air - 1) < 14) shortBeeps(1);	//do beeps if beacon was traced and lost gps fix
						if(pp_devices_phy[time_slot]->beacon_lost)						//if beacon_traced became zero
						{
							memory_points_save();		//save beacon trace if it was traced and no validFixFlag[time_slot] for timeout_threshold
						//start long beep on next case 2 в слоте того устройства, которое пропало
							pp_devices_phy[time_slot]->beeper_flag = 1;
//							shortBeeps(time_slot);
						}
					}
				}
				if(find_nearest_trekpoint_flag)
				{
					__disable_irq();
					find_nearest_trekpoint();
				}
				while(find_nearest_trekpoint_flag);		//__enable_irq(); after that
				read_north();

				if(long_beep_ones)		//(main_flags.beeper_flag_received || long_beep_ones)	//used on case 2 previously
				{
					led_w_off();
//					main_flags.beeper_flag_received = 0;
					long_beep_ones = 0;
				}
				main_flags.short_beeps? led_w_on(): (main_flags.update_screen = 1);
				main_flags.permit_actions = 1;		//process buttons here after
				break;

			case 4:
				if(main_flags.short_beeps)
				{
					led_w_off();
					main_flags.short_beeps--;
				}
				break;

			case 5:
				if((main_flags.short_beeps) && (main_flags.short_beeps < 3))
				{
					led_w_on();
				}
				break;

			case 7:
				if(main_flags.short_beeps == 1)
				{
					led_w_on();
				}
				break;

			case 6:					//if PPS did not come, but cycle has complete + 50mS
				getADC_sensors(p_settings_phy->device_number);
				clear_fix_data(p_settings_phy->device_number);
				HAL_TIM_Base_Stop_IT(&htim1);
				pps_flag = 0;
				time_slot = 0;			// from TIM1_IRQ case: 2
				main_flags.time_slot_timer_ovf = 0;
				__HAL_TIM_SET_COUNTER(&htim1, 0);
				__HAL_TIM_CLEAR_FLAG(&htim1, TIM_SR_UIF);		// очищаем флаг
				HAL_TIM_Base_Start_IT(&htim1);
				break;

			default:
				break;
			}	//end of switch/case
//		} else {	//no pps_flag
//			main_flags.update_screen = 1;
//		}
	  }
  	  else	// if(scanRadioFlag)
      {
  		  led_toggle();	//led_red
  		  Radio.SetChannel((433000 + 50 + (channel_ind*5 + FREQ_CHANNEL_FIRST) * 25) * 1000);	//(RF_FREQUENCY);
  		  Radio.Rx(45);
  		  HAL_Delay(Radio.GetWakeupTime());
  		  rssi_by_channel[1][channel_ind] = Radio.Rssi(MODEM_LORA);
  		  sprintf(&Lines[channel_ind + 1][0], "Ch %02d RSSI %02ddBm", (channel_ind*5 + FREQ_CHANNEL_FIRST), Radio.Rssi(MODEM_LORA));
  		  channel_ind++;
  		  if(((FREQ_CHANNEL_LAST - FREQ_CHANNEL_FIRST)/5 - 1) < channel_ind) HAL_TIM_Base_Stop_IT(&htim1);	// 12 < channel_ind
      }
  	}

	if(htim->Instance == TIM2)					//Scan buttons interval	void TIM3_IRQHandler(void)
	{											//TIM3->SR &= ~TIM_SR_UIF;        //clear gating timer int
		if (main_flags.buttons_scanned == 0)	//if not scanned yet
		{
			button_code = scan_button(processing_button);
			if (button_code)
			{
				main_flags.buttons_scanned = 1;
			}
		}
	}

	if(htim->Instance == TIM16) 		// the same as on PPS front interrupt
	{
		HAL_TIM_Base_Stop_IT(&htim16);		//start onRxDone if no PPS
//    	HAL_TIM_OnePulse_Stop_IT(&htim16, 0);
//		HAL_TIM_Base_Stop_IT(&htim1);
//		__HAL_TIM_SET_COUNTER(&htim1, 0);
		__HAL_TIM_CLEAR_FLAG(&htim1, TIM_SR_UIF); // очищаем флаг
		HAL_TIM_Base_Start_IT(&htim1);		//timer1_start();

		pps_flag = 1;
		time_slot = 0;
		main_flags.time_slot_timer_ovf = 0;
//    	endRX_2_PPS = __HAL_TIM_GET_COUNTER(&htim17);		//сохранить интервал от конца RX до фронта PPS
//		HAL_TIM_Base_Stop_IT(&htim17);
	}

	if(htim->Instance == TIM17)			//started on last case BTN_UP
	{
		HAL_TIM_Base_Stop_IT(&htim17);
		current_point_group = 0;
	}
}
/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/
static void OnTxDone(void)
{
  /* USER CODE BEGIN OnTxDone */
	main_flags.permit_actions = 1;
	led_red_off();
  /* USER CODE END OnTxDone */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
  /* USER CODE BEGIN OnRxDone */
	//replaced BufferAirSize, BufferRxSize with macros
	if(pp_devices_phy[p_settings_phy->device_number]->display_status == 1) {
		HAL_TIM_Base_Stop_IT(&htim17);				//if not stopped by IRQ
		__HAL_TIM_SET_COUNTER(&htim17, 0);			// измеряем интервал от RX_DONE до TX по отсчету TIM17
		__HAL_TIM_SET_AUTORELOAD(&htim17, 9999);
		HAL_TIM_Base_Start(&htim17);				//сбросить счетчик в конце RX и начать отсчет
	}
//	memset(validFixFlag, 0, 4);
//	BufferAirSize = size;
	memcpy(bufferRx, payload, BUFFER_AIR_SIZE);		//BufferAirSize
	bufferRx[BUFFER_AIR_SIZE] = (int8_t)rssi;				//BufferAirSize + 1
	bufferRx[BUFFER_AIR_SIZE + 1] = LoraSnr_FskCfo;	//BufferAirSize + 2
	//if received time slot == device number and it fix is valid
	if(((bufferRx[0] & 0x07) == time_slot) && ((bufferRx[1] & 0x10) >> 4))
	{	//todo replace it all for:
		//&& ((bufferRx[14] & 0x10) >> 4)) rx_to_devices(time_slot); where (uint8_t *buffer = bufferRx;)
		rx_to_devices(time_slot); //where (uint8_t *buffer = bufferRx;)
		switch (time_slot)
		{
		case 1:
//		ptrNode1rx = bufferRx;
			memcpy(bufNode[1], &bufferRx, BUFFER_RX);		// + rssi + LoraSnr_FskCfo BufferRxSize
//		memcpy(bufNode[1], payload, BufferAirSize);
//		time_slot == bufNode[1][0]? bufNode[1][BufferRxSize] = rssi: (bufNode[1][BufferRxSize] = 0);
//			validFixFlag[1] = ((bufferRx[14] & 0x10) >> 4);
//			if(validFixFlag[1]) rx_to_devices(1);
			break;
		case 2:
//		ptrNode2rx = bufferRx;
			memcpy(bufNode[2], &bufferRx, BUFFER_RX);		// + rssi + LoraSnr_FskCfo BufferRxSize
//		memcpy(bufNode[2], payload, BufferAirSize);
//		time_slot == bufNode[2][0]? bufNode[2][13] = rssi: (bufNode[2][13] = 0);
//			validFixFlag[2] = ((bufferRx[14] & 0x10) >> 4);
//			if(validFixFlag[2]) rx_to_devices(2);
			break;
		case 3:
//		ptrNode3rx = bufferRx;
			memcpy(bufNode[3], &bufferRx, BUFFER_RX);		// + rssi + LoraSnr_FskCfo BufferRxSize
//		memcpy(bufNode[3], payload, BufferAirSize);
		//time_slot == bufNode[3][0]? bufNode[3][BufferRxSize] = rssi:
//			validFixFlag[3] = ((bufferRx[14] & 0x10) >> 4);
//			if(validFixFlag[3]) rx_to_devices(3);
			break;
		case 4:
		//		ptrNode3rx = bufferRx;
			memcpy(bufNode[4], &bufferRx, BUFFER_RX);
			break;
		case 5:
		//		ptrNode3rx = bufferRx;
			memcpy(bufNode[5], &bufferRx, BUFFER_RX);
			break;
		default:
			break;
		}
	}
	//host device valid GPS fix - pDop and accuracy
	if(pp_devices_phy[p_settings_phy->device_number]->valid_fix_flag)	calc_relative_position(time_slot);

	for(int8_t i = 0; i < BUFFER_RX; i++)	//BufferRxSize
	{
		bufferRx[i] = 0;
	}

	State = RX_DONE;			//do nothing

	if (!pps_flag)		//отключено пока тесты	//todo if fix valid	(validFixFlag[time_slot])
	{
/*		HAL_TIM_Base_Stop_IT(&htim1);		//прекратить прием
		__HAL_TIM_SET_COUNTER(&htim1, 0);
		switch (bufferRx[0] & 0x3)		//received NODE_ID
		{
			case 1:
				__HAL_TIM_SET_AUTORELOAD(&htim16, no_PPS_gap1);		//end of RX to start TIM1(PPS)front
				break;
			case 2:
				__HAL_TIM_SET_AUTORELOAD(&htim16, no_PPS_gap2);		//end of RX to start TIM1(PPS)front
				break;
			case 3:
				__HAL_TIM_SET_AUTORELOAD(&htim16, no_PPS_gap3);		//end of RX to start TIM1(PPS)front
				break;
			default:
				break;
		}
		__HAL_TIM_CLEAR_FLAG(&htim16, TIM_SR_UIF);			// очищаем флаг
		HAL_TIM_Base_Start_IT(&htim16);*/
	}
  /* USER CODE END OnRxDone */
}

static void OnTxTimeout(void)
{
  /* USER CODE BEGIN OnTxTimeout */
	State = TX_TO;
  /* USER CODE END OnTxTimeout */
}

static void OnRxTimeout(void)
{
  /* USER CODE BEGIN OnRxTimeout */
	State = RX_TO;
  /* USER CODE END OnRxTimeout */
}

static void OnRxError(void)
{
  /* USER CODE BEGIN OnRxError */
	State = RX_ERR;
  /* USER CODE END OnRxError */
}

/* USER CODE BEGIN PrFD */
//void Sensor_Process(void)
void transmit_data(void)
{
	int8_t beeper_flag_to_transmit;
	int8_t device_number;
	int8_t buffer_to_transmit;

	(pp_devices_phy[p_settings_phy->device_number]->beeper_flag)? (beeper_flag_to_transmit = 1): (beeper_flag_to_transmit = 0);

	if(p_settings_phy->spreading_factor == 12)
	{
		device_number = pp_devices_phy[3]->beeper_flag;
		buffer_to_transmit = 3;	//transmit flags and time todo: change time to gnss fix data
	}
	else
	{
		device_number = p_settings_phy->device_number;
		buffer_to_transmit = BUFFER_AIR_SIZE;
	}

   	  bufferTx[0] =	(IS_BEACON << 7) +
    		  (pp_devices_phy[p_settings_phy->device_number]->emergency_flag << 6) +
			  (pp_devices_phy[p_settings_phy->device_number]->alarm_flag << 5) +
			  (pp_devices_phy[p_settings_phy->device_number]->gather_flag << 4) +
			  (beeper_flag_to_transmit << 3) + device_number;

   	  (pp_devices_phy[p_settings_phy->device_number]->gps_speed > GPS_SPEED_THRS)? (bufferTx[1] |= 1 << 7): (bufferTx[1] &= ~(1 << 7));	//if device is moving
   	  bufferTx[1] = ((PVTbuffer[20+6] & 0x03) << 5) +				//fix type, 2 bits only to transmit	//mask 0b0000 0011
   			  	  	 ((PVTbuffer[21+6] & 0x01) << 4) +				//fix valid, bit0 only				//mask 0b0000 0001
   					 (pp_devices_phy[p_settings_phy->device_number]->batt_voltage/10 & 0x0F);			//mask 0b0000 1111 (0-:150 -> 0-:-15)s
   	  bufferTx[2] = (pp_devices_phy[p_settings_phy->device_number]->p_dop/10 & 0xFF);					//pDop/10 (0...25.5)

	  bufferTx[3] = PVTbuffer[30];	//(uint8_t) ((UBLOX_Handle.lon >> 24) & 0xFF);		//*** longitude
	  bufferTx[4] = PVTbuffer[31];	//(uint8_t) ((UBLOX_Handle.lon >> 16) & 0xFF);
	  bufferTx[5] = PVTbuffer[32];	//(uint8_t) ((UBLOX_Handle.lon >> 8) & 0xFF);
	  bufferTx[6] = PVTbuffer[33];	//(uint8_t) ((UBLOX_Handle.lon >> 0) & 0xFF);

	  bufferTx[7] = PVTbuffer[34];	//(uint8_t) ((UBLOX_Handle.lat >> 24) & 0xFF);		//*** latitude
	  bufferTx[8] = PVTbuffer[35];	//(uint8_t) ((UBLOX_Handle.lat >> 16) & 0xFF);
	  bufferTx[9] = PVTbuffer[36];	//(uint8_t) ((UBLOX_Handle.lat >> 8) & 0xFF);
	  bufferTx[10] = PVTbuffer[37];	//(uint8_t) ((UBLOX_Handle.lat >> 0) & 0xFF);

//	  daylight_hour = PVTbuffer[14] + p_settings_phy->time_zone_hour;
//	  if(daylight_hour > 24) daylight_hour = daylight_hour - 24;
//	  (daylight_hour > 8)? (daylight_hour = daylight_hour - 8): (daylight_hour = 0);	//9-:_23
//	  bufferTx[11] = (daylight_hour << 4) +				//часы	  mask 0b00011111	  0-:-15
//	  	   	  	  	 ((PVTbuffer[15] & 0x3F) >> 2);		//минуты  mask 0b00111111
//	  bufferTx[12] = ((PVTbuffer[15] & 0x03) << 6) +	//минуты  mask 0b00000011
//			  	  	  (PVTbuffer[16] & 0x3F);			//секунды mask 0b00111111

	if(pp_devices_phy[p_settings_phy->device_number]->gps_speed > GPS_SPEED_THRS)
	{
		bufferTx[11] = ((pp_devices_phy[p_settings_phy->device_number]->gps_speed & 0x7F) << 1) +	// 0 - 128 km/h
					((pp_devices_phy[p_settings_phy->device_number]->gps_heading & 0x1FF) >> 8);	//mask 0b0000 0001 1111 1111
	  	bufferTx[12] = (pp_devices_phy[p_settings_phy->device_number]->gps_heading & 0xFF);			//mask 0b0000 0000 1111 1111
	}
	else bufferTx[12] = bufferTx[11] = 0;

	  Radio.Send(bufferTx, buffer_to_transmit); 	// to be filled by attendee BufferAirSize
}

void scan_channels(void)
{
	HAL_TIM_Base_Stop_IT(&htim1);
	  __HAL_TIM_SET_COUNTER(&htim1, 0);
	  __HAL_TIM_CLEAR_FLAG(&htim1, TIM_SR_UIF); // очищаем флаг
	fillScreen(BLACK);
	HAL_LPTIM_PWM_Start(&hlptim1, 16, brightness);
	while (1)//(GPIOA->IDR & BTN_2_Pin)		//wait for OK click to start cal
	{
	ST7735_SetRotation(0);
		sprintf(&Lines[0][0], "     TO SCAN");
		sprintf(&Lines[6][0], "    FREQUENCY");
		sprintf(&Lines[7][0], "     CHANNELS");
		sprintf(&Lines[8][0], "  Click ESC/DOWN");
		sprintf(&Lines[9][0], "        OR");
		sprintf(&Lines[10][0], " POWER FOR REBOOT");
	ST7735_WriteString(3, 33, &Lines[0][0], Font_7x10, YELLOW,BLACK);
	ST7735_WriteString(3, 44, &Lines[6][0], Font_7x10, YELLOW,BLACK);
	ST7735_WriteString(0, 55, &Lines[7][0], Font_7x10, YELLOW,BLACK);
	ST7735_WriteString(0, 77, &Lines[8][0], Font_7x10, GREEN,BLACK);
	ST7735_WriteString(0, 99, &Lines[9][0], Font_7x10, YELLOW,BLACK);
	ST7735_WriteString(3, 121, &Lines[10][0], Font_7x10, GREEN,BLACK);

		while (GPIOA->IDR & BTN_1_Pin)
		{
			if (!(GPIOA->IDR & BTN_3_Pin))	//ECS for scan
			{
				for(int8_t i = 0; i < ((FREQ_CHANNEL_LAST - FREQ_CHANNEL_FIRST)/5); i++)		// j < 13
				{
					rssi_by_channel[0][i] = -127;
				}
				fillScreen(BLACK);
//				sprintf(&Lines[0][0], "pressPWR to reboot");
//				sprintf(&Lines[1][0], "pressESC to rescan");
				for(int8_t i = 0; i < 7; i++)
				{
					HAL_TIM_Base_Start_IT(&htim1);
					HAL_Delay(700);
					HAL_TIM_Base_Stop_IT(&htim1);
					sprintf(&Lines[0][0], "channels = %02d", channel_ind);
//					for (uint8_t k = 0; k < 4; k++)
//					{
						ST7735_WriteString(0, 4+0*11, &Lines[0][0], Font_7x10, CYAN,BLACK);
//					}
					for (uint8_t j = 0; j < ((FREQ_CHANNEL_LAST - FREQ_CHANNEL_FIRST)/5); j++)		// j < 13
					{
						if(rssi_by_channel[0][j] < rssi_by_channel[1][j]) rssi_by_channel[0][j] = rssi_by_channel[1][j];
						sprintf(&Lines[j + 1][0], "Ch %02d RSSI %04ddBm", (j*5 + FREQ_CHANNEL_FIRST), rssi_by_channel[0][j]);

						if(rssi_by_channel[0][j] > -60) ST7735_WriteString(0, 4+(j+1)*11, &Lines[j+1][0], Font_7x10, RED,BLACK);
						else if(rssi_by_channel[0][j] < -80) ST7735_WriteString(0, 4+(j+1)*11, &Lines[j+1][0], Font_7x10, GREEN,BLACK);
						else ST7735_WriteString(0, 4+(j+1)*11, &Lines[j+1][0], Font_7x10, YELLOW,BLACK);
					}
					channel_ind = 0;
				}
			}
		}
		NVIC_SystemReset();
	}
}
/* USER CODE END PrFD */
