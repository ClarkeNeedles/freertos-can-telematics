/**
  ******************************************************************************
  * @file           : bme280.h
  * @brief          : Header file for BME280 sensor driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#ifndef __BME280_H
#define __BME280_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
#define BME280_STANDBY_10   0x06
#define BME280_STANDBY_20   0x07

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
 *                     	  ##### TYPE DEFINITIONS #####
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

/*******************************************************************************
 * BME280_Init()
 *
 * @brief Initializes the BME280 sensor with user configuration.
 *
 * This function performs a sequence of verification, reset, and configuration steps
 * to prepare the sensor for operation. It verifies the device ID, triggers a software
 * reset, reads the factory calibration coefficients, and writes the configuration
 * parameters (oversampling, filtering, standby time, and operating mode) to the device.
 * 
 * @note The humidity oversampling register (@c BME280_REG_CTRL_HUM) must be written 
 *       before writing the control measurement register (@c BME280_REG_CTRL_MEAS) for 
 *       the changes to take effect.
 *
 * @param[in,out] dev Pointer to the BME280 device handle containing I2C details.
 * @param[in]     cfg Pointer to the configuration structure holding sensor settings.
 *
 * @retval HAL_OK    Sensor successfully initialized and configured.
 * @retval HAL_ERROR Device verification failed, reset timed out, or I2C communication errored.
 ******************************************************************************/
HAL_StatusTypeDef BME280_Init(BME280_Handle_t *dev, BME280_Config_t *cfg);

/*******************************************************************************
 * BME280_Reset()
 *
 * @brief Performs a software reset on the BME280 sensor.
 *
 * This function writes the soft reset command code (0xB6) to the sensor's
 * reset register. A soft reset provides the same effect as a power-on reset, 
 * erasing all setting values and restoring factory defaults. It includes a 
 * mandatory blocking delay to allow the sensor time to complete its boot procedure.
 *
 * @note This function blocks execution for 100 ms using @c HAL_Delay() to ensure 
 *       the sensor is fully operational before subsequent commands are sent.
 *
 * @param[in] dev Pointer to the BME280 device handle containing I2C details.
 *
 * @retval HAL_OK    The reset command was successfully sent and the startup delay has expired.
 * @retval HAL_ERROR The I2C write transaction failed.
 ******************************************************************************/
HAL_StatusTypeDef BME280_Reset(BME280_Handle_t *dev);

/*******************************************************************************
 * BME280_Sleep()
 *
 * @brief Places the BME280 sensor into sleep mode.
 *
 * This function switches the sensor's power mode to sleep by writing to the
 * control measurement register. In sleep mode, no measurements are performed 
 * and power consumption is minimized to its lowest state.
 *
 * @warning Writing directly to @c BME280_REG_CTRL_MEAS with this value will overwrite 
 *          and clear the temperature and pressure oversampling settings. If you intend 
 *          to preserve those configurations, consider a read-modify-write operation instead.
 *
 * @param[in] dev Pointer to the BME280 device handle containing I2C details.
 *
 * @retval HAL_OK    The sensor successfully entered sleep mode.
 * @retval HAL_ERROR The I2C write transaction failed.
 ******************************************************************************/
HAL_StatusTypeDef BME280_Sleep(BME280_Handle_t *dev);

/*******************************************************************************
 * BME280_Wakeup()
 *
 * @brief Wakes up the BME280 sensor by forcing a measurement.
 *
 * This function reads the current control measurement register, modifies the mode 
 * bits to set the sensor into forced mode (@c BME280_MODE_FORCED), and writes it back. 
 * This safely switches the sensor out of sleep mode while preserving any existing 
 * temperature and pressure oversampling configurations.
 *
 * @note In forced mode, the sensor performs a single measurement sequence according 
 *       to the selected oversampling settings and then automatically returns to sleep mode.
 *
 * @param[in] dev Pointer to the BME280 device handle containing I2C details.
 *
 * @retval HAL_OK    The sensor was successfully switched to forced measurement mode.
 * @retval HAL_ERROR The I2C read or write transaction failed.
 ******************************************************************************/
HAL_StatusTypeDef BME280_Wakeup(BME280_Handle_t *dev);

/*******************************************************************************
 * BME280_CheckID()
 *
 * @brief Verifies the BME280 sensor's unique chip identifier.
 *
 * This function reads the device identification register and compares the retrieved
 * value against the expected factory chip ID constant (@c BME280_CHIP_ID). It is typically
 * used during system startup to ensure proper wiring and correct slave addressing.
 *
 * @param[in] dev Pointer to the BME280 device handle containing I2C details.
 *
 * @retval HAL_OK    The sensor was successfully identified and matches the expected chip ID.
 * @retval HAL_ERROR The I2C read transaction failed, or the retrieved ID does not match.
 ******************************************************************************/
HAL_StatusTypeDef BME280_CheckID(BME280_Handle_t *dev);

/*******************************************************************************
 * BME280_ReadRaw()
 *
 * @brief Reads the uncompensated raw ADC data from the BME280 sensor.
 *
 * This function performs an 8-byte burst read starting from the pressure MSB register
 * to retrieve the raw pressure, temperature, and humidity ADC values in a single 
 * transaction. It then combines the individual data bytes into their respective 
 * 20-bit (pressure/temperature) and 16-bit (humidity) raw integer fields.
 *
 * @note Performing a burst read ensures that all data fields belong to the same 
 *       measurement cycle, preventing data inconsistency bugs.
 *
 * @param[in]  dev Pointer to the BME280 device handle containing I2C details.
 * @param[out] pData Pointer to the output structure where the raw ADC values will be stored.
 *
 * @retval HAL_OK    The device was verified and the raw data was successfully read and parsed.
 * @retval HAL_ERROR The device ID check failed, or the I2C burst read transaction failed.
 ******************************************************************************/
