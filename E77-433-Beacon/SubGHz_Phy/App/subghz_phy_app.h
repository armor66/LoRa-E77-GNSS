/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    subghz_phy_app.h
  * @author  MCD Application Team
  * @brief   Header of application of the SubGHz_Phy Middleware
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
#ifndef __SUBGHZ_PHY_APP_H__
#define __SUBGHZ_PHY_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

/* USER CODE BEGIN EC */
#define LORA_BANDWIDTH                              0         /* [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
//#define LORA_SPREADING_FACTOR                       11         /* [SF7..SF12] */
//#define LORA_CODINGRATE                             2         /* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define LORA_PREAMBLE_LENGTH                        8         /* Same for Tx and Rx */
#define LORA_SYMBOL_TIMEOUT                         5         /* Symbols */
#define LORA_FIX_LENGTH_PAYLOAD_ON                  true	//TX_Start->OnRxDone ==(727...729)mS false==820mS
//#define LORA_IQ_INVERSION	                        false

/* USER CODE BEGIN EC */
//#define RX_TIMEOUT_VALUE                            2000
#define TX_TIMEOUT_VALUE                            750		//3000
#define BUFFER_AIR_SIZE                             13  /* Define the payload size here */
#define BUFFER_RX                                 	15	//BUFFER_SIZE + 2 rssi and snr
//#define TX_PERIOD_MS                                10000  /* App TX duty cycle */

#define TCXO_WORKAROUND_TIME_MARGIN                 50  /* 50ms margin */

#define RSSI_SENSING_TIME                           10   /* [ms] */
//#define CS_BACKOFF_TIME_UNIT                        200   /* [ms] */

#define NODES										6

#define WATCHDOG_RX_PERIOD						    60  /* [s] */

//#define DEFAULT_TEMPERATURE                      	(-100.0f)
//#define DEFAULT_HUMIDITY                      		(-100.0f)
/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief  Init Subghz Application
  */
void SubghzApp_Init(void);

/* USER CODE BEGIN EFP */
typedef enum
{
  RX_DONE,
  RX_TO,
  RX_ERR,
  TX_START,
  TX_DONE,
  RX_START,
  TX_TO,
} States_t;
//void Sensor_Process(void);
void transmit_data(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*__SUBGHZ_PHY_APP_H__*/
