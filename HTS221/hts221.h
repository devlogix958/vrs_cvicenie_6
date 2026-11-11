#ifndef HTS221_H_
#define HTS221_H_

#include <stdint.h>

#define HTS221_I2C_ADDRESS 0x5FU

#define HTS221_REG_WHO_AM_I 0x0FU
#define HTS221_REG_AV_CONF 0x10U
#define HTS221_REG_CTRL_REG1 0x20U
#define HTS221_REG_CTRL_REG2 0x21U
#define HTS221_REG_CTRL_REG3 0x22U
#define HTS221_REG_STATUS 0x27U
#define HTS221_REG_HUMIDITY_OUT_L 0x28U
#define HTS221_REG_TEMP_OUT_L 0x2AU
#define HTS221_REG_CALIB_START 0x30U

#define HTS221_AUTO_INCREMENT 0x80U

#define HTS221_WHO_AM_I_VALUE 0xBCU

#define HTS221_CTRL1_PD 0x80U
#define HTS221_CTRL1_BDU 0x04U
#define HTS221_CTRL1_ODR_1HZ 0x01U
#define HTS221_CTRL1_ODR_7HZ 0x02U
#define HTS221_CTRL1_ODR_12HZ5 0x03U

#define HTS221_AV_CONF_DEFAULT 0x1BU

#define HTS221_STATUS_T_DA 0x01U
#define HTS221_STATUS_H_DA 0x02U

#define HTS221_CALIB_SIZE 16U

#define HTS221_OK 0U
#define HTS221_ERR_NO_INTERFACE 1U
#define HTS221_ERR_COMM 2U
#define HTS221_ERR_WRONG_ID 3U
#define HTS221_ERR_NOT_READY 4U
#define HTS221_ERR_CALIBRATION 5U
#define HTS221_ERR_CONFIG 6U

typedef struct
{
	uint8_t (*read_byte)(uint8_t slave_address, uint8_t register_address, uint8_t *data);
	uint8_t (*write_byte)(uint8_t slave_address, uint8_t register_address, uint8_t data);
	uint8_t (*read_multi)(uint8_t slave_address, uint8_t register_address, uint8_t *data, uint8_t length);
	uint8_t (*write_multi)(uint8_t slave_address, uint8_t register_address, const uint8_t *data, uint8_t length);
} HTS221_Interface_t;

void HTS221_RegisterInterface(const HTS221_Interface_t *interface);

uint8_t HTS221_Init(void);

uint8_t HTS221_ReadRegister(uint8_t register_address, uint8_t *value);
uint8_t HTS221_WriteRegister(uint8_t register_address, uint8_t value);

uint8_t HTS221_ReadTemperature(float *temperature_degc);
uint8_t HTS221_ReadHumidity(float *humidity_percent);

#endif
