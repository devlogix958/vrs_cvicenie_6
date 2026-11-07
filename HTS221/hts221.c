#include "hts221.h"

typedef struct
{
	float t0_degc;
	float t1_degc;
	int16_t t0_out;
	int16_t t1_out;
	float h0_rh;
	float h1_rh;
	int16_t h0_out;
	int16_t h1_out;
} HTS221_Calibration_t;

static HTS221_Interface_t hts221_interface = {0, 0, 0, 0};
static HTS221_Calibration_t hts221_calibration;

static int16_t hts221_make_int16(uint8_t low, uint8_t high){
	return (int16_t)(((uint16_t)high << 8) | low);
}

static uint8_t hts221_interface_ready(void){
	return (hts221_interface.read_byte != 0) &&(hts221_interface.write_byte != 0) &&(hts221_interface.read_multi != 0) &&(hts221_interface.write_multi != 0);
}

static uint8_t hts221_read_calibration(void){
	uint8_t raw[HTS221_CALIB_SIZE];
	uint16_t t0_x8;
	uint16_t t1_x8;

	if(hts221_interface.read_multi(HTS221_I2C_ADDRESS, HTS221_REG_CALIB_START | HTS221_AUTO_INCREMENT, raw, HTS221_CALIB_SIZE) != 0U)
	{
		return HTS221_ERR_COMM;
	}

	t0_x8 = (uint16_t)(((uint16_t)(raw[5] & 0x03U) << 8) | raw[2]);
	t1_x8 = (uint16_t)(((uint16_t)(raw[5] & 0x0CU) << 6) | raw[3]);

	hts221_calibration.t0_degc = (float)t0_x8 / 8.0f;
	hts221_calibration.t1_degc = (float)t1_x8 / 8.0f;
	hts221_calibration.t0_out = hts221_make_int16(raw[12], raw[13]);
	hts221_calibration.t1_out = hts221_make_int16(raw[14], raw[15]);

	hts221_calibration.h0_rh = (float)raw[0] / 2.0f;
	hts221_calibration.h1_rh = (float)raw[1] / 2.0f;
	hts221_calibration.h0_out = hts221_make_int16(raw[6], raw[7]);
	hts221_calibration.h1_out = hts221_make_int16(raw[10], raw[11]);

	if((hts221_calibration.t1_out == hts221_calibration.t0_out) ||
	   (hts221_calibration.h1_out == hts221_calibration.h0_out))
	{
		return HTS221_ERR_CALIBRATION;
	}

	return HTS221_OK;
}

static uint8_t hts221_write_verified(uint8_t register_address, uint8_t value){
	uint8_t readback = 0;

	if(HTS221_WriteRegister(register_address, value) != HTS221_OK){
		return HTS221_ERR_COMM;
	}

	if(HTS221_ReadRegister(register_address, &readback) != HTS221_OK){
		return HTS221_ERR_COMM;
	}

	if(readback != value){
		return HTS221_ERR_CONFIG;
	}

	return HTS221_OK;
}

void HTS221_RegisterInterface(const HTS221_Interface_t *interface){
	if(interface != 0){
		hts221_interface = *interface;
	}
}

uint8_t HTS221_ReadRegister(uint8_t register_address, uint8_t *value){
	if(!hts221_interface_ready()){
		return HTS221_ERR_NO_INTERFACE;
	}

	if(hts221_interface.read_byte(HTS221_I2C_ADDRESS, register_address, value) != 0U){
		return HTS221_ERR_COMM;
	}

	return HTS221_OK;
}

uint8_t HTS221_WriteRegister(uint8_t register_address, uint8_t value){
	if(!hts221_interface_ready()){
		return HTS221_ERR_NO_INTERFACE;
	}

	if(hts221_interface.write_byte(HTS221_I2C_ADDRESS, register_address, value) != 0U){
		return HTS221_ERR_COMM;
	}

	return HTS221_OK;
}

