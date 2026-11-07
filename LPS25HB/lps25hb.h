#ifndef LPS25HB_H_
#define LPS25HB_H_

#include <stdint.h>

#define LPS25HB_I2C_ADDRESS_SA0_HIGH 0x5DU
#define LPS25HB_I2C_ADDRESS_SA0_LOW  0x5CU

#define LPS25HB_REG_WHO_AM_I         0x0FU
#define LPS25HB_REG_CTRL_REG1        0x20U
#define LPS25HB_REG_CTRL_REG2        0x21U
#define LPS25HB_REG_STATUS           0x27U
#define LPS25HB_REG_PRESS_OUT_XL     0x28U
#define LPS25HB_REG_TEMP_OUT_L       0x2BU

#define LPS25HB_AUTO_INCREMENT       0x80U

#define LPS25HB_WHO_AM_I_VALUE       0xBDU

#define LPS22HB_WHO_AM_I_VALUE       0xB1U
#define LPS22HB_REG_CTRL_REG1        0x10U
#define LPS22HB_REG_CTRL_REG2        0x11U
#define LPS22HB_CTRL1_ODR_10HZ       0x20U
#define LPS22HB_CTRL1_BDU            0x02U
#define LPS22HB_CTRL2_IF_ADD_INC     0x10U
#define LPS22HB_STATUS_P_DA          0x01U
#define LPS22HB_STATUS_T_DA          0x02U
#define LPS22HB_TEMP_LSB_PER_DEGC    100.0f

#define LPS25HB_CTRL1_PD             0x80U
#define LPS25HB_CTRL1_ODR_12HZ5      0x30U
#define LPS25HB_CTRL1_BDU            0x04U

#define LPS25HB_STATUS_T_DA          0x01U
#define LPS25HB_STATUS_P_DA          0x02U

#define LPS25HB_PRESSURE_LSB_PER_HPA 4096.0f
#define LPS25HB_TEMP_LSB_PER_DEGC    480.0f
#define LPS25HB_TEMP_OFFSET_DEGC     42.5f

#define LPS25HB_OK                   0U
#define LPS25HB_ERR_NO_INTERFACE     1U
#define LPS25HB_ERR_COMM             2U
#define LPS25HB_ERR_WRONG_ID         3U
#define LPS25HB_ERR_NOT_READY        4U
#define LPS25HB_ERR_CONFIG           5U

typedef struct
{
	uint8_t (*read_byte)(uint8_t slave_address, uint8_t register_address, uint8_t *data);
	uint8_t (*write_byte)(uint8_t slave_address, uint8_t register_address, uint8_t data);
	uint8_t (*read_multi)(uint8_t slave_address, uint8_t register_address, uint8_t *data, uint8_t length);
	uint8_t (*write_multi)(uint8_t slave_address, uint8_t register_address, const uint8_t *data, uint8_t length);
} LPS25HB_Interface_t;

void LPS25HB_RegisterInterface(const LPS25HB_Interface_t *interface);

uint8_t LPS25HB_Init(void);
uint8_t LPS25HB_GetSensorId(void);

uint8_t LPS25HB_ReadRegister(uint8_t register_address, uint8_t *value);
uint8_t LPS25HB_WriteRegister(uint8_t register_address, uint8_t value);

uint8_t LPS25HB_ReadPressure(float *pressure_hpa);
uint8_t LPS25HB_ReadTemperature(float *temperature_degc);

float LPS25HB_CalcRelativeHeight(float pressure_hpa, float reference_pressure_hpa);

#endif
