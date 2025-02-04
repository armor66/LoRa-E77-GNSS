/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
void interrupt_init(void);
void enable_buttons_interrupts(void);
void disable_buttons_interrupts(void);

void led_toggle(void);
void led_red_on(void);
void led_red_off(void);
void led_green_on(void);
void led_green_off(void);
void led_blue_on(void);
void led_blue_off(void);

void led_w_on(void);
void led_w_off(void);

void lcd_on(void);
void lcd_off(void);

void gps_enable(void);
void gps_disable(void);

void hold_power(void);
void release_power(void);

void longBeepsBlocking(int8_t beeps);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

