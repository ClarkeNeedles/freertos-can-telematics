/**
  ******************************************************************************
  * @file           : bme280.h
  * @brief          : Header file for BME280 sensor driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#ifndef __BME280_H
#define __BME280_H

#include "main.h"
#include "stm32f4xx_hal.h"

// Select communication interface
#define BME280_USE_I2C 0/**
  ******************************************************************************
  * @file           : bme280.h
  * @brief          : BME280 sensor driver header
  * @author         : Clarke Needles
  ******************************************************************************
  */

#ifndef BME280_H
#define BME280_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

/*
 * ============================================================================
 *                     	   ##### CONFIG OPTIONS #####
 * ============================================================================
 */

// Oversampling
#define BME280_OSRS_OFF     0x00
#define BME280_OSRS_1       0x01
#define BME280_OSRS_2       0x02
#define BME280_OSRS_4       0x03
#define BME280_OSRS_8       0x04
#define BME280_OSRS_16      0x05

// Operating modes
#define BME280_MODE_SLEEP   0x00
#define BME280_MODE_FORCED  0x01
#define BME280_MODE_NORMAL  0x03

// Standby times (ms)
#define BME280_STANDBY_0_5  0x00
#define BME280_STANDBY_62_5 0x01
#define BME280_STANDBY_125  0x02
#define BME280_STANDBY_250  0x03
#define BME280_STANDBY_500  0x04
#define BME280_STANDBY_1000 0x05
#define BME280_STANDBY_10    0x06
#define BME280_STANDBY_20    0x07

// Filter settings
#define BME280_FILTER_OFF   0x00
#define BME280_FILTER_2     0x01
#define BME280_FILTER_4     0x02
#define BME280_FILTER_8     0x03
#define BME280_FILTER_16    0x04

/*
 * ============================================================================
 *                     	   ##### REGISTER MAP #####
 * ============================================================================
 */

#define BME280_REG_ID          0xD0
#define BME280_REG_RESET       0xE0
#define BME280_REG_CTRL_HUM    0xF2
#define BME280_REG_STATUS      0xF3
#define BME280_REG_CTRL_MEAS   0xF4
#define BME280_REG_CONFIG      0xF5

#define BME280_REG_PRESS_MSB   0xF7  // burst read start
#define BME280_REG_TEMP_MSB    0xFA
#define BME280_REG_HUM_MSB     0xFD

#define BME280_CHIP_ID         0x60

/*
 * ============================================================================
 *                     	   ##### TYPE DEFINITIONS #####
 * ============================================================================
 */

typedef struct
{
	uint8_t osrs_t;
	uint8_t osrs_p;
	uint8_t osrs_h;
	uint8_t mode;
	uint8_t standby;
	uint8_t filter;
} BME280_Config_t;

typedef struct
{
	int32_t raw_temp;
	int32_t raw_press;
	int32_t raw_hum;
} BME280_RawData_t;

typedef struct
{
	float temperature_c;
	float pressure_pa;
	float humidity_pct;
} BME280_Data_t;

typedef struct
{
	I2C_HandleTypeDef *hi2c;
	uint8_t address;

	int32_t t_fine;

	// calibration data (filled at init)
	struct
	{
		uint16_t dig_T1;
		int16_t  dig_T2;
		int16_t  dig_T3;

		uint16_t dig_P1;
		int16_t  dig_P2;
		int16_t  dig_P3;
		int16_t  dig_P4;
		int16_t  dig_P5;
		int16_t  dig_P6;
		int16_t  dig_P7;
		int16_t  dig_P8;
		int16_t  dig_P9;

		uint8_t  dig_H1;
		int16_t  dig_H2;
		uint8_t  dig_H3;
		int16_t  dig_H4;
		int16_t  dig_H5;
		int8_t   dig_H6;
	} calib;
} BME280_Handle_t;

/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

HAL_StatusTypeDef BME280_Init(BME280_Handle_t *dev, BME280_Config_t *cfg);
HAL_StatusTypeDef BME280_Reset(BME280_Handle_t *dev);
HAL_StatusTypeDef BME280_Sleep(BME280_Handle_t *dev);
HAL_StatusTypeDef BME280_Wakeup(BME280_Handle_t *dev);
HAL_StatusTypeDef BME280_CheckID(BME280_Handle_t *dev);

HAL_StatusTypeDef BME280_ReadRaw(BME280_Handle_t *dev, BME280_RawData_t *raw);
HAL_StatusTypeDef BME280_ReadTemperature(BME280_Handle_t *dev, float *temp);
HAL_StatusTypeDef BME280_ReadPressure(BME280_Handle_t *dev, float *press);
HAL_StatusTypeDef BME280_ReadHumidity(BME280_Handle_t *dev, float *hum);
HAL_StatusTypeDef BME280_ReadAll(BME280_Handle_t *dev, BME280_Data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* BME280_H */

#define BME280_INTERFACE BME280_USE_I2C

// I2C
#define BME280_ADDRESS 0xEC // Address determined by SDO pin

// Oversampling definitions
#define OSRS_OFF    	0x00
#define OSRS_1      	0x01
#define OSRS_2      	0x02
#define OSRS_4      	0x03
#define OSRS_8      	0x04
#define OSRS_16     	0x05

// MODE Definitions
#define MODE_SLEEP      0x00
#define MODE_FORCED     0x01
#define MODE_NORMAL     0x03

// Standby Time
#define T_SB_0p5    	0x00
#define T_SB_62p5   	0x01
#define T_SB_125    	0x02
#define T_SB_250    	0x03
#define T_SB_500    	0x04
#define T_SB_1000   	0x05
#define T_SB_10     	0x06
#define T_SB_20     	0x07

// IIR Filter Coefficients
#define IIR_OFF     	0x00
#define IIR_2       	0x01
#define IIR_4       	0x02
#define IIR_8       	0x03
#define IIR_16      	0x04

// REGISTERS DEFINITIONS
#define ID_REG      	0xD0
#define RESET_REG  		0xE0
#define CTRL_HUM_REG    0xF2
#define STATUS_REG      0xF3
#define CTRL_MEAS_REG   0xF4
#define CONFIG_REG      0xF5
#define PRESS_MSB_REG   0xF7

#endif /* __BME280_H */
