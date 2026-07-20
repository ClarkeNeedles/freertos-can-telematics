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
 *                  ##### PRIVATE FUNCTION PROTOTYPES  #####
 * ============================================================================
 */

static float BME280_CompensateTemp(BME280_Handle_t *dev, int32_t adc_T);
static float BME280_CompensatePress(BME280_Handle_t *dev, int32_t adc_P);
static float BME280_CompensateHum(BME280_Handle_t *dev, int32_t adc_H);
static BME280_Status_t BME280_CheckID(BME280_Handle_t *dev);
static BME280_Status_t BME280_Reset(BME280_Handle_t *dev);
static BME280_Status_t BME280_ReadRaw(BME280_Handle_t *dev, BME280_RawData_t *pData);
static BME280_Status_t BME280_ReadCalibration(BME280_Handle_t *dev);

/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

// -----------------------------------------------------------------------------
BME280_Status_t BME280_Init(BME280_Handle_t *dev, BME280_Config_t cfg)
{
    BME280_Status_t status;
    uint8_t config;

    status = BME280_CheckID(dev);
    if (status != BME280_OK)
    {
        return status;
    }

    status = BME280_Reset(dev);
    if (status != BME280_OK)
    {
        return status;
    }

    status = BME280_ReadCalibration(dev);
    if (status != BME280_OK)
    {
        return status;
    }

    // Humidity oversampling must be set first
    config = (uint8_t) cfg.osrs_h
    if (HAL_I2C_Mem_Write(dev->hi2c, dev->address,
            BME280_REG_CTRL_HUM, I2C_MEMADD_SIZE_8BIT,
            &config, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

    // Config register
    config = (uint8_t) (cfg.standby << 5) | (cfg.filter << 2);
    if (HAL_I2C_Mem_Write(dev->hi2c, dev->address,
            BME280_REG_CONFIG, I2C_MEMADD_SIZE_8BIT,
            &config, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

    // Control measurement register
    config = (uint8_t) (cfg.osrs_t << 5) | (cfg.osrs_p << 2) | (cfg.mode);
    if (HAL_I2C_Mem_Write(dev->hi2c, dev->address,
            BME280_REG_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT,
            &ctrl_meas, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

    return BME280_OK;
}

// -----------------------------------------------------------------------------
BME280_Status_t BME280_Sleep(BME280_Handle_t *dev)
{ 
    uint8_t mode = BME280_MODE_SLEEP;

    if (HAL_I2C_Mem_Write(dev->hi2c, dev->address,
                          BME280_REG_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT,
                          &mode, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

    return BME280_OK;
}

// -----------------------------------------------------------------------------
BME280_Status_t BME280_Wakeup(BME280_Handle_t *dev)
{
    uint8_t ctrl;

    if (HAL_I2C_Mem_Read(dev->hi2c, dev->address,
            BME280_REG_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT,
            &ctrl, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

    ctrl |= BME280_MODE_FORCED;

    if (HAL_I2C_Mem_Write(dev->hi2c, dev->address,
                            BME280_REG_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT,
                            &ctrl, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

    return BME280_OK;
}

// -----------------------------------------------------------------------------
BME280_Status_t BME280_ReadTemperature(BME280_Handle_t *dev, float *pData)
{
    BME280_RawData_t raw;
    if (BME280_ReadRaw(dev, &raw) != BME280_OK)
    {
        return BME280_ERR_I2C;
    }

    *pData = BME280_CompensateTemp(dev, raw.raw_temp);

    return BME280_OK;
}

// -----------------------------------------------------------------------------
BME280_Status_t BME280_ReadPressure(BME280_Handle_t *dev, float *pData)
{
    BME280_RawData_t raw;
    if (BME280_ReadRaw(dev, &raw) != BME280_OK)
    {
        return BME280_ERR_I2C;
    }

    *pData = BME280_CompensatePress(dev, raw.raw_press);

    return BME280_OK;
}

// -----------------------------------------------------------------------------
BME280_Status_t BME280_ReadHumidity(BME280_Handle_t *dev, float *pData)
{
    BME280_RawData_t raw;
    if (BME280_ReadRaw(dev, &raw) != BME280_OK)
    {
        return BME280_ERR_I2C;
    }

    *pData = BME280_CompensateHum(dev, raw.raw_hum);

    return BME280_OK;
}

// -----------------------------------------------------------------------------
BME280_Status_t BME280_ReadAll(BME280_Handle_t *dev, BME280_Data_t *pData)
{
    BME280_RawData_t raw;
    if (BME280_ReadRaw(dev, &raw) != BME280_OK)
    {
        return BME280_ERR_I2C;
    }

    pData->temperature_c = BME280_CompensateTemp(dev, raw.raw_temp);
    pData->pressure_pa   = BME280_CompensatePress(dev, raw.raw_press);
    pData->humidity_pct  = BME280_CompensateHum(dev, raw.raw_hum);

    return BME280_OK;
}

/*
 * ============================================================================
 *                ##### PRIVATE FUNCTION IMPLEMENTATIONS #####
 * ============================================================================
 */

/*******************************************************************************
 * @brief  Compensates the raw temperature ADC reading into degrees Celsius.
 * @note   This is an internal utility function used exclusively within this file.
 *
 * This function implements the official Bosch compensation formula using double-precision
 * floating-point math. It converts the 20-bit raw temperature ADC value into actual 
 * degrees Celsius and computes the intermediate @c t_fine value, which must be saved 
 * to the device handle because it is required for pressure and humidity compensation.
 *
 * @param[in,out] dev   Pointer to the BME280 device handle where @c t_fine will be cached.
 * @param[in]     adc_T Raw 20-bit temperature value read from the sensor's ADC.
 *
 * @return The compensated temperature in degrees Celsius (°C).
 ******************************************************************************/
static float BME280_CompensateTemp(BME280_Handle_t *dev, int32_t adc_T)
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

/*******************************************************************************
 * @brief  Compensates the raw pressure ADC reading into Pascals.
 * @note   This is an internal utility function used exclusively within this file.
 * @note   This function relies on @c dev->t_fine. Therefore, @c BME280_CompensateTemp() 
 *         must be called prior to executing this function.
 *
 * This function implements the official Bosch compensation formula using double-precision
 * floating-point math. It converts the 20-bit raw pressure ADC value into a calibrated
 * value in Pascals. It gracefully handles the physical edge case where a corrupted or 
 * uninitialized calibration parameter (@c dig_P1) could otherwise cause a crash.
 *
 * @param[in] dev   Pointer to the BME280 device handle containing calibration data and @c t_fine.
 * @param[in] adc_P Raw 20-bit pressure value read from the sensor's ADC.
 *
 * @return The compensated pressure in Pascals (Pa), or @c 0.0 if a division-by-zero 
 *         condition is detected.
 ******************************************************************************/
static float BME280_CompensatePress(BME280_Handle_t *dev, int32_t adc_P)
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
    {
        return 0;
    }

    p = 1048576.0 - adc_P;
    p = (p - var2 / 4096.0) * 6250.0 / var1;
    var1 = dev->calib.dig_P9 * p * p / 2147483648.0;
    var2 = p * dev->calib.dig_P8 / 32768.0;

    return p + (var1 + var2 + dev->calib.dig_P7) / 16.0;
}

/*******************************************************************************
 * @brief  Compensates the raw humidity ADC reading into percentage Relative Humidity.
 * @note   This is an internal utility function used exclusively within this file.
 * @note   This function relies on @c dev->t_fine. Therefore, @c BME280_CompensateTemp() 
 *         must be called prior to executing this function.
 *
 * This function implements the official Bosch compensation formula using double-precision
 * floating-point math. It converts the 16-bit raw humidity ADC value into Relative Humidity. 
 * The mathematical output is automatically clamped to ensure it remains within the logical 
 * physical range of 0.0% to 100.0%.
 *
 * @param[in] dev   Pointer to the BME280 device handle containing calibration data and @c t_fine.
 * @param[in] adc_H Raw 16-bit humidity value read from the sensor's ADC.
 *
 * @return The compensated relative humidity as a percentage (%RH), clamped between @c 0.0 and @c 100.0.
 ******************************************************************************/
static float BME280_CompensateHum(BME280_Handle_t *dev, int32_t adc_H)
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
    {
        h = 100.0;
    }

    if (h < 0.0)
    {
        h = 0.0;
    }

    return h;
}

/*******************************************************************************
 * @brief Verifies the BME280 sensor's unique chip identifier.
 *
 * This function reads the device identification register and compares the retrieved
 * value against the expected factory chip ID constant (@c BME280_CHIP_ID). It is typically
 * used during system startup to ensure proper wiring and correct slave addressing.
 *
 * @param[in] dev Pointer to the BME280 device handle containing I2C details.
 *
 * @retval BME280_OK The sensor was successfully identified and matches the expected chip ID.
 * @retval BME280_ERR_I2C The I2C read transaction failed, or the retrieved ID does not match.
 * @retval BME280_ERR_ID The chip id was not correct
 ******************************************************************************/
static BME280_Status_t BME280_CheckID(BME280_Handle_t *dev)
{
    uint8_t id = 0;

    if (HAL_I2C_Mem_Read(dev->hi2c, dev->address,
            BME280_REG_ID, I2C_MEMADD_SIZE_8BIT,
            &id, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

    if (id != BME280_CHIP_ID)
    {
        return BME280_ERR_ID;
    }

    return BME280_OK;
}

/*******************************************************************************
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
 * @retval BME280_OK    The reset command was successfully sent and the startup delay has expired.
 * @retval BME280_ERR_I2C The I2C write transaction failed.
 ******************************************************************************/
static BME280_Status_t BME280_Reset(BME280_Handle_t *dev)
{
    if (HAL_I2C_Mem_Write(dev->hi2c, dev->address,
            BME280_REG_RESET, I2C_MEMADD_SIZE_8BIT,
            (uint8_t *)0xB6, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

    HAL_Delay(100);
    return BME280_OK;
}

/*******************************************************************************
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
 * @retval BME280_OK The device was verified and the raw data was successfully read and parsed.
 * @retval BME280_ERR_I2C The device ID check failed, or the I2C burst read transaction failed.
 ******************************************************************************/
static BME280_Status_t BME280_ReadRaw(BME280_Handle_t *dev, BME280_RawData_t *pData)
{
    uint8_t data[8];

    if (HAL_I2C_Mem_Read(dev->hi2c, dev->address,
            BME280_REG_PRESS_MSB, I2C_MEMADD_SIZE_8BIT,
            data, 8, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

    pData->raw_press = (int32_t)((data[0] << 12) | (data[1] << 4) | (data[2] >> 4));
    pData->raw_temp = (int32_t)((data[3] << 12) | (data[4] << 4) | (data[5] >> 4));
    pData->raw_hum = (int32_t)((data[6] << 8) | data[7]);

    return BME280_OK;
}

/*******************************************************************************
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
 * @retval BME280_OK All calibration data was successfully retrieved and parsed.
 * @retval BME280_ERR_I2C An I2C communication error occurred during one of the reads.
 ******************************************************************************/
static BME280_Status_t BME280_ReadCalibration(BME280_Handle_t *dev)
{
    uint8_t calib[32];
    uint8_t calib2[7];

    if (HAL_I2C_Mem_Read(dev->hi2c, dev->address,
            0x88, I2C_MEMADD_SIZE_8BIT,
            calib, 25, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

    if (HAL_I2C_Mem_Read(dev->hi2c, dev->address,
            0xE1, I2C_MEMADD_SIZE_8BIT,
            calib2, 7, HAL_MAX_DELAY) != HAL_OK)
    {
        return BME280_ERR_I2C;
    }

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

    return BME280_OK;
}
