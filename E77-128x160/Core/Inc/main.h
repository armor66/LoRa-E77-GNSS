/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wlxx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32wle5xx.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RTC_PREDIV_A ((1<<(15-RTC_N_PREDIV_S))-1)
#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S ((1<<RTC_N_PREDIV_S)-1)
#define LED_B_Pin GPIO_PIN_3
#define LED_B_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_4
#define LED_G_GPIO_Port GPIOB
#define LED_R_Pin GPIO_PIN_5
#define LED_R_GPIO_Port GPIOB
#define PPS_Pin GPIO_PIN_8
#define PPS_GPIO_Port GPIOB
#define BTN_3_Pin GPIO_PIN_0
#define BTN_3_GPIO_Port GPIOA
#define BTN_2_Pin GPIO_PIN_1
#define BTN_2_GPIO_Port GPIOA
#define BTN_1_Pin GPIO_PIN_4
#define BTN_1_GPIO_Port GPIOA
#define MUTE_Pin GPIO_PIN_5
#define MUTE_GPIO_Port GPIOA
#define RF_CTRL1_Pin GPIO_PIN_6
#define RF_CTRL1_GPIO_Port GPIOA
#define RF_CTRL2_Pin GPIO_PIN_7
#define RF_CTRL2_GPIO_Port GPIOA
#define CS_Pin GPIO_PIN_9
#define CS_GPIO_Port GPIOA
#define LED_W_Pin GPIO_PIN_12
#define LED_W_GPIO_Port GPIOB
#define DC_Pin GPIO_PIN_11
#define DC_GPIO_Port GPIOA
#define RST_Pin GPIO_PIN_12
#define RST_GPIO_Port GPIOA
#define HOLD_Pin GPIO_PIN_13
#define HOLD_GPIO_Port GPIOC
#define BATT_Pin GPIO_PIN_15
#define BATT_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
//#define NV3023
#define ST7735
//#define ST7735_IS_160X128_BLUE
//#define ST7735_IS_160X128_RED
#define BUFFER_AIR_SIZE            	13  	/* Define the payload size here */
#define BUFFER_RX                  	15		//BUFFER_SIZE + 2 rssi and snr
#define PPS_SKIP	(2)		//how many first PPS pulses are skipped before switching to active mode

void shortBeeps(int8_t beeps);
uint32_t get_abs_pps_cntr(void);

struct main_flags_struct
{
    int8_t permit_actions;
    int8_t processing_button;
    int8_t buttons_scanned;
    int8_t button_code;

    int8_t update_screen;
    int8_t display_status;
    int8_t time_slot;
    int8_t time_slot_timer_ovf;

    int8_t long_beeps;
    int8_t long_beeps_flag;
    int8_t short_beeps;
    int8_t short_beeps_flag;
    int8_t opt_short_beeps;
    int8_t opt_long_beeps;
    int8_t brightness;
    uint16_t endRX_2_TX;

    int8_t nav_pvt_ram_flag;
    int8_t out_ubx_ram_flag;
    int8_t out_nmea_ram_flag;
    int8_t ubx_hwVersion;

    int8_t GPScheckFlag;
    int8_t GPSconfigureFlag;
    int8_t GPScold_restarted;
    int8_t find_nearest_trekpoint_flag;

	int8_t scanRadioFlag;
    int8_t transmit_iq_inverted_flag;
    int8_t calibrateCompassFlag;
   	int8_t compass_fault;

    int8_t fix_valid;
    int8_t first_time_locked;
    int8_t pps_counter;
    int8_t pps_synced;
    int8_t current_point_group;
//    int8_t pattern_started;

    int8_t adc_calibration_factor;
    uint8_t uartIdx;
    uint8_t settings_index;

    uint32_t settings_address;
};
//struct main_flags_struct *get_main_flags(void);
extern struct main_flags_struct main_flags;

void shortBeeps(int8_t beeps);
void longBeeps(int8_t beeps);

extern uint8_t PVTbuffer[];
extern uint8_t bufferRx[];
extern uint8_t points_array[];
extern char string_buffer[20][27];

//extern uint8_t gps_speed;		//subghz_phy_app.c
//extern int16_t gps_heading;		//subghz_phy_app.c
extern int16_t heading_deg;			//compass.c
extern double heading_rad;

extern double distance[];
extern double arc_length[];
extern int16_t azimuth_deg_signed[];
extern int16_t azimuth_deg_unsigned[];
extern double azimuth_rad[];

extern uint16_t endRX_2_TX;
extern int8_t RssiValue;
extern int8_t SnrValue;
extern int8_t actual_menu;
extern char *region[];
extern char *fixType[];

extern int16_t x;
extern int16_t y;
extern double comp_x;
extern double comp_y;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
