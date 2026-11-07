#include "lps25hb.h"
#include <math.h>

#define LPS25HB_HEIGHT_FACTOR_M   44330.0f
#define LPS25HB_HEIGHT_EXPONENT   0.1903f

typedef struct
{
	uint8_t ctrl1_register;
	uint8_t ctrl2_register;
	uint8_t ctrl1_value;
	uint8_t ctrl2_value;
	uint8_t auto_increment;
	uint8_t status_pressure_mask;
	uint8_t status_temperature_mask;
	float temperature_lsb_per_degc;
	float temperature_offset_degc;
} LPS25HB_Variant_t;

static const LPS25HB_Variant_t variant_lps25hb =
{
	LPS25HB_REG_CTRL_REG1,
	LPS25HB_REG_CTRL_REG2,
	LPS25HB_CTRL1_PD | LPS25HB_CTRL1_ODR_12HZ5 | LPS25HB_CTRL1_BDU,
	0x00U,
	LPS25HB_AUTO_INCREMENT,
	LPS25HB_STATUS_P_DA,
	LPS25HB_STATUS_T_DA,
	LPS25HB_TEMP_LSB_PER_DEGC,
	LPS25HB_TEMP_OFFSET_DEGC
};

static const LPS25HB_Variant_t variant_lps22hb =
{
	LPS22HB_REG_CTRL_REG1,
	LPS22HB_REG_CTRL_REG2,
	LPS22HB_CTRL1_ODR_10HZ | LPS22HB_CTRL1_BDU,
	LPS22HB_CTRL2_IF_ADD_INC,
	0x00U,
	LPS22HB_STATUS_P_DA,
	LPS22HB_STATUS_T_DA,
	LPS22HB_TEMP_LSB_PER_DEGC,
	0.0f
};

static LPS25HB_Interface_t lps25hb_interface = {0, 0, 0, 0};
static uint8_t lps25hb_address = LPS25HB_I2C_ADDRESS_SA0_HIGH;
static uint8_t lps25hb_id = 0;
static const LPS25HB_Variant_t *lps25hb_variant = &variant_lps25hb;

static uint8_t lps25hb_interface_ready(void)
{
	return (lps25hb_interface.read_byte != 0) &&
		   (lps25hb_interface.write_byte != 0) &&
		   (lps25hb_interface.read_multi != 0) &&
		   (lps25hb_interface.write_multi != 0);
}

static uint8_t lps25hb_write_verified(uint8_t register_address, uint8_t value)
{
	uint8_t readback = 0;

	if(LPS25HB_WriteRegister(register_address, value) != LPS25HB_OK)
	{
		return LPS25HB_ERR_COMM;
	}

	if(LPS25HB_ReadRegister(register_address, &readback) != LPS25HB_OK)
	{
		return LPS25HB_ERR_COMM;
	}

	if(readback != value)
	{
		return LPS25HB_ERR_CONFIG;
	}

	return LPS25HB_OK;
}

static uint8_t lps25hb_data_ready(uint8_t mask)
{
	uint8_t data_status = 0;
	uint8_t status = LPS25HB_ReadRegister(LPS25HB_REG_STATUS, &data_status);

	if(status != LPS25HB_OK)
	{
		return status;
	}

	if((data_status & mask) == 0U)
	{
		return LPS25HB_ERR_NOT_READY;
	}

	return LPS25HB_OK;
}

void LPS25HB_RegisterInterface(const LPS25HB_Interface_t *interface)
{
	if(interface != 0)
	{
		lps25hb_interface = *interface;
	}
}

uint8_t LPS25HB_ReadRegister(uint8_t register_address, uint8_t *value)
{
	if(!lps25hb_interface_ready())
	{
		return LPS25HB_ERR_NO_INTERFACE;
	}

	if(lps25hb_interface.read_byte(lps25hb_address, register_address, value) != 0U)
	{
		return LPS25HB_ERR_COMM;
	}

	return LPS25HB_OK;
}

uint8_t LPS25HB_WriteRegister(uint8_t register_address, uint8_t value)
{
	if(!lps25hb_interface_ready())
	{
		return LPS25HB_ERR_NO_INTERFACE;
	}

	if(lps25hb_interface.write_byte(lps25hb_address, register_address, value) != 0U)
	{
		return LPS25HB_ERR_COMM;
	}

	return LPS25HB_OK;
}

