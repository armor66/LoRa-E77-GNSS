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
#include "bit_band.h"
//#include "stm32wle5xx.h"
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, HOLD_Pin|LED_B_Pin|RST_Pin|DC_Pin
                          |LED_G_Pin|LCD_BL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_W_Pin|RF_CTRL1_Pin|RF_CTRL2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PBPin PBPin PBPin PBPin */
  GPIO_InitStruct.Pin = HOLD_Pin|RST_Pin|DC_Pin|LCD_BL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin */
  GPIO_InitStruct.Pin = LED_B_Pin|LED_G_Pin;
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
  GPIO_InitStruct.Pin = BUZZ_IN_Pin|BTN_3_Pin|BTN_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = LED_W_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_W_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = BTN_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = LED_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_R_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin */
  GPIO_InitStruct.Pin = RF_CTRL1_Pin|RF_CTRL2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */
void interrupt_init(void)
{
	TIM1->SR &= ~TIM_SR_UIF;                //clear update interrupt
	TIM1->DIER |= TIM_DIER_UIE;             //update interrupt enable
    NVIC_EnableIRQ(TIM1_UP_IRQn);

    TIM2->SR &= ~TIM_SR_UIF;                //clear update interrupt
    TIM2->DIER |= TIM_DIER_UIE;         //update interrupt enable
   	NVIC_EnableIRQ(TIM2_IRQn);

    TIM16->SR &= ~TIM_SR_UIF;            //clear update interrupt
    TIM16->DIER |= TIM_DIER_UIE;         //update interrupt enable
   	NVIC_EnableIRQ(TIM16_IRQn);

//PA0 - BUZZ_IN
//    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI2_PB;	//exti 2 source is port B
//    EXTI->RTSR |= EXTI_RTSR_TR2;				//interrupt 2 on rising edge
//    EXTI->IMR1 |= EXTI_IMR1_IM8;					//unmask interrupt 2
    NVIC_EnableIRQ(EXTI0_IRQn);             	//enable interrupt

//PC1 - PWR button
//    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI3_PB;	//exti 3 source is port B
//    EXTI->FTSR |= EXTI_FTSR_TR3;				//interrupt 3 on falling edge
    NVIC_EnableIRQ(EXTI1_IRQn);             	//enable interrupt

//PA2 - UP/OK button
//    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI4_PB;	//exti 4 source is port B
//    EXTI->FTSR |= EXTI_FTSR_TR4;				//interrupt 4 on falling edge
    NVIC_EnableIRQ(EXTI2_IRQn);             	//enable interrupt

//PA3 - DOWN/ESC button
//    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI5_PB;   //exti 5 source is port B
//    EXTI->FTSR |= EXTI_FTSR_TR5;                //interrupt 5 on falling edge
    NVIC_EnableIRQ(EXTI3_IRQn);               //enable interrupt

//PB - PPS interrupt on rising edge
    NVIC_EnableIRQ(EXTI4_IRQn);               //enable interrupt
//
//    EXTI->PR1 = (uint32_t)0x0007FFFF;            //clear all pending interrupts

    /* DMA1_Channel1_IRQn interrupt configuration */
//    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void enable_buttons_interrupts(void)
{
	BIT_BAND_PERI(EXTI->IMR1, EXTI_IMR1_IM1) = 1;		//unmask interrupt
//	EXTI->IMR1 |= EXTI_IMR1_IM5;
	BIT_BAND_PERI(EXTI->IMR1, EXTI_IMR1_IM2) = 1;		//unmask interrupt
//	EXTI->IMR1 |= EXTI_IMR1_IM1;
	BIT_BAND_PERI(EXTI->IMR1, EXTI_IMR1_IM3) = 1;		//unmask interrupt
//	EXTI->IMR1 |= EXTI_IMR1_IM0;
}

void disable_buttons_interrupts(void)
{
	BIT_BAND_PERI(EXTI->IMR1, EXTI_IMR1_IM1) = 0;		//mask interrupt
//	EXTI->IMR1 &= ~EXTI_IMR1_IM5;
	BIT_BAND_PERI(EXTI->IMR1, EXTI_IMR1_IM2) = 0;		//mask interrupt
//	EXTI->IMR1 &= ~EXTI_IMR1_IM1;
	BIT_BAND_PERI(EXTI->IMR1, EXTI_IMR1_IM3) = 0;		//mask interrupt
//	EXTI->IMR1 &= ~EXTI_IMR1_IM0;
}

void led_toggle(void){
	GPIOC->ODR ^= GPIO_ODR_OD0;
}
//PC0
void led_red_on(void){
	GPIOC->BSRR = GPIO_BSRR_BR0;
}
void led_red_off(void){
	GPIOC->BSRR = GPIO_BSRR_BS0;
}
//PB5
void led_green_on(void){
	GPIOB->BSRR = GPIO_BSRR_BR5;
}
void led_green_off(void){
	GPIOB->BSRR = GPIO_BSRR_BS5;
}
//PB3
void led_blue_on(void){
	GPIOB->BSRR = GPIO_BSRR_BR3;
}
void led_blue_off(void){
	GPIOB->BSRR = GPIO_BSRR_BS3;
}
//PA9
void led_w_on(void){
	GPIOA->BSRR = GPIO_BSRR_BS9;
}
void led_w_off(void){
	GPIOA->BSRR = GPIO_BSRR_BR9;
}
//PB10
void lcd_on(void){
	LCD_BL_GPIO_Port->BSRR = LCD_BL_Pin;
}
void lcd_off(void){
	LCD_BL_GPIO_Port->BSRR = (LCD_BL_Pin << 16);
}
//PB15
void hold_power(void) {
    GPIOB->BSRR = GPIO_BSRR_BS15;
}
void release_power(void) {
    GPIOB->BSRR = GPIO_BSRR_BR15;
}

/* USER CODE END 2 */
