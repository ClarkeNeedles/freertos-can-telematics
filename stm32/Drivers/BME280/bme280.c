/**
  ******************************************************************************
  * @file           : bme280.c
  * @brief          : BME280 sensor driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#include "bme280.h"

/*
 * ============================================================================
 *                       ##### CALIBRATION LOADING #####
 * ============================================================================
 */

HAL_StatusTypeDef BME280_ReadCalibration(BME280_Handle_t *dev)
{
	uint8_t calib[32];
	uint8_t calib2[7];
	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Read(
		dev->hi2c, 
		dev->address, 
		0x88, 
		I2C_MEMADD_SIZE_8BIT, 
		&calib, 
		25, 
		HAL_MAX_DELAY
	);

	if (status != HAL_OK)
		return status;

	status = HAL_I2C_Mem_Read(
		dev->hi2c, 
		dev->address, 
		0xE1, 
		I2C_MEMADD_SIZE_8BIT, 
		&calib2, 
		7, 
		HAL_MAX_DELAY
	);

	if (status != HAL_OK)
		return status;

	dev->calib.dig_T1 = (uint16_t)(calib[1] << 8 | calib[0]);
	dev->calib.dig_T2 = (int16_t)(calib[3] << 8 | calib[2]);
	dev->calib.dig_T3 = (int16_t)(calib[5] << 8 | calib[4]);

	dev->calib.dig_P1 = (uint16_t)(calib[7] << 8 | calib[6]);
	dev->calib.dig_P2 = (int16_t)(calib[9] << 8 | calib[8]);
	dev->calib.dig_P3 = (int16_t)(calib[11] << 8 | calib[10]);
	dev->calib.dig_P4 = (int16_t)(calib[13] << 8 | calib[12]);
	dev->calib.dig_P5 = (int16_t)(calib[15] << 8 | calib[14]);
	dev->calib.dig_P6 = (int16_t)(calib[17] << 8 | calib[16]);
	dev->calib.dig_P7 = (int16_t)(calib[19] << 8 | calib[18]);
	dev->calib.dig_P8 = (int16_t)(calib[21] << 8 | calib[20]);
	dev->calib.dig_P9 = (int16_t)(calib[23] << 8 | calib[22]);

	dev->calib.dig_H1 = calib[24];
	dev->calib.dig_H2 = (int16_t)(calib2[1] << 8 | calib2[0]);
	dev->calib.dig_H3 = calib2[2];
	dev->calib.dig_H4 = (int16_t)((calib2[3] << 4) | (calib2[4] & 0x0F));
	dev->calib.dig_H5 = (int16_t)((calib2[5] << 4) | (calib2[4] >> 4));
	dev->calib.dig_H6 = (int8_t)calib2[6];

	return HAL_OK;
}

/*
 * ============================================================================
 *                        ##### DEVICE CONTROL #####
 * ============================================================================
 */

HAL_StatusTypeDef BME280_Reset(BME280_Handle_t *dev)
{
	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Write(
		dev->hi2c, 
		dev->address, 
		BME280_REG_RESET, 
		I2C_MEMADD_SIZE_8BIT, 
		(uint8_t *)0xB6, 
		1, 
		HAL_MAX_DELAY
	);

	if (status != HAL_OK)
		return status;

	HAL_Delay(100);
	return HAL_OK;
}

HAL_StatusTypeDef BME280_CheckID(BME280_Handle_t *dev)
{
	uint8_t id = 0;
	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Read(
		dev->hi2c, 
		dev->address, 
		BME280_REG_ID, 
		I2C_MEMADD_SIZE_8BIT, 
		&id, 
		1, 
		HAL_MAX_DELAY
	);
	if (status != HAL_OK)
		return status;

	if (id != BME280_CHIP_ID)
		return HAL_ERROR;

	return HAL_OK;
}

HAL_StatusTypeDef BME280_Sleep(BME280_Handle_t *dev)
{ 
	return HAL_I2C_Mem_Write(
		dev->hi2c, 
		dev->address, 
		BME280_REG_CTRL_MEAS, 
		I2C_MEMADD_SIZE_8BIT, 
		(uint8_t *)BME280_MODE_SLEEP, 
		1, 
		HAL_MAX_DELAY
	);
}

