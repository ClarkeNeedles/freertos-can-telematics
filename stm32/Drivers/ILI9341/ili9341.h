/**
  ******************************************************************************
  * @file           : ili9341.h
  * @brief          : Header file for ILI9341 display driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#ifndef ILI9341_H
#define ILI9341_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 *                     	   ##### ILI9341 DEFINITIONS #####
 * ============================================================================
 */

// Swap layout dimensions for landscape mode
#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240

/*
 * ============================================================================
 *                     	 ##### ILI9341 REGISTER MAPS #####
 * ============================================================================
 */

/* Core System Commands */
#define ILI9341_REG_SWRESET      0x01  // Software Reset
#define ILI9341_REG_SLPOUT       0x11  // Sleep Out Mode
#define ILI9341_REG_DISPON       0x29  // Display On
#define ILI9341_REG_CASET        0x2A  // Column Address Set
#define ILI9341_REG_PASET        0x2B  // Page/Row Address Set
#define ILI9341_REG_RAMWR        0x2C  // Memory Write
#define ILI9341_REG_MADCTL       0x36  // Memory Access Control (Orientation)
#define ILI9341_REG_COLMOD       0x3A  // Interface Pixel Format (RGB565)

/* Extended Display Parameters */
#define ILI9341_REG_FRMCTR1      0xB1  // Frame Rate Control (Normal Mode)
#define ILI9341_REG_DFC          0xB6  // Display Function Control
#define ILI9341_REG_PWCTR1       0xC0  // Power Control 1
#define ILI9341_REG_PWCTR2       0xC1  // Power Control 2
#define ILI9341_REG_VMCTR1       0xC5  // VCOM Control 1
#define ILI9341_REG_VMCTR2       0xC7  // VCOM Control 2

/* Manufacturing / Proprietary Control Blocks */
#define ILI9341_REG_PWCTRA       0xCB  // Power Control A
#define ILI9341_REG_PWCTRB       0xCF  // Power Control B
#define ILI9341_REG_TIMCTRA      0xE8  // Timing Control A
#define ILI9341_REG_TIMCTRB      0xEA  // Timing Control B
#define ILI9341_REG_POWERON      0xED  // Power-On Sequence Control
#define ILI9341_REG_PUMPRATIO    0xF7  // Pump Ratio Control

/*
 * ============================================================================
 *                     	  ##### TYPE DEFINITIONS #####
 * ============================================================================
 */

typedef struct 
{
    SPI_HandleTypeDef *hspi;     // Pointer to HAL SPI handle
    GPIO_TypeDef      *cs_port;  // Chip Select GPIO Port
    uint16_t           cs_pin;   // Chip Select GPIO Pin
    GPIO_TypeDef      *dc_port;  // Data/Command GPIO Port
    uint16_t           dc_pin;   // Data/Command GPIO Pin
    GPIO_TypeDef      *rst_port; // Reset GPIO Port (Optional, can tie to HW Reset)
    uint16_t           rst_pin;  // Reset GPIO Pin
} ILI9341_Handle_t;

typedef enum 
{
    ILI9341_DATA    = 0,
    ILI9341_COMMAND = 1
} ILI9341_WriteType_t;

/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

/**
 * @brief Initializes the ILI9341 display with default landscape configuration.
 *
 * This function handles the hardware power-on reset, executes the mandatory factory 
 * startup command sequences (power control, frame control, gamma correction), configures
 * the display mode to 320x240 landscape orientation, and wakes up the display panel.
 *
 * @param[in,out] dev Pointer to the ILI9341 device handle containing hardware configuration.
 */
void ILI9341_Init(ILI9341_Handle_t *dev);

/**
 * @brief Fills a bounded rectangular segment block with a solid color.
 *
 * This function sets up a bounding framework and streams pixel color data into 
 * the defined zone. It automatically manages boundaries to clip requests outside 
 * the screen dimensions.
 *
 * @param[in] dev          Pointer to the ILI9341 device handle.
 * @param[in] x            Top-left column location point.
 * @param[in] y            Top-left row location point.
 * @param[in] w            Width size allocation block.
 * @param[in] h            Height size allocation block.
 * @param[in] color_rgb565 Raw 16-bit colour formatting code in RGB565 matrix pattern.
 */
void ILI9341_FillRectangle(ILI9341_Handle_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color_rgb565);

/**
 * @brief Draws a single pixel at a designated (X, Y) coordinate.
 *
 * @param[in] dev          Pointer to the ILI9341 device handle.
 * @param[in] x            Column location of the pixel.
 * @param[in] y            Row location of the pixel.
 * @param[in] color_rgb565 Raw 16-bit colour formatting code in RGB565 matrix pattern.
 */
void ILI9341_DrawPixel(ILI9341_Handle_t *dev, uint16_t x, uint16_t y, uint16_t color_rgb565);

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_H */
