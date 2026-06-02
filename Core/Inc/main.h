/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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

#include "stm32g0xx_ll_adc.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_iwdg.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_exti.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_utils.h"
#include "stm32g0xx_ll_pwr.h"
#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_usart.h"
#include "stm32g0xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BAT_LED_GPIO_OUT_Pin LL_GPIO_PIN_9
#define BAT_LED_GPIO_OUT_GPIO_Port GPIOB
#define CHARGING_LED_GPIO_OUT_Pin LL_GPIO_PIN_14
#define CHARGING_LED_GPIO_OUT_GPIO_Port GPIOC
#define PIR_SENSOR_GPIO_IN_Pin LL_GPIO_PIN_15
#define PIR_SENSOR_GPIO_IN_GPIO_Port GPIOC
#define ARRAY_V_ADC_Pin LL_GPIO_PIN_0
#define ARRAY_V_ADC_GPIO_Port GPIOA
#define ARRAY_I_ADC_Pin LL_GPIO_PIN_1
#define ARRAY_I_ADC_GPIO_Port GPIOA
#define DEBUG_UART2_TX_Pin LL_GPIO_PIN_2
#define DEBUG_UART2_TX_GPIO_Port GPIOA
#define DEBUG_UART_RX_Pin LL_GPIO_PIN_3
#define DEBUG_UART_RX_GPIO_Port GPIOA
#define LOAD_V_ADC_Pin LL_GPIO_PIN_4
#define LOAD_V_ADC_GPIO_Port GPIOA
#define LOAD_I_ADC_Pin LL_GPIO_PIN_5
#define LOAD_I_ADC_GPIO_Port GPIOA
#define PS_PWM_TIM16_Pin LL_GPIO_PIN_6
#define PS_PWM_TIM16_GPIO_Port GPIOA
#define PWM_TIM1_CH1N_Pin LL_GPIO_PIN_7
#define PWM_TIM1_CH1N_GPIO_Port GPIOA
#define BATT_V_ADC_Pin LL_GPIO_PIN_0
#define BATT_V_ADC_GPIO_Port GPIOB
#define HYB_V_ADC_Pin LL_GPIO_PIN_1
#define HYB_V_ADC_GPIO_Port GPIOB
#define PWM_TIM1_CH1_Pin LL_GPIO_PIN_8
#define PWM_TIM1_CH1_GPIO_Port GPIOA
#define LOAD_GPIO_OUT_Pin LL_GPIO_PIN_11
#define LOAD_GPIO_OUT_GPIO_Port GPIOA
#define HYBRID_GPIO_OUT_Pin LL_GPIO_PIN_15
#define HYBRID_GPIO_OUT_GPIO_Port GPIOA
#define ARRAY_GPIO_OUT_Pin LL_GPIO_PIN_4
#define ARRAY_GPIO_OUT_GPIO_Port GPIOB
#define LED_A_GPIO_OUT_Pin LL_GPIO_PIN_5
#define LED_A_GPIO_OUT_GPIO_Port GPIOB
#define LED_B_GPIO_OUT_Pin LL_GPIO_PIN_6
#define LED_B_GPIO_OUT_GPIO_Port GPIOB
#define LED_C_GPIO_OUT_Pin LL_GPIO_PIN_7
#define LED_C_GPIO_OUT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