HAL_StatusTypeDef BME280_Wakeup(BME280_Handle_t *dev)
{
	uint8_t ctrl;
	HAL_StatusTypeDef status;

	status = HAL_I2C_Mem_Read(
		dev->hi2c, 
		dev->address, 
		BME280_REG_CTRL_MEAS, 
		I2C_MEMADD_SIZE_8BIT, 
		&ctrl, 
		1, 
		HAL_MAX_DELAY
	);
	if (status != HAL_OK)
		return status;

	ctrl |= BME280_MODE_FORCED;

	return HAL_I2C_Mem_Write(
		dev->hi2c, 
		dev->address, 
		BME280_REG_CTRL_MEAS, 
		I2C_MEMADD_SIZE_8BIT, 
		&ctrl, 
		1, 
		HAL_MAX_DELAY
	);
}

/*
 * ============================================================================
 *                        ##### INITIALIZATION #####
 * ============================================================================
 */

HAL_StatusTypeDef BME280_Init(BME280_Handle_t *dev,
							  BME280_Config_t *cfg)
{
	HAL_StatusTypeDef status;

	if (BME280_CheckID(dev) != HAL_OK)
		return HAL_ERROR;

	if (BME280_Reset(dev) != HAL_OK)
		return HAL_ERROR;

	if (BME280_ReadCalibration(dev) != HAL_OK)
		return HAL_ERROR;

	// Humidity oversampling must be set first
	status = HAL_I2C_Mem_Write(
		dev->hi2c, 
		dev->address, 
		BME280_REG_CTRL_HUM, 
		I2C_MEMADD_SIZE_8BIT, 
		&cfg->osrs_h, 
		1, 
		HAL_MAX_DELAY
	);

	if (status != HAL_OK)
		return status;

	// Config register
	uint8_t config = (cfg->standby << 5) | (cfg->filter << 2);
	status = HAL_I2C_Mem_Write(
		dev->hi2c, 
		dev->address, 
		BME280_REG_CONFIG, 
		I2C_MEMADD_SIZE_8BIT, 
		&config, 
		1, 
		HAL_MAX_DELAY
	);

	if (status != HAL_OK)
		return status;

	// Control measurement register
	uint8_t ctrl_meas = (cfg->osrs_t << 5) | (cfg->osrs_p << 2) | (cfg->mode);
	status = HAL_I2C_Mem_Write(
		dev->hi2c, 
		dev->address, 
		BME280_REG_CTRL_MEAS, 
		I2C_MEMADD_SIZE_8BIT, 
		&ctrl_meas, 
		1, 
		HAL_MAX_DELAY
	);

	if (status != HAL_OK)
		return status;

	return HAL_OK;
}

/*
 * ============================================================================
 *                  	  ##### RAW DATA READ #####
 * ============================================================================
 */

HAL_StatusTypeDef BME280_ReadRaw(BME280_Handle_t *dev,
								 BME280_RawData_t *raw)
{
	uint8_t data[8];
	HAL_StatusTypeDef status;

	if (BME280_CheckID(dev) != HAL_OK)
		return HAL_ERROR;

	status = HAL_I2C_Mem_Read(
		dev->hi2c, 
		dev->address, 
		BME280_REG_PRESS_MSB, 
		I2C_MEMADD_SIZE_8BIT, 
		&data, 
		8, 
		HAL_MAX_DELAY
	);

	if (status != HAL_OK)
		return status;

	raw->raw_press = (int32_t)((data[0] << 12) | (data[1] << 4) | (data[2] >> 4));
	raw->raw_temp = (int32_t)((data[3] << 12) | (data[4] << 4) | (data[5] >> 4));
	raw->raw_hum = (int32_t)((data[6] << 8) | data[7]);

	return HAL_OK;
}

/*
 * ============================================================================
 *                  ##### COMPENSATION FUNCTIONS #####
 * ============================================================================
 */

