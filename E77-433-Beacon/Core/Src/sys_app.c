/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sys_app.c
  * @author  MCD Application Team
  * @brief   Initializes HW and SW system entities (not related to the radio)
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
#include <stdio.h>
#include "platform.h"
#include "sys_app.h"
#include "stm32_seq.h"
#include "stm32_systime.h"
#include "stm32_lpm.h"
#include "timer_if.h"
#include "utilities_def.h"

/* USER CODE BEGIN Includes */
#include "adc_if.h"
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LORAWAN_MAX_BAT   254
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static uint8_t SYS_TimerInitialisedFlag = 0;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */


/* USER CODE END PFP */

/* Exported functions ---------------------------------------------------------*/
void SystemApp_Init(void)
{
  /* USER CODE BEGIN SystemApp_Init_1 */
	__HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);

	  /*Initialize timer and RTC*/
	  UTIL_TIMER_Init();
	  SYS_TimerInitialisedFlag = 1;
  /* USER CODE END SystemApp_Init_1 */

}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private functions ---------------------------------------------------------*/
/* USER CODE BEGIN PrFD */

uint8_t GetBatteryLevel(void)
{
  uint8_t batteryLevel = 0;
  uint16_t batteryLevelmV;

  /* USER CODE BEGIN GetBatteryLevel_0 */

  /* USER CODE END GetBatteryLevel_0 */

  batteryLevelmV = (uint16_t)SYS_GetBatteryLevel();

  /* Convert batterey level from mV to linea scale: 1 (very low) to 254 (fully charged) */
/*  if (batteryLevelmV > VDD_BAT)
  {
    batteryLevel = LORAWAN_MAX_BAT;
  }
  else if (batteryLevelmV < VDD_MIN)
  {
    batteryLevel = 0;
  }
  else
  {*/
	  batteryLevel = ((batteryLevelmV - 2700) / 10);		//0...150  2700 instead of VDD_MIN
//    batteryLevel = (((uint32_t)(batteryLevelmV - VDD_MIN) * LORAWAN_MAX_BAT) / (VDD_BAT - VDD_MIN));
//  }

//  APP_LOG(TS_OFF, VLEVEL_M, "VDDA: %d\r\n", batteryLevel);

  /* USER CODE BEGIN GetBatteryLevel_2 */
  //batteryLevel = ((batteryLevelmV - VDD_MIN) / 100);
  /* USER CODE END GetBatteryLevel_2 */

  return batteryLevel;  /* 1 (very low) to 254 (fully charged) */
}

uint16_t GetTemperatureLevel(void)
{
  int16_t temperatureLevel = 0;

  temperatureLevel = (int16_t)(SYS_GetTemperatureLevel() / 256);
  /* USER CODE BEGIN GetTemperatureLevel */
//  APP_LOG(TS_OFF, VLEVEL_M, "CoreTemp: %d'C\r\n", temperatureLevel);
  /* USER CODE END GetTemperatureLevel */
  return temperatureLevel;
}

/* USER CODE END PrFD */

/* HAL overload functions ---------------------------------------------------------*/

/* Set #if 0 if you want to keep the default HAL instead overcharge them*/
/* USER CODE BEGIN Overload_HAL_weaks_1 */
/* USER CODE END Overload_HAL_weaks_1 */

/**
  * @note This function overwrites the __weak one from HAL
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  /*Don't enable SysTick if TIMER_IF is based on other counters (e.g. RTC) */
  /* USER CODE BEGIN HAL_InitTick_1 */

  /* USER CODE END HAL_InitTick_1 */
  return HAL_OK;
  /* USER CODE BEGIN HAL_InitTick_2 */

  /* USER CODE END HAL_InitTick_2 */
}

/* USER CODE BEGIN Overload_HAL_weaks_1a */

/* USER CODE END Overload_HAL_weaks_1a */

/**
  * @note This function overwrites the __weak one from HAL
  */
uint32_t HAL_GetTick(void)
{
  uint32_t ret = 0;
  /* TIMER_IF can be based on other counter the SysTick e.g. RTC */
  /* USER CODE BEGIN HAL_GetTick_1 */

  /* USER CODE END HAL_GetTick_1 */
  if (SYS_TimerInitialisedFlag == 0)
  {
    /* TIMER_IF_GetTimerValue should be used only once UTIL_TIMER_Init() is initialized */
    /* If HAL_Delay or a TIMEOUT countdown is necessary during initialization phase */
    /* please use temporarily another timebase source (SysTick or TIMx), which implies also */
    /* to rework the above function HAL_InitTick() and to call HAL_IncTick() on the timebase IRQ */
    /* Note: when TIMER_IF is based on RTC, stm32wlxx_hal_rtc.c calls this function before TimeServer is functional */
    /* RTC TIMEOUT will not expire, i.e. if RTC has an hw problem it will keep looping in the RTC_Init function */
    /* USER CODE BEGIN HAL_GetTick_EarlyCall */

    /* USER CODE END HAL_GetTick_EarlyCall */
  }
  else
  {
    ret = TIMER_IF_GetTimerValue();
  }
  /* USER CODE BEGIN HAL_GetTick_2 */

  /* USER CODE END HAL_GetTick_2 */
  return ret;
}

/**
  * @note This function overwrites the __weak one from HAL
  */
void HAL_Delay(__IO uint32_t Delay)
{
  /* TIMER_IF can be based on other counter the SysTick e.g. RTC */
  /* USER CODE BEGIN HAL_Delay_1 */

  /* USER CODE END HAL_Delay_1 */
  TIMER_IF_DelayMs(Delay);
  /* USER CODE BEGIN HAL_Delay_2 */

  /* USER CODE END HAL_Delay_2 */
}

/* USER CODE BEGIN Overload_HAL_weaks_2 */

/* USER CODE END Overload_HAL_weaks_2 */
