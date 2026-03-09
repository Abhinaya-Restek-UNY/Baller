/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PWM1_Pin GPIO_PIN_5
#define PWM1_GPIO_Port GPIOE
#define PWM2_Pin GPIO_PIN_6
#define PWM2_GPIO_Port GPIOE
#define ENC1A_Pin GPIO_PIN_0
#define ENC1A_GPIO_Port GPIOA
#define ENC1B_Pin GPIO_PIN_1
#define ENC1B_GPIO_Port GPIOA
#define Motor1B_Pin GPIO_PIN_4
#define Motor1B_GPIO_Port GPIOA
#define Motor1A_Pin GPIO_PIN_5
#define Motor1A_GPIO_Port GPIOA
#define Motor2B_Pin GPIO_PIN_6
#define Motor2B_GPIO_Port GPIOA
#define Motor2A_Pin GPIO_PIN_7
#define Motor2A_GPIO_Port GPIOA
#define Motor3A_Pin GPIO_PIN_4
#define Motor3A_GPIO_Port GPIOC
#define Motor3B_Pin GPIO_PIN_5
#define Motor3B_GPIO_Port GPIOC
#define Motor4B_Pin GPIO_PIN_0
#define Motor4B_GPIO_Port GPIOB
#define Motor4A_Pin GPIO_PIN_2
#define Motor4A_GPIO_Port GPIOB
#define PWM4_Pin GPIO_PIN_15
#define PWM4_GPIO_Port GPIOB
#define ENC3A_Pin GPIO_PIN_12
#define ENC3A_GPIO_Port GPIOD
#define ENC3B_Pin GPIO_PIN_13
#define ENC3B_GPIO_Port GPIOD
#define ENC2A_Pin GPIO_PIN_4
#define ENC2A_GPIO_Port GPIOB
#define ENC2B_Pin GPIO_PIN_5
#define ENC2B_GPIO_Port GPIOB
#define PWM3_Pin GPIO_PIN_9
#define PWM3_GPIO_Port GPIOB
#define LED_BUILTIN_Pin GPIO_PIN_1
#define LED_BUILTIN_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
