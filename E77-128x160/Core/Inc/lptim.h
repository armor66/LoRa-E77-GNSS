/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lptim.h
  * @brief   This file contains all the function prototypes for
  *          the lptim.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LPTIM_H__
#define __LPTIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

//extern LPTIM_HandleTypeDef hlptim1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_LPTIM1_Init(void);

/* USER CODE BEGIN Prototypes */
void lptim1_start(uint8_t period, uint8_t pulse);
void lptim1_stop(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __LPTIM_H__ */

