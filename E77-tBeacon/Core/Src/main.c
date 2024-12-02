/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "iwdg.h"
#include "lptim.h"
#include "spi.h"
#include "app_subghz_phy.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "stm32wlxx_nucleo_radio.h"
#include "sys_app.h"
#include "menu.h"
#include "ST7735.h"
//#include <stdio.h>			//for 'sprintf'
#include "subghz_phy_app.h"
#include "lrns.h"
//#include "compass.h"
#include "buttons.h"
#include "gnss.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
struct main_flags_struct main_flags = {0};
void shortBeeps(int8_t beeps)
{
	main_flags.short_beeps = beeps;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
//!!--comment MX_SubGHz_Phy_Init(); with SystemApp_Init(); line after reconfiguration--!!
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  SystemApp_Init();		//MX_SubGHz_Phy_Init();

  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
//  MX_LPTIM1_Init();

  MX_ADC_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
//  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Stop_IT(&htim1);
  __HAL_TIM_SET_COUNTER(&htim1, 0);
  __HAL_TIM_CLEAR_FLAG(&htim1, TIM_SR_UIF); // очищаем флаг
  HAL_TIM_Base_Stop_IT(&htim2);
  HAL_TIM_Base_Stop_IT(&htim16);		//starts onRxDone if no PPS
  disable_buttons_interrupts();

  EXTI->IMR1 &= ~EXTI_IMR1_IM4;			//interrupt disabled on PPS front

  hold_power();
  led_w_off();
  led_red_off();
  led_green_off();
  led_blue_on();

    if(!(RCC->CSR & RCC_CSR_SFTRSTF))	// || !(RCC->CSR & RCC_CSR_IWDGRSTF))	//if the reset is not caused by software (save & restart after settings changed)
    {																		//or not independent watchdog reset occurs
	   release_power();					//initially set off position
	   HAL_Delay(1000); 				//startup delay ~2sec
	   NVIC_SystemReset();
	}else								//if Software reset or Independent watchdog reset occurred
	{
		RCC->CSR |= RCC_CSR_RMVF;		//clear the reset flags IWDGRSTF, SFTRSTF by writing bit to the RMVF
	    led_w_on();
	}

   HAL_Delay(10);
   led_w_off();

  settings_load();
  init_lrns();

  ST7735_Init(0);
  fillScreen(BLACK);

  	if(!(GPIOA->IDR & BTN_3_Pin) && (GPIOA->IDR & BTN_2_Pin))
	{
  		main_flags.GPSconfigureFlag = 1;	//if DOWN button is pressed and  OK button is released upon power up
		init_gnss();
	}
  	else if(!(GPIOA->IDR & BTN_2_Pin) && (GPIOA->IDR & BTN_3_Pin))
	{
		scanRadioFlag = 1;		//if OK button is pressed and DOWN button is released upon power up
		SubghzApp_Init();
	}
  	else if(!(GPIOA->IDR & BTN_2_Pin) && !(GPIOA->IDR & BTN_3_Pin))
	{
//		calibrateCompassFlag = 1;//if both buttons is pressed
//		init_compass();
	}
  	else
	{
//		init_compass();
		SubghzApp_Init();

//		for(uint8_t i = 1; i < (DEVICES_ON_AIR_MAX+1); i++)
//		{
//			lost_device_load(i);
//		}
//		for(uint8_t i = 0; i < MEMORY_POINT_GROUPS; i++)
//		{
//			saved_group_load(i);
//		}

		init_gnss();
		enable_buttons_interrupts();

		EXTI->IMR1 |= EXTI_IMR1_IM4;				//interrupt enabled on PPS front
		USART2->CR1 &= ~USART_CR1_RXNEIE_RXFNEIE;

		init_menu();
		main_flags.update_screen = 1;
//		HAL_LPTIM_PWM_Start(&hlptim1, 16, brightness);
		HAL_TIM_Base_Start_IT(&htim1);
//	    MX_IWDG_Init();
	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  	while (1)
  	{
    /* USER CODE END WHILE */
//    MX_SubGHz_Phy_Process();

    /* USER CODE BEGIN 3 */
//restrict buttons actions when data is transmitting, permit_actions always on case 3 (radio tx or rx finished)
  		if((main_flags.buttons_scanned) && (main_flags.permit_actions))
  		{
  			change_menu(button_code);
  			main_flags.buttons_scanned = 0;
  			main_flags.update_screen = 1;
  		}

  		if (main_flags.update_screen)	//buttons processed or on case 3 or if no PPS signal
  		{
//  	  	HAL_IWDG_Refresh(&hiwdg);
  			draw_current_menu();
  			main_flags.update_screen = 0;
  		}
  		__NOP();
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_PWR;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
  RCC_OscInitStruct.HSEDiv = RCC_HSE_DIV1;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 6;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
