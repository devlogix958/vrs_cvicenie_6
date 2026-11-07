/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "hts221.h"
#include "lps25hb.h"
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
static const HTS221_Interface_t hts221_interface =
{
	i2c_master_read_byte,
	i2c_master_write_byte,
	i2c_master_read_multi,
	i2c_master_write_multi
};

static const LPS25HB_Interface_t lps25hb_interface =
{
	i2c_master_read_byte,
	i2c_master_write_byte,
	i2c_master_read_multi,
	i2c_master_write_multi
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static int32_t round_scaled(float value, int32_t scale);
static void format_fixed(char *buffer, uint32_t size, int32_t scaled_value, uint8_t decimals);
static uint8_t measure_reference_pressure(float *reference_pressure);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */


  float reference_pressure = 0.0f;

  HTS221_RegisterInterface(&hts221_interface);
  LPS25HB_RegisterInterface(&lps25hb_interface);

  while(HTS221_Init() != HTS221_OK)
  {
	  USART2_SendString("HTS221 init error\r\n");
	  LL_mDelay(1000);
  }

  while(LPS25HB_Init() != LPS25HB_OK)
  {
	  USART2_SendString("LPS25HB init error\r\n");
	  LL_mDelay(1000);
  }

  while(measure_reference_pressure(&reference_pressure) != 0U)
  {
	  USART2_SendString("Reference pressure error\r\n");
	  LL_mDelay(1000);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  float temperature,humidity, pressure,height;

	  if((HTS221_ReadTemperature(&temperature) == HTS221_OK) && (HTS221_ReadHumidity(&humidity) == HTS221_OK) && (LPS25HB_ReadPressure(&pressure) == LPS25HB_OK))
	  {
		  char temperature_text[16];
		  char humidity_text[16];
		  char pressure_text[16];
		  char height_text[16];
		  char line[80];

		  height = LPS25HB_CalcRelativeHeight(pressure, reference_pressure);

		  format_fixed(temperature_text, sizeof(temperature_text), round_scaled(temperature, 10), 1);
		  format_fixed(humidity_text, sizeof(humidity_text), round_scaled(humidity, 1), 0);
		  format_fixed(pressure_text, sizeof(pressure_text), round_scaled(pressure, 100), 2);
		  format_fixed(height_text, sizeof(height_text), round_scaled(height, 100), 2);

		  snprintf(line, sizeof(line), "%s,%s,%s,%s\r\n", temperature_text, humidity_text, pressure_text, height_text);
		  USART2_SendString(line);
	  }

	  LL_mDelay(MEASURE_PERIOD_MS);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  Error_Handler();
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(8000000);
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(8000000);
  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_HSI);
}

/* USER CODE BEGIN 4 */
static int32_t round_scaled(float value, int32_t scale){
	float scaled = value * (float)scale;

	if(scaled >= 0.0f){
		return (int32_t)(scaled + 0.5f);
	}
    return (int32_t)(scaled - 0.5f);
}


static void format_fixed(char *buffer, uint32_t size, int32_t scaled_value, uint8_t decimals)
{
	const char *sign = "";
	uint32_t magnitude;
	uint32_t divisor = 1U;

	if(scaled_value < 0){
		sign = "-";
		magnitude = (uint32_t)(-scaled_value);
	}
	else{
		magnitude = (uint32_t)scaled_value;
	}

	for(uint8_t i = 0; i < decimals; i++){
		divisor *= 10U;
	}

	if(decimals == 0U){
		snprintf(buffer, size, "%s%lu", sign, (unsigned long)magnitude);
	}
	else if(decimals == 1U){
		snprintf(buffer, size, "%s%lu.%lu", sign, (unsigned long)(magnitude / divisor), (unsigned long)(magnitude % divisor));
	}
	else{
		snprintf(buffer, size, "%s%lu.%02lu", sign, (unsigned long)(magnitude / divisor), (unsigned long)(magnitude % divisor));
	}
}



static uint8_t measure_reference_pressure(float *reference_pressure)
{
	float sum = 0.0f;
	float minimum = 0.0f;
	float maximum = 0.0f;
	float pressure;
	uint32_t samples = 0U;
	uint32_t skipped = 0U;

	for(uint32_t attempt = 0U; attempt < REFERENCE_MAX_ATTEMPTS; attempt++)
	{
		if(LPS25HB_ReadPressure(&pressure) == LPS25HB_OK){
			if(skipped < 5U){
				skipped++;
			}
			else{
				if(samples == 0U){
					minimum = pressure;
					maximum = pressure;
					sum = 0.0f;
				}

				if(pressure < minimum){
					minimum = pressure;
				}
				if(pressure > maximum){
					maximum = pressure;
				}

				sum += pressure;
				samples++;

				if((maximum - minimum) > 1.0f){
					samples = 0U;
				}
				else if(samples >= REFERENCE_SAMPLES){
					*reference_pressure = sum / (float)samples;
					return 0U;
				}
			}
		}

		LL_mDelay(MEASURE_PERIOD_MS);
	}

	return 1U;
}
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

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