HAL_StatusTypeDef BME280_ReadRaw(BME280_Handle_t *dev, BME280_RawData_t *pData);

/*******************************************************************************
 * BME280_ReadCalibration()
 *
 * @brief Reads and parses the factory calibration coefficients from the sensor.
 *
 * This function reads the non-contiguous calibration register blocks from the BME280.
 * It performs two separate I2C memory reads: a 25-byte burst read starting at register
 * @c 0x88 (for temperature, pressure, and @c dig_H1), and a 7-byte burst read starting
 * at register @c 0xE1 (for the remaining humidity coefficients). The raw bytes are 
 * then combined, correctly sign-extended, and stored into the device handle's 
 * internal calibration structure.
 *
 * @note This function must be executed successfully before attempting to run any 
 *       data compensation algorithms (converting raw ADC readings to physical units).
 *
 * @param[in,out] dev Pointer to the BME280 device handle where the parsed coefficients 
 *                    will be stored.
 *
 * @retval HAL_OK    All calibration data was successfully retrieved and parsed.
 * @retval HAL_ERROR An I2C communication error occurred during one of the reads.
 ******************************************************************************/
HAL_StatusTypeDef BME280_ReadCalibration(BME280_Handle_t *dev);

/*******************************************************************************
 * BME280_ReadTemperature()
 *
 * @brief Reads the current temperature from the sensor.
 *
 * This function triggers a fresh raw burst read from the sensor, extracts the
 * raw temperature ADC value, and processes it using the internal compensation math.
 * It also populates the intermediate @c t_fine variable inside the device handle.
 *
 * @param[in,out] dev  Pointer to the BME280 device handle where @c t_fine will be updated.
 * @param[out]    pData Pointer to the floating-point variable where the temperature (°C) will be stored.
 *
 * @retval HAL_OK    The temperature was successfully read and compensated.
 * @retval HAL_ERROR The underlying I2C raw read transaction failed.
 ******************************************************************************/
HAL_StatusTypeDef BME280_ReadTemperature(BME280_Handle_t *dev, float *pData);

/*******************************************************************************
 * BME280_ReadPressure()
 *
 * @brief Reads the current barometric pressure from the sensor.
 *
 * This function captures a raw burst read from the device, calculates the updated
 * temperature to refresh the internal @c t_fine variable, and applies the mathematical 
 * compensation formulas to evaluate the true atmospheric pressure.
 *
 * @note Because pressure compensation directly relies on temperature calculations, this 
 *       function implicitly calculates and refreshes the device's internal temperature calibration state.
 *
 * @param[in,out] dev   Pointer to the BME280 device handle where @c t_fine will be updated.
 * @param[out]    pData Pointer to the floating-point variable where the pressure (Pa) will be stored.
 *
 * @retval HAL_OK    The pressure was successfully read and compensated.
 * @retval HAL_ERROR The underlying I2C raw read transaction failed.
 ******************************************************************************/
HAL_StatusTypeDef BME280_ReadPressure(BME280_Handle_t *dev, float *pData);

/*******************************************************************************
 * BME280_ReadHumidity()
 *
 * @brief Reads the current relative humidity from the sensor.
 *
 * This function gathers a raw burst read from the device, calculates the updated 
 * temperature to refresh the internal @c t_fine variable, and applies the calibration 
 * coefficients to evaluate the relative humidity percentage.
 *
 * @note Because humidity compensation directly relies on temperature calculations, this 
 *       function implicitly calculates and refreshes the device's internal temperature calibration state.
 *
 * @param[in,out] dev Pointer to the BME280 device handle where @c t_fine will be updated.
 * @param[out]    pData Pointer to the floating-point variable where the humidity (%RH) will be stored.
 *
 * @retval HAL_OK    The humidity was successfully read and compensated.
 * @retval HAL_ERROR The underlying I2C raw read transaction failed.
 ******************************************************************************/
HAL_StatusTypeDef BME280_ReadHumidity(BME280_Handle_t *dev, float *pData);

/*******************************************************************************
 * BME280_ReadAll()
 *
 * @brief Reads all environmental measurements (Temperature, Pressure, Humidity) simultaneously.
 *
 * This is the preferred, high-efficiency function for collecting complete environmental datasets. 
 * It issues a single, unified 8-byte I2C burst read to capture all physical values from a single 
 * sample frame. It then executes the temperature math first to properly cache @c t_fine, before safely 
 * executing the sequential pressure and humidity compensation pipelines.
 *
 * @note Using this function is faster and more resource-efficient than calling individual 
 *       read functions sequentially, as it removes redundant I2C transactional overhead.
 *
 * @param[in,out] dev Pointer to the BME280 device handle where @c t_fine will be updated.
 * @param[out]    pData Pointer to the output structure where all physical data readings will be saved.
 *
 * @retval HAL_OK    All measurements were successfully pulled, calculated, and stored.
 * @retval HAL_ERROR The underlying I2C raw read transaction failed.
 ******************************************************************************/
HAL_StatusTypeDef BME280_ReadAll(BME280_Handle_t *dev, BME280_Data_t *pData);

#ifdef __cplusplus
}
#endif

#endif /* BME280_H */
