/**
  ******************************************************************************
  * File Name          : I2C.c
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

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */


static uint32_t i2c_flag_is_set(i2c_wait_t flag){
	switch(flag)
	{
	case I2C_WAIT_TXIS:
		return LL_I2C_IsActiveFlag_TXIS(I2C1);
	case I2C_WAIT_RXNE:
		return LL_I2C_IsActiveFlag_RXNE(I2C1);
	case I2C_WAIT_TC:
		return LL_I2C_IsActiveFlag_TC(I2C1);
	default:
		return LL_I2C_IsActiveFlag_STOP(I2C1);
	}
}



static void i2c_recover(void){
	LL_I2C_Disable(I2C1);
	LL_I2C_Enable(I2C1);
}



static uint8_t i2c_wait_flag(i2c_wait_t flag){
	uint32_t timeout = I2C_TIMEOUT;

	while(!i2c_flag_is_set(flag)){

		if(LL_I2C_IsActiveFlag_NACK(I2C1)){
			return I2C_ERR_NACK;
		}

		--timeout;

		if(timeout == 0U){
			return I2C_ERR_TIMEOUT;
		}
	}

	return I2C_OK;
}



static uint8_t i2c_wait_bus_free(void)
{
	uint32_t timeout = I2C_TIMEOUT;

	while(LL_I2C_IsActiveFlag_BUSY(I2C1))
	{
		--timeout;

		if(timeout == 0U)
		{
			return I2C_ERR_TIMEOUT;
		}
	}

	return I2C_OK;
}



static uint8_t i2c_finish(uint8_t status)
{
	if(status == I2C_OK){
		LL_I2C_ClearFlag_STOP(I2C1);
	}
	else{
		i2c_recover();
	}

	return status;
}
/* USER CODE END 0 */

/* I2C1 init function */
void MX_I2C1_Init(void){
  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB6   ------> I2C1_SCL
  PB7   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* I2C1 interrupt Init */
  NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(I2C1_EV_IRQn);

  /** I2C Initialization
  */
  LL_I2C_EnableAutoEndMode(I2C1);
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x2000090E;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 2;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);

}



/* USER CODE BEGIN 1 */
uint8_t i2c_master_read_multi(uint8_t slave_address, uint8_t register_address, uint8_t *data, uint8_t length){
	uint8_t status;
	uint8_t address = (uint8_t)(slave_address << 1);

	if((data == 0) || (length == 0U) || (length > I2C_MAX_TRANSFER))
	{
		return I2C_ERR_PARAM;
	}

	status = i2c_wait_bus_free();
	if(status != I2C_OK)
	{
		return i2c_finish(status);
	}

	LL_I2C_HandleTransfer(I2C1, address, LL_I2C_ADDRSLAVE_7BIT, 1, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

	status = i2c_wait_flag(I2C_WAIT_TXIS);
	if(status != I2C_OK)
	{
		return i2c_finish(status);
	}

	LL_I2C_TransmitData8(I2C1, register_address);

	status = i2c_wait_flag(I2C_WAIT_TC);
	if(status != I2C_OK)
	{
		return i2c_finish(status);
	}

	LL_I2C_HandleTransfer(I2C1, address, LL_I2C_ADDRSLAVE_7BIT, length, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ);

	for(uint8_t i = 0; i < length; i++)
	{
		status = i2c_wait_flag(I2C_WAIT_RXNE);
		if(status != I2C_OK)
		{
			return i2c_finish(status);
		}

		data[i] = LL_I2C_ReceiveData8(I2C1);
	}

	status = i2c_wait_flag(I2C_WAIT_STOP);

	return i2c_finish(status);
}




uint8_t i2c_master_read_byte(uint8_t slave_address, uint8_t register_address, uint8_t *data)
{
	return i2c_master_read_multi(slave_address, register_address, data, 1);
}





uint8_t i2c_master_write_multi(uint8_t slave_address, uint8_t register_address, const uint8_t *data, uint8_t length)
{
	uint8_t status;
	uint8_t address = (uint8_t)(slave_address << 1);

	if((data == 0) || (length == 0U) || (length > I2C_MAX_TRANSFER))
	{
		return I2C_ERR_PARAM;
	}

	status = i2c_wait_bus_free();
	if(status != I2C_OK)
	{
		return i2c_finish(status);
	}

	LL_I2C_HandleTransfer(I2C1, address, LL_I2C_ADDRSLAVE_7BIT, (uint32_t)length + 1U, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	status = i2c_wait_flag(I2C_WAIT_TXIS);
	if(status != I2C_OK)
	{
		return i2c_finish(status);
	}

	LL_I2C_TransmitData8(I2C1, register_address);

	for(uint8_t i = 0; i < length; i++)
	{
		status = i2c_wait_flag(I2C_WAIT_TXIS);
		if(status != I2C_OK)
		{
			return i2c_finish(status);
		}
		LL_I2C_TransmitData8(I2C1, data[i]);
	}

	status = i2c_wait_flag(I2C_WAIT_STOP);

	return i2c_finish(status);
}




uint8_t i2c_master_write_byte(uint8_t slave_address, uint8_t register_address, uint8_t data)
{
	return i2c_master_write_multi(slave_address, register_address, &data, 1);
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
