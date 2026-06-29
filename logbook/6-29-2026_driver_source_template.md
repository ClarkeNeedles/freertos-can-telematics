---
title: "Driver source file template"
date: 6-29-2026
status: in-progress
version: 0.1.1
---

This is a template source file that I should use for all of my drivers

```
/**
  ******************************************************************************
  * @file           : driver_template.c
  * @brief          : Source file for MODULE_NAME driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#include "driver_template.h"

/*
 * ============================================================================
 *                  ##### PRIVATE FUNCTION PROTOTYPES  #####
 * ============================================================================
 */

static void DRIVER_NAME_LowLevelWrite(DRIVER_NAME_Handle_t *dev, uint8_t register_addr, uint8_t value);


/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

// -----------------------------------------------------------------------------
HAL_StatusTypeDef DRIVER_NAME_Init(DRIVER_NAME_Handle_t *dev)
{
    // Defensive verification check
    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    // Example sequence using the low-level private function
    DRIVER_NAME_LowLevelWrite(dev, DRIVER_NAME_REG_WHO_AM_I, DRIVER_NAME_EXAMPLE_CONFIG_VAL);

    return HAL_OK;
}


/*
 * ============================================================================
 *                ##### PRIVATE FUNCTION IMPLEMENTATIONS #####
 * ============================================================================
 */

/**
 * @brief  Low-level utility to write a value to a hardware register.
 * @note   Internal utility; handles direct peripheral register modification.
 *
 * @param[in,out] dev           Pointer to the hardware handle.
 * @param[in]     register_addr Target internal register address location.
 * @param[in]     value         Byte data value payload to transmit.
 */
static void DRIVER_NAME_LowLevelWrite(DRIVER_NAME_Handle_t *dev, uint8_t register_addr, uint8_t value)
{
    uint8_t buffer[2] = { register_addr, value };

    // Example I2C transaction framework matching your hardware handle style
    HAL_I2C_Master_Transmit(dev->hi2c, dev->address, buffer, 2, HAL_MAX_DELAY);
}
```