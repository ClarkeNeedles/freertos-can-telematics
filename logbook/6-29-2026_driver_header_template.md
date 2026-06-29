---
title: "Driver header file template"
date: 6-29-2026
status: in-progress
version: 0.1.1
---

This is a template header file that I should use for all of my drivers

```
/**
  ******************************************************************************
  * @file           : driver_template.h
  * @brief          : Header file for MODULE_NAME driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#ifndef DRIVER_NAME_H
#define DRIVER_NAME_H

#include "stm32f4xx_hal.h"  /* Replace with your exact MCU HAL target */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 *                     	##### DRIVER_NAME DEFINITIONS #####
 * ============================================================================
 */

#define DRIVER_NAME_EXAMPLE_CONFIG_VAL    0x01


/*
 * ============================================================================
 *                     	##### DRIVER_NAME REGISTER MAPS #####
 * ============================================================================
 */

#define DRIVER_NAME_REG_WHO_AM_I          0x00  /**< Device Identification Register */


/*
 * ============================================================================
 *                     	  ##### TYPE DEFINITIONS #####
 * ============================================================================
 */

/**
 * @brief Configuration configuration parameters for the peripheral.
 */
typedef struct 
{
    uint8_t mode;  /**< Operating runtime mode configuration */
} DRIVER_NAME_Config_t;

/**
 * @brief Device structure holding peripheral interface bus details.
 */
typedef struct 
{
    I2C_HandleTypeDef    *hi2c;    /**< Pointer to HAL communication bus handle */
    uint16_t              address; /**< Hardware slave device bus address */
    DRIVER_NAME_Config_t  config;  /**< Active runtime device settings profile */
} DRIVER_NAME_Handle_t;


/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

/**
 * @brief  Initializes the hardware module with designated configurations.
 *
 * @param[in,out] dev Pointer to the device configuration state handle structure.
 * @retval HAL_OK    Peripheral was successfully initialized and verified.
 * @retval HAL_ERROR Initialization validation check failed on the hardware bus.
 */
HAL_StatusTypeDef DRIVER_NAME_Init(DRIVER_NAME_Handle_t *dev);


#ifdef __cplusplus
}
#endif

#endif /* DRIVER_NAME_H */
```
