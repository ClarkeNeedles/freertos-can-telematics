/**
  ******************************************************************************
  * @file           : can_bus.c
  * @brief          : can bus driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#include "can_bus.h"

/*
 * ============================================================================
 *                  ##### PRIVATE FUNCTION PROTOTYPES  #####
 * ============================================================================
 */


 
/*
 * ============================================================================
 *                         ##### PUBLIC API #####
 * ============================================================================
 */

// -----------------------------------------------------------------------------
CAN_Bus_Status_t CAN_Bus_Init(CAN_Bus_Handle_t *dev)
{
    CAN_FilterTypeDef filterConfig;

    // Configure the Acceptance Filter (Allows all incoming messages by default)
    filterConfig.FilterBank           = 0;
    filterConfig.FilterMode           = CAN_FILTERMODE_IDMASK;
    filterConfig.FilterScale          = CAN_FILTERSCALE_32BIT;
    filterConfig.FilterIdHigh         = 0x0000;
    filterConfig.FilterIdLow          = 0x0000;
    filterConfig.FilterMaskIdHigh     = 0x0000;
    filterConfig.FilterMaskIdLow      = 0x0000;
    filterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    filterConfig.FilterActivation     = ENABLE;
    filterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(dev->hcan, &filterConfig) != HAL_OK)
    {
        return CAN_BUS_ERROR;
    }

    // Start the internal CAN hardware peripheral
    if (HAL_CAN_Start(dev->hcan) != HAL_OK)
    {
        return CAN_BUS_ERROR;
    }

    // Set transceiver to normal high-speed operation mode
    HAL_GPIO_WritePin(dev->rs_port, dev->rs_pin, GPIO_PIN_SET);

    return CAN_BUS_OK;
}

// -----------------------------------------------------------------------------
CAN_Bus_Status_t CAN_Bus_Transmit(CAN_Bus_Handle_t *dev, CAN_Message_t *msg)
{
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;

    txHeader.StdId              = msg->is_extended ? 0 : msg->id;
    txHeader.ExtId              = msg->is_extended ? msg->id : 0;
    txHeader.IDE                = msg->is_extended ? CAN_ID_EXT : CAN_ID_STD;
    txHeader.RTR                = CAN_RTR_DATA;
    txHeader.DLC                = msg->length;
    txHeader.TransmitGlobalTime = DISABLE;

    // Block safely until at least one transmit mailbox becomes empty
    while (HAL_CAN_GetTxMailboxesFreeLevel(dev->hcan) == 0);

    if (HAL_CAN_AddTxMessage(dev->hcan, &txHeader, msg->data, &txMailbox) != HAL_OK)
    {
        return CAN_BUS_ERROR;
    }

    return CAN_BUS_OK;
}

// -----------------------------------------------------------------------------
CAN_Bus_Status_t CAN_Bus_Receive(CAN_Bus_Handle_t *dev, CAN_Message_t *msg)
{
    // Check if any messages are currently waiting in FIFO 0
    if (HAL_CAN_GetRxFifoFillLevel(dev->hcan, CAN_RX_FIFO0) == 0)
    {
        return CAN_BUS_EMPTY; // Return FIFO is empty
    }

    CAN_RxHeaderTypeDef rxHeader;

    if (HAL_CAN_GetRxMessage(dev->hcan, CAN_RX_FIFO0, &rxHeader, msg->data) != HAL_OK)
    {
        return CAN_BUS_ERROR;
    }

    msg->id          = (rxHeader.IDE == CAN_ID_EXT) ? rxHeader.ExtId : rxHeader.StdId;
    msg->is_extended = (rxHeader.IDE == CAN_ID_EXT) ? 1 : 0;
    msg->length      = rxHeader.DLC;

    return CAN_BUS_OK;
}

/*
 * ============================================================================
 *                ##### PRIVATE FUNCTION IMPLEMENTATIONS #####
 * ============================================================================
 */

