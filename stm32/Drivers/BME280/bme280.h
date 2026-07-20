/**
  ******************************************************************************
  * @file           : bme280.h
  * @brief          : Header file for BME280 sensor driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#ifndef BME280_H
#define BME280_H

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

#define BME280_REG_PRESS_MSB   0xF7  // Burst read start
#define BME280_REG_TEMP_MSB    0xFA
#define BME280_REG_HUM_MSB     0xFD

#define BME280_CHIP_ID         0x60

#define BME280_ADDRESS 0xEC // Address determined by SDO pin

/*
 * ============================================================================
 *                     	  ##### TYPE DEFINITIONS #####
 * ============================================================================
 */

typedef enum
{
    BME280_OK          = 0,  // Operation completed successfully.
    BME280_ERR_I2C     = 1,  // Physical bus communication failure (I2C NACK or timeout).
    BME280_ERR_ID      = 2,  // Device ID mismatch (Chip is not a BME280).
    BME280_ERR_RESET   = 3   // Sensor soft-reset failed or timed out.
} BME280_Status_t;

typedef struct
{
    uint32_t osrs_t  : 3;  // Bits 0..2
    uint32_t osrs_p  : 3;  // Bits 3..5
    uint32_t osrs_h  : 3;  // Bits 6..8
    uint32_t mode    : 2;  // Bits 9..10
    uint32_t standby : 3;  // Bits 11..13
    uint32_t filter  : 3;  // Bits 14..16
                           // Bits 17..31 are automatic padding (15 bits)
} BME280_Config_t;         // Total size: Exactly 4 bytes (1 word)

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
    // Do not alter the order of this data structure
    // The memory packing is properly aligned

    I2C_HandleTypeDef *hi2c;
    int32_t t_fine;

    // Calibration data (filled at init)
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

        
        int16_t  dig_H2;
        int16_t  dig_H4;
        int16_t  dig_H5;
        uint8_t  dig_H1;
        uint8_t  dig_H3;
        int8_t   dig_H6;
    } calib;

    uint8_t address;

} BME280_Handle_t;

/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

/*******************************************************************************
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
 * @param[in]     cfg Configuration structure holding sensor settings.
 *
 * @retval BME280_OK        Sensor successfully initialized and configured.
 * @retval BME280_ERR_ID    Device chip ID verification failed.
 * @retval BME280_ERR_RESET Sensor software reset sequence timed out or failed.
 * @retval BME280_ERR_I2C   Physical I2C bus communications failed.
 ******************************************************************************/
BME280_Status_t BME280_Init(BME280_Handle_t *dev, BME280_Config_t cfg);

/*******************************************************************************
 * @brief Places the BME280 sensor into a low-power sleep state.
 *
 * @param[in,out] dev Pointer to the BME280 device handle.
 * @retval BME280_OK      Device entered sleep mode successfully.
 * @retval BME280_ERR_I2C Physical I2C bus communications failed.
 ******************************************************************************/
BME280_Status_t BME280_Sleep(BME280_Handle_t *dev);

/*******************************************************************************
 * @brief Wakes up the sensor by putting it into forced measurement mode.
 *
 * @param[in,out] dev Pointer to the BME280 device handle.
 * @retval BME280_OK      Device forced measurement mode triggered.
 * @retval BME280_ERR_I2C Physical I2C bus communications failed.
 ******************************************************************************/
BME280_Status_t BME280_Wakeup(BME280_Handle_t *dev);

/*******************************************************************************
 * @brief Reads and calculates the compensated temperature.
 *
 * @param[in,out] dev   Pointer to the BME280 device handle.
 * @param[out]    pData Pointer to float destination container.
 * @retval BME280_OK    Temperature successfully captured.
 * @retval BME280_ERR_I2C Physical I2C bus communications failed.
 ******************************************************************************/
BME280_Status_t BME280_ReadTemperature(BME280_Handle_t *dev, float *pData);

/*******************************************************************************
 * @brief Reads and calculates the compensated barometric pressure.
 *
 * @param[in,out] dev   Pointer to the BME280 device handle.
 * @param[out]    pData Pointer to float destination container.
 * @retval BME280_OK    Pressure successfully captured.
 * @retval BME280_ERR_I2C Physical I2C bus communications failed.
 ******************************************************************************/
BME280_Status_t BME280_ReadPressure(BME280_Handle_t *dev, float *pData);

/*******************************************************************************
 * @brief Reads and calculates the compensated relative humidity percentage.
 *
 * @param[in,out] dev   Pointer to the BME280 device handle.
 * @param[out]    pData Pointer to float destination container.
 * @retval BME280_OK    Humidity successfully captured.
 * @retval BME280_ERR_I2C Physical I2C bus communications failed.
 ******************************************************************************/
BME280_Status_t BME280_ReadHumidity(BME280_Handle_t *dev, float *pData);

/*******************************************************************************
 * @brief Reads and updates all three sensor parameters simultaneously.
 *
 * @param[in,out] dev   Pointer to the BME280 device handle.
 * @param[out]    pData Pointer to the collective telemetry struct container.
 * @retval BME280_OK    All telemetry metrics successfully captured.
 * @retval BME280_ERR_I2C Physical I2C bus communications failed.
 ******************************************************************************/
BME280_Status_t BME280_ReadAll(BME280_Handle_t *dev, BME280_Data_t *pData);

#ifdef __cplusplus
}
#endif

#endif /* BME280_H */