uint8_t HTS221_Init(void){
	uint8_t who_am_i = 0;
	uint8_t status;

	if(!hts221_interface_ready())
	{
		return HTS221_ERR_NO_INTERFACE;
	}

	status = HTS221_ReadRegister(HTS221_REG_WHO_AM_I, &who_am_i);
	if(status != HTS221_OK)
	{
		return status;
	}

	if(who_am_i != HTS221_WHO_AM_I_VALUE)
	{
		return HTS221_ERR_WRONG_ID;
	}

	status = hts221_read_calibration();
	if(status != HTS221_OK)
	{
		return status;
	}

	status = hts221_write_verified(HTS221_REG_AV_CONF, HTS221_AV_CONF_DEFAULT);
	if(status != HTS221_OK)
	{
		return status;
	}

	status = hts221_write_verified(HTS221_REG_CTRL_REG2, 0x00U);
	if(status != HTS221_OK)
	{
		return status;
	}

	status = hts221_write_verified(HTS221_REG_CTRL_REG3, 0x00U);
	if(status != HTS221_OK)
	{
		return status;
	}

	return hts221_write_verified(HTS221_REG_CTRL_REG1, HTS221_CTRL1_PD | HTS221_CTRL1_BDU | HTS221_CTRL1_ODR_12HZ5);
}

uint8_t HTS221_ReadTemperature(float *temperature_degc)
{
	uint8_t data_status = 0;
	uint8_t raw[2];
	int16_t t_out;
	uint8_t status;

	if(temperature_degc == 0)
	{
		return HTS221_ERR_COMM;
	}

	status = HTS221_ReadRegister(HTS221_REG_STATUS, &data_status);
	if(status != HTS221_OK)
	{
		return status;
	}

	if((data_status & HTS221_STATUS_T_DA) == 0U)
	{
		return HTS221_ERR_NOT_READY;
	}

	if(hts221_interface.read_multi(HTS221_I2C_ADDRESS, HTS221_REG_TEMP_OUT_L | HTS221_AUTO_INCREMENT, raw, 2) != 0U)
	{
		return HTS221_ERR_COMM;
	}

	t_out = hts221_make_int16(raw[0], raw[1]);

	*temperature_degc = hts221_calibration.t0_degc +
			((float)(t_out - hts221_calibration.t0_out) * (hts221_calibration.t1_degc - hts221_calibration.t0_degc)) /
			(float)(hts221_calibration.t1_out - hts221_calibration.t0_out);

	return HTS221_OK;
}

uint8_t HTS221_ReadHumidity(float *humidity_percent)
{
	uint8_t data_status = 0;
	uint8_t raw[2];
	int16_t h_out;
	float humidity;
	uint8_t status;

	if(humidity_percent == 0)
	{
		return HTS221_ERR_COMM;
	}

	status = HTS221_ReadRegister(HTS221_REG_STATUS, &data_status);
	if(status != HTS221_OK)
	{
		return status;
	}

	if((data_status & HTS221_STATUS_H_DA) == 0U)
	{
		return HTS221_ERR_NOT_READY;
	}

	if(hts221_interface.read_multi(HTS221_I2C_ADDRESS, HTS221_REG_HUMIDITY_OUT_L | HTS221_AUTO_INCREMENT, raw, 2) != 0U)
	{
		return HTS221_ERR_COMM;
	}

	h_out = hts221_make_int16(raw[0], raw[1]);

	humidity = hts221_calibration.h0_rh +
			((float)(h_out - hts221_calibration.h0_out) * (hts221_calibration.h1_rh - hts221_calibration.h0_rh)) /
			(float)(hts221_calibration.h1_out - hts221_calibration.h0_out);

	if(humidity < 0.0f)
	{
		humidity = 0.0f;
	}
	else if(humidity > 100.0f)
	{
		humidity = 100.0f;
	}

	*humidity_percent = humidity;

	return HTS221_OK;
}
