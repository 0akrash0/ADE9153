/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_NUM_CH          6U
#define ADC_AVG_SAMPLES     16U
#define MPPT_INTERVAL_MS    100U

/* TIM1 ARR = 2666; limits keep duty between 5 % and 95 % */
#define PWM_DUTY_MIN        134U
#define PWM_DUTY_MAX        2533U
#define PWM_DUTY_STEP       27U     /* ~1 % perturbation per MPPT iteration */

/* ADC reference and full-scale */
#define VREF_MV             3300U
#define ADC_MAX             4095U

/*
 * Hardware scaling — MUST be calibrated to your actual circuit:
 *
 *   Voltage pins use a resistor divider: Vreal = Vadc_mv * SCALE
 *     ARRAY_V_SCALE 10  ->  10 kOhm + 100 kOhm divider (Vadc = Vreal / 10)
 *     BATT_V_SCALE  5   ->  10 kOhm + 47  kOhm divider (Vadc = Vreal / 5)
 *
 *   Current pins: Ireal_ma = Vadc_mv * 1000 / I_MV_PER_A
 *     500 mV/A  ->  e.g. 0.1 Ohm shunt + 5 V/V gain
 */
#define ARRAY_V_SCALE       10U
#define BATT_V_SCALE        5U
#define LOAD_V_SCALE        5U
#define ARRAY_I_MV_PER_A    500U
#define LOAD_I_MV_PER_A     500U

/* Battery protection thresholds (mV) -- defaults for 12 V lead-acid */
#define BATT_OVP_MV         14400U
#define BATT_UVP_MV         11000U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* DMA circular buffer: 16 complete scans x 6 channels = 96 samples */
volatile uint16_t adc_dma_buf[ADC_NUM_CH * ADC_AVG_SAMPLES];
extern volatile uint32_t tick_ms;  /* incremented by SysTick_Handler in stm32g0xx_it.c */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Average one ADC channel across all samples in the circular DMA buffer.
 * Channel indices follow fixed forward-scan order:
 *   0=ARRAY_V  1=ARRAY_I  2=LOAD_V  3=LOAD_I  4=BATT_V  5=HYB_V        */
static uint32_t adc_avg(uint8_t ch)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < ADC_AVG_SAMPLES; i++)
        sum += adc_dma_buf[(uint16_t)i * ADC_NUM_CH + ch];
    return sum / ADC_AVG_SAMPLES;
}

static uint32_t raw_to_mv(uint32_t raw)
{
    return (raw * VREF_MV + ADC_MAX / 2U) / ADC_MAX;
}

static void pwm_set_duty(int32_t duty)
{
    if (duty > (int32_t)PWM_DUTY_MAX) duty = (int32_t)PWM_DUTY_MAX;
    if (duty < (int32_t)PWM_DUTY_MIN) duty = (int32_t)PWM_DUTY_MIN;
    LL_TIM_OC_SetCompareCH1(TIM1, (uint32_t)duty);
}

/*
 * Perturb-and-Observe MPPT -- call every MPPT_INTERVAL_MS.
 *
 * Controls TIM1 CH1/CH1N duty cycle to maximise P = V_array x I_array.
 * Battery OVP overrides MPPT and reduces duty; UVP disconnects the load.
 */
static void mppt_run(void)
{
    static uint32_t prev_power = 0;
    static int8_t   direction  = 1;

    uint32_t array_v_mv = raw_to_mv(adc_avg(0)) * ARRAY_V_SCALE;
    uint32_t array_i_ma = raw_to_mv(adc_avg(1)) * 1000U / ARRAY_I_MV_PER_A;
    uint32_t batt_v_mv  = raw_to_mv(adc_avg(4)) * BATT_V_SCALE;

    /* Battery overvoltage: reduce duty to lower charge current */
    if (batt_v_mv >= BATT_OVP_MV)
    {
        pwm_set_duty((int32_t)LL_TIM_OC_GetCompareCH1(TIM1) - (int32_t)PWM_DUTY_STEP);
        prev_power = 0;
        return;
    }

    /* Battery undervoltage: disconnect load to protect battery */
    if (batt_v_mv <= BATT_UVP_MV)
        LL_GPIO_ResetOutputPin(LOAD_GPIO_OUT_GPIO_Port, LOAD_GPIO_OUT_Pin);
    else
        LL_GPIO_SetOutputPin(LOAD_GPIO_OUT_GPIO_Port, LOAD_GPIO_OUT_Pin);

    /* Power in units of 100 uW -- safe for uint32_t up to ~430 W */
    uint32_t power = (array_v_mv / 10U) * (array_i_ma / 10U);
    int32_t  duty  = (int32_t)LL_TIM_OC_GetCompareCH1(TIM1);

    if (power >= prev_power)
        duty += (int32_t)direction * (int32_t)PWM_DUTY_STEP;
    else
    {
        direction = -direction;
        duty += (int32_t)direction * (int32_t)PWM_DUTY_STEP;
    }

    pwm_set_duty(duty);
    prev_power = power;
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
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, 3);

  /** Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
  */
  LL_SYSCFG_DisableDBATT(LL_SYSCFG_UCPD1_STROBE | LL_SYSCFG_UCPD2_STROBE);

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_IWDG_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

  /* Wire DMA channel 3 to ADC1 DR and our averaging buffer */
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)adc_dma_buf);
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_3,
      LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA));
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, ADC_NUM_CH * ADC_AVG_SAMPLES);
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

  /* ADC self-calibration must run before Enable */
  LL_ADC_StartCalibration(ADC1);
  while (LL_ADC_IsCalibrationOnGoing(ADC1)) {}

  LL_ADC_Enable(ADC1);
  while (!LL_ADC_IsActiveFlag_ADRDY(ADC1)) {}
  LL_ADC_REG_StartConversion(ADC1);

  /* TIM3 TRGO triggers ADC at ~10 kHz */
  LL_TIM_EnableCounter(TIM3);

  /* TIM1: start complementary half-bridge PWM at minimum duty */
  pwm_set_duty(PWM_DUTY_MIN);
  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH1N);
  LL_TIM_EnableAllOutputs(TIM1);
  LL_TIM_EnableCounter(TIM1);

  /* Enable solar array input relay/switch */
  LL_GPIO_SetOutputPin(ARRAY_GPIO_OUT_GPIO_Port, ARRAY_GPIO_OUT_Pin);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t last_mppt_ms = 0U;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint32_t now = tick_ms;
    if ((now - last_mppt_ms) >= MPPT_INTERVAL_MS)
    {
      last_mppt_ms = now;
      mppt_run();
    }
    LL_IWDG_ReloadCounter(IWDG);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2)
  {
  }

  /* HSI configuration and activation */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* LSI configuration and activation */
  LL_RCC_LSI_Enable();
  while(LL_RCC_LSI_IsReady() != 1)
  {
  }

  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 8, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* Sysclk activation on the main PLL */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Set APB1 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_Init1msTick(64000000);
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(64000000);
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

  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
