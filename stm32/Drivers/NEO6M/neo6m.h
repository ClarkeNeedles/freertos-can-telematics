/**
  ******************************************************************************
  * @file           : neo6m.h
  * @brief          : Header file for NEO-6M sensor driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#ifndef NEO6M_H
#define NEO6M_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <usart.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 *                     	##### NEO6M DEFINITIONS #####
 * ============================================================================
 */

#define GPSBUFSIZE  128   // GPS buffer size

/*
 * ============================================================================
 *                     	  ##### TYPE DEFINITIONS #####
 * ============================================================================
 */
    
typedef struct
{
    // Calculated values
    float dec_longitude;
    float dec_latitude;

    // Status flag: 1 = Valid satellite fix ('A'), 0 = No lock ('V')
    uint8_t valid_fix;
} NEO6M_Data_t;

typedef struct
{
    UART_HandleTypeDef *huart;

    // Receive buffer
    uint8_t rx_data = 0;
    uint8_t rx_buffer[GPSBUFSIZE];
    uint8_t rx_index = 0;

    NEO6M_Data_t gps_data;
} NEO6M_Handle_t;

/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

/**
 * @brief Initializes the NEO-6M GPS receiver driver structure.
 *
 * This function prepares the driver handle context and arms the low-level STM32
 * UART peripheral hardware to listen for incoming characters. It triggers the
 * first asynchronous background byte capture sequence using non-blocking interrupts.
 *
 * @param[in,out] dev Pointer to the master NEO6M handle structure to initialize.
 *
 * @retval HAL_OK    The background interrupt listener armed successfully.
 * @retval HAL_ERROR The underlying HAL UART interrupt register subsystem failed to arm.
 */
HAL_StatusTypeDef NEO6M_Init(NEO6M_Handle_t *dev);


HAL_StatusTypeDef NEO6M_UART_CallBack(NEO6M_Handle_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* NEO6M_H */
