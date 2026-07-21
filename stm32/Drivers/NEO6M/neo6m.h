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

typedef enum
{
  NEO6M_OK             = 0,  // Character appended or sentence successfully committed.
  NEO6M_ERR_UART       = 1,  // Native STM32 HAL UART peripheral communication failure.
  NEO6M_ERR_VALIDATION = 2,  // NMEA packet failed checksum validation metrics.
  NEO6M_ERR_NO_FIX     = 3   // Sentence parsed cleanly but GPS reports no active fix.
} NEO6M_Status_t;

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

    NEO6M_Data_t gps_data;

    // Receive buffer
    uint8_t rx_data;
    uint8_t rx_buffer[GPSBUFSIZE];
    uint8_t rx_index;
} NEO6M_Handle_t;

/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

/*******************************************************************************
 * @brief Initializes the NEO-6M GPS driver state structures.
 *
 * This function clears internal index markers, purges buffer histories, and activates
 * non-blocking asynchronous UART character streaming reception using hardware interrupts.
 *
 * @param[in,out] dev Pointer to the NEO-6M driver handle containing peripheral details.
 *
 * @retval NEO6M_OK       Interrupt streaming initialized successfully.
 * @retval NEO6M_ERR_UART Internal hardware UART peripheral rejected the request.
 ******************************************************************************/
NEO6M_Status_t NEO6M_Init(NEO6M_Handle_t *dev);

/*******************************************************************************
 * @brief Processes bytes received inside the native UART RX interrupt routine.
 *
 * This function buffers characters sequentially until a newline token is caught.
 * Upon string completion, it hashes structural checksums, executes string extractions,
 * maps coordinate telemetry fields, and re-arms the underlying UART interrupt handler.
 *
 * @param[in,out] dev Pointer to the NEO-6M driver handle containing peripheral details.
 *
 * @retval NEO6M_OK             Sentence parsed successfully or byte appended.
 * @retval NEO6M_ERR_VALIDATION Structural sentence validation hash failed.
 * @retval NEO6M_ERR_NO_FIX     Sentence parsed but GPS has lost satellite lock.
 * @retval NEO6M_ERR_UART       Asynchronous UART re-arm execution failed.
 ******************************************************************************/
NEO6M_Status_t NEO6M_UART_CallBack(NEO6M_Handle_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* NEO6M_H */
