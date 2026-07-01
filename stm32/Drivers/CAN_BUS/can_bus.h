/**
  ******************************************************************************
  * @file           : can_bus.h
  * @brief          : Header file for can bus driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#ifndef CAN_BUS_H
#define CAN_BUS_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 *                       ##### CAN_BUS DEFINITIONS #####
 * ============================================================================
 */



/*
 * ============================================================================
 *                        ##### TYPE DEFINITIONS #####
 * ============================================================================
 */

typedef enum
{
    CAN_BUS_OK       = 0,  // Operation succeeded
    CAN_BUS_EMPTY    = 1,  // No data available (Expected state)
    CAN_BUS_ERROR    = 2,  // Hardware error or peripheral failure
    CAN_BUS_BUSY     = 3   // Mailboxes full or bus heavily congested
} CAN_Bus_Status_t;

typedef struct
{
    uint32_t id;          // Standard (11-bit) or Extended (29-bit) ID
    uint8_t  is_extended; // 0 = Standard, 1 = Extended
    uint8_t  length;      // Data length (0 to 8 bytes)
    uint8_t  data[8];     // Payload buffer
} CAN_Message_t;

typedef struct
{
    CAN_HandleTypeDef *hcan;
    GPIO_TypeDef *rs_port;   // GPIO Port for RS Pin (Set to NULL if tied to GND)
    uint16_t rs_pin;         // GPIO Pin for RS Pin (Set to 0 if tied to GND)
} CAN_Bus_Handle_t;

/*
 * ============================================================================
 *                         ##### PUBLIC API #####
 * ============================================================================
 */

/*******************************************************************************
 * @brief Initializes CAN filters, starts the peripheral, and wakes up transceiver.
 *
 * This function configures the hardware acceptance filters to accept all incoming
 * messages by default, starts the internal STM32 CAN controller, and puts the
 * external SN65HVD230 transceiver into normal high-speed operation mode.
 *
 * @param[in,out] dev Pointer to the CAN bus handle containing hardware instances.
 *
 * @retval CAN_BUS_OK    Peripheral started and filter configured successfully.
 * @retval CAN_BUS_ERROR Filter configuration failed or CAN peripheral failed to start.
 ******************************************************************************/
CAN_Bus_Status_t CAN_Bus_Init(CAN_Bus_Handle_t *dev);

/*******************************************************************************
 * @brief Transmits a standard or extended CAN frame.
 *
 * This function packages the driver message structure into an ST HAL transmit
 * header, blocks safely until a hardware transmit mailbox is available, and
 * requests transmission on the physical bus lines.
 *
 * @param[in,out] dev Pointer to the CAN bus handle containing hardware instances.
 * @param[in]     msg Pointer to the outgoing message structure containing payload.
 *
 * @retval CAN_BUS_OK    Message successfully placed into a hardware mailbox.
 * @retval CAN_BUS_ERROR Internal HAL peripheral failed to add the message.
 ******************************************************************************/
CAN_Bus_Status_t CAN_Bus_Transmit(CAN_Bus_Handle_t *dev, CAN_Message_t *msg);

/*******************************************************************************
 * @brief Polls and fetches an incoming message from RX FIFO 0.
 *
 * This function checks the hardware fill level of the first internal receive FIFO.
 * If a message is waiting, it unpacks the hardware headers and safely parses the
 * payload data back out to the user application structure.
 *
 * @param[in,out] dev Pointer to the CAN bus handle containing hardware instances.
 * @param[out]    msg Pointer to the container structure where received data is written.
 *
 * @retval CAN_BUS_OK    Message successfully retrieved and written to the container.
 * @retval CAN_BUS_EMPTY No messages are currently waiting in the hardware FIFO.
 * @retval CAN_BUS_ERROR Internal HAL peripheral failed to extract the message.
 ******************************************************************************/
CAN_Bus_Status_t CAN_Bus_Receive(CAN_Bus_Handle_t *dev, CAN_Message_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* CAN_BUS_H */
