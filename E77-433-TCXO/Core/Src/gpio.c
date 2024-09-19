/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_B_Pin|LED_G_Pin|LED_R_Pin|LED_W_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPS_EN_GPIO_Port, GPS_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RF_CTRL2_Pin|RF_CTRL1_Pin|CS_Pin|DC_Pin
                          |RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(HOLD_GPIO_Port, HOLD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PBPin PBPin PBPin PBPin */
  GPIO_InitStruct.Pin = LED_B_Pin|LED_G_Pin|LED_R_Pin|LED_W_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = PPS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(PPS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin PAPin */
  GPIO_InitStruct.Pin = BTN_3_Pin|BTN_2_Pin|BTN_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = GPS_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;		//NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPS_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin */
  GPIO_InitStruct.Pin = RF_CTRL2_Pin|RF_CTRL1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin PAPin */
  GPIO_InitStruct.Pin = CS_Pin|DC_Pin|RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = HOLD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HOLD_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 2 */

void led_toggle(void){
//GPIOB->ODR ^= GPIO_ODR_OD4;
GPIOB->ODR ^= GPIO_ODR_OD5;
}
void led_red_on(void){
	GPIOB->BSRR = GPIO_BSRR_BR5;	//gnd for custom pcb
}
void led_red_off(void){
	GPIOB->BSRR = GPIO_BSRR_BS5;
}
void hold_power(void) {
    GPIOC->BSRR = GPIO_BSRR_BS13;	//+
}
void release_power(void) {
    GPIOC->BSRR = GPIO_BSRR_BR13;	//gnd
}

void led_green_on(void){
	GPIOB->BSRR = GPIO_BSRR_BR4;	//gnd for custom pcb
}
void led_green_off(void){
	GPIOB->BSRR = GPIO_BSRR_BS4;
}
void led_blue_on(void){
	GPIOB->BSRR = GPIO_BSRR_BR3;	//gnd for custom pcb
}
void led_blue_off(void){
	GPIOB->BSRR = GPIO_BSRR_BS3;
}
void led_w_on(void){
	GPIOB->BSRR = GPIO_BSRR_BS12;	//+
//	GPIOA->BSRR = GPIO_BSRR_BS5;
}
void led_w_off(void){
	GPIOB->BSRR = GPIO_BSRR_BR12;
//	GPIOA->BSRR = GPIO_BSRR_BR5;
}
void gps_enable(void){
	GPIOA->BSRR = GPIO_BSRR_BS5;	//+
}
void gps_disable(void){
	GPIOA->BSRR = GPIO_BSRR_BR5;	//gnd
}
void buzz_on(void){					//same as gps_enable
	GPIOA->BSRR = GPIO_BSRR_BS5;	//+
}
void buzz_off(void){				//same as gps_disable
	GPIOA->BSRR = GPIO_BSRR_BR5;	//gnd
}

void shortBeepsBlocking(int8_t beeps)
{
	for(int8_t i = 0; i < beeps; i++)
	{
		HAL_Delay(100);
		led_w_on();
		HAL_Delay(10);
		led_w_off();
	}
}
void longBeepsBlocking(int8_t beeps)
{
	for(int8_t i = 0; i < beeps; i++)
	{
		HAL_Delay(100);
		led_w_on();
		HAL_Delay(500);
		led_w_off();
	}
}
/* USER CODE END 2 */
