/**
  ******************************************************************************
  * File Name          : I2C.h
  * Description        : This file provides code for the configuration
  *                      of the I2C instances.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __i2c_H
#define __i2c_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define I2C_OK 0U
#define I2C_ERR_NACK 1U
#define I2C_ERR_TIMEOUT 2U
#define I2C_ERR_PARAM 3U
#define I2C_MAX_TRANSFER 254U
#define I2C_TIMEOUT 100000U

 typedef enum{I2C_WAIT_TXIS,I2C_WAIT_RXNE,I2C_WAIT_TC,I2C_WAIT_STOP} i2c_wait_t;
/* USER CODE END Private defines */

void MX_I2C1_Init(void);

/* USER CODE BEGIN Prototypes */
uint8_t i2c_master_read_byte(uint8_t slave_address, uint8_t register_address, uint8_t *data);
uint8_t i2c_master_read_multi(uint8_t slave_address, uint8_t register_address, uint8_t *data, uint8_t length);
uint8_t i2c_master_write_byte(uint8_t slave_address, uint8_t register_address, uint8_t data);
uint8_t i2c_master_write_multi(uint8_t slave_address, uint8_t register_address, const uint8_t *data, uint8_t length);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ i2c_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