uint8_t LPS25HB_GetSensorId(void)
{
	return lps25hb_id;
}

uint8_t LPS25HB_Init(void)
{
	static const uint8_t candidates[2] = {LPS25HB_I2C_ADDRESS_SA0_HIGH, LPS25HB_I2C_ADDRESS_SA0_LOW};
	uint8_t who_am_i = 0;
	uint8_t status = LPS25HB_ERR_COMM;

	if(!lps25hb_interface_ready())
	{
		return LPS25HB_ERR_NO_INTERFACE;
	}

	for(uint8_t i = 0; i < 2U; i++)
	{
		lps25hb_address = candidates[i];
		who_am_i = 0;

		status = LPS25HB_ReadRegister(LPS25HB_REG_WHO_AM_I, &who_am_i);
		if(status == LPS25HB_OK)
		{
			break;
		}
	}

	if(status != LPS25HB_OK)
	{
		return status;
	}

	lps25hb_id = who_am_i;

	if(who_am_i == LPS25HB_WHO_AM_I_VALUE)
	{
		lps25hb_variant = &variant_lps25hb;
	}
	else if(who_am_i == LPS22HB_WHO_AM_I_VALUE)
	{
		lps25hb_variant = &variant_lps22hb;
	}
	else
	{
		return LPS25HB_ERR_WRONG_ID;
	}

	status = lps25hb_write_verified(lps25hb_variant->ctrl2_register, lps25hb_variant->ctrl2_value);
	if(status != LPS25HB_OK)
	{
		return status;
	}

	return lps25hb_write_verified(lps25hb_variant->ctrl1_register, lps25hb_variant->ctrl1_value);
}

uint8_t LPS25HB_ReadPressure(float *pressure_hpa)
{
	uint8_t raw[3];
	int32_t value;
	uint8_t status;

	if(pressure_hpa == 0)
	{
		return LPS25HB_ERR_COMM;
	}

	status = lps25hb_data_ready(lps25hb_variant->status_pressure_mask);
	if(status != LPS25HB_OK)
	{
		return status;
	}

	if(lps25hb_interface.read_multi(lps25hb_address, LPS25HB_REG_PRESS_OUT_XL | lps25hb_variant->auto_increment, raw, 3) != 0U)
	{
		return LPS25HB_ERR_COMM;
	}

	value = (int32_t)(((uint32_t)raw[2] << 16) | ((uint32_t)raw[1] << 8) | raw[0]);
	if((value & 0x00800000) != 0)
	{
		value |= (int32_t)0xFF000000;
	}

	*pressure_hpa = (float)value / LPS25HB_PRESSURE_LSB_PER_HPA;

	return LPS25HB_OK;
}

uint8_t LPS25HB_ReadTemperature(float *temperature_degc)
{
	uint8_t raw[2];
	int16_t value;
	uint8_t status;

	if(temperature_degc == 0)
	{
		return LPS25HB_ERR_COMM;
	}

	status = lps25hb_data_ready(lps25hb_variant->status_temperature_mask);
	if(status != LPS25HB_OK)
	{
		return status;
	}

	if(lps25hb_interface.read_multi(lps25hb_address, LPS25HB_REG_TEMP_OUT_L | lps25hb_variant->auto_increment, raw, 2) != 0U)
	{
		return LPS25HB_ERR_COMM;
	}

	value = (int16_t)(((uint16_t)raw[1] << 8) | raw[0]);

	*temperature_degc = lps25hb_variant->temperature_offset_degc + ((float)value / lps25hb_variant->temperature_lsb_per_degc);

	return LPS25HB_OK;
}

float LPS25HB_CalcRelativeHeight(float pressure_hpa, float reference_pressure_hpa)
{
	if((pressure_hpa <= 0.0f) || (reference_pressure_hpa <= 0.0f))
	{
		return 0.0f;
	}

	return LPS25HB_HEIGHT_FACTOR_M * (1.0f - powf(pressure_hpa / reference_pressure_hpa, LPS25HB_HEIGHT_EXPONENT));
}