static float BME280_CompensateTemp(BME280_Handle_t *dev,
								   int32_t adc_T)
{
	double var1, var2;
	var1 = (((adc_T / 16384.0) - (dev->calib.dig_T1 / 1024.0)) *
			dev->calib.dig_T2);
	var2 = (((adc_T / 131072.0) - (dev->calib.dig_T1 / 8192.0)) *
			((adc_T / 131072.0) - (dev->calib.dig_T1 / 8192.0))) *
			dev->calib.dig_T3;

	dev->t_fine = (int32_t)(var1 + var2);

	return (var1 + var2) / 5120.0;
}

static float BME280_CompensatePress(BME280_Handle_t *dev,
									int32_t adc_P)
{
	double var1, var2, p;
	var1 = (dev->t_fine / 2.0) - 64000.0;
	var2 = var1 * var1 * dev->calib.dig_P6 / 32768.0;
	var2 = var2 + var1 * dev->calib.dig_P5 * 2.0;
	var2 = (var2 / 4.0) + (dev->calib.dig_P4 * 65536.0);
	var1 = (dev->calib.dig_P3 * var1 * var1 / 524288.0 +
			dev->calib.dig_P2 * var1) / 524288.0;
	var1 = (1.0 + var1 / 32768.0) * dev->calib.dig_P1;

	if (var1 == 0)
		return 0;

	p = 1048576.0 - adc_P;
	p = (p - var2 / 4096.0) * 6250.0 / var1;
	var1 = dev->calib.dig_P9 * p * p / 2147483648.0;
	var2 = p * dev->calib.dig_P8 / 32768.0;

	return p + (var1 + var2 + dev->calib.dig_P7) / 16.0;
}

static float BME280_CompensateHum(BME280_Handle_t *dev,
								  int32_t adc_H)
{
	double h;
	h = dev->t_fine - 76800.0;
	h = (adc_H - (dev->calib.dig_H4 * 64.0 +
		 dev->calib.dig_H5 / 16384.0 * h)) *
		(dev->calib.dig_H2 / 65536.0 *
		(1.0 + dev->calib.dig_H6 / 67108864.0 * h *
		(1.0 + dev->calib.dig_H3 / 67108864.0 * h)));
	h = h * (1.0 - dev->calib.dig_H1 * h / 524288.0);

	if (h > 100.0)
		h = 100.0;
	if (h < 0.0)
		h = 0.0;

	return h;
}

/*
 * ============================================================================
 *                  ##### HIGH LEVEL READ FUNCTIONS #####
 * ============================================================================
 */

HAL_StatusTypeDef BME280_ReadAll(BME280_Handle_t *dev,
								 BME280_Data_t *out)
{
	BME280_RawData_t raw;
	if (BME280_ReadRaw(dev, &raw) != HAL_OK)
		return HAL_ERROR;

	out->temperature_c = BME280_CompensateTemp(dev, raw.raw_temp);
	out->pressure_pa   = BME280_CompensatePress(dev, raw.raw_press);
	out->humidity_pct  = BME280_CompensateHum(dev, raw.raw_hum);

	return HAL_OK;
}

HAL_StatusTypeDef BME280_ReadTemperature(BME280_Handle_t *dev,
										 float *temp)
{
	BME280_RawData_t raw;
	if (BME280_ReadRaw(dev, &raw) != HAL_OK)
		return HAL_ERROR;

	*temp = BME280_CompensateTemp(dev, raw.raw_temp);

	return HAL_OK;
}

HAL_StatusTypeDef BME280_ReadPressure(BME280_Handle_t *dev,
									  float *press)
{
	BME280_RawData_t raw;
	if (BME280_ReadRaw(dev, &raw) != HAL_OK)
		return HAL_ERROR;

	*press = BME280_CompensatePress(dev, raw.raw_press);

	return HAL_OK;
}

HAL_StatusTypeDef BME280_ReadHumidity(BME280_Handle_t *dev,
									  float *hum)
{
	BME280_RawData_t raw;
	if (BME280_ReadRaw(dev, &raw) != HAL_OK)
		return HAL_ERROR;

	*hum = BME280_CompensateHum(dev, raw.raw_hum);

	return HAL_OK;
}
