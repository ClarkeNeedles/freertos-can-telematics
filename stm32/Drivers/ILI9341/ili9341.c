/**
  ******************************************************************************
  * @file           : ili9341.c
  * @brief          : ILI9341 display driver
  * @author         : Clarke Needles
  ******************************************************************************
  */

#include "ili9341.h"

/*
 * ============================================================================
 *                  ##### PRIVATE FUNCTION PROTOTYPES  #####
 * ============================================================================
 */

static void ILI9341_Write(ILI9341_Handle_t *dev, uint8_t byte, ILI9341_WriteType_t type);
static void ILI9341_SetAddressWindow(ILI9341_Handle_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

// -----------------------------------------------------------------------------
void ILI9341_Init(ILI9341_Handle_t *dev)
{
    // 1. Perform Hardware Reset if reset pins are designated
    if (dev->rst_port != NULL) 
    {
        HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, GPIO_PIN_RESET);
        HAL_Delay(10);
        HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, GPIO_PIN_SET);
        HAL_Delay(120);
    }

    // 2. Soft Reset Command
    ILI9341_Write(dev, ILI9341_REG_SWRESET, ILI9341_COMMAND);
    HAL_Delay(120);

    // 3. Power Control A (0xCB)
    ILI9341_Write(dev, ILI9341_REG_PWCTRA, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x39, ILI9341_DATA); // Fixed corporate code byte 1
    ILI9341_Write(dev, 0x2C, ILI9341_DATA); // Fixed corporate code byte 2
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // ESD protection control
    ILI9341_Write(dev, 0x34, ILI9341_DATA); // Vcore power control (VREG1OUT = 1.55V)
    ILI9341_Write(dev, 0x02, ILI9341_DATA); // DDVDD supply voltage control

    // 4. Power Control B (0xCF)
    ILI9341_Write(dev, ILI9341_REG_PWCTRB, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // Standby/Normal mode power optimization
    ILI9341_Write(dev, 0xC1, ILI9341_DATA); // Power control parameter 2
    ILI9341_Write(dev, 0x30, ILI9341_DATA); // ESD control parameter

    // 5. Driver Timing Control A (0xE8)
    ILI9341_Write(dev, ILI9341_REG_TIMCTRA, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x85, ILI9341_DATA); // Non-overlap period / Gate driver timing
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // Source driver timing control
    ILI9341_Write(dev, 0x78, ILI9341_DATA); // EQ (equalization) timing period

    // 6. Driver Timing Control B (0xEA)
    ILI9341_Write(dev, ILI9341_REG_TIMCTRB, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // Gate driver timing adjustment
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // Source timing margin parameter

    // 7. Power On Sequence Control (0xED)
    ILI9341_Write(dev, ILI9341_REG_POWERON, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x64, ILI9341_DATA); // Soft start enable parameter
    ILI9341_Write(dev, 0x03, ILI9341_DATA); // Power on sequence parameter 1
    ILI9341_Write(dev, 0x12, ILI9341_DATA); // Power on sequence parameter 2
    ILI9341_Write(dev, 0x81, ILI9341_DATA); // Pump internal clock selection

    // 8. Pump Ratio Control (0xF7)
    ILI9341_Write(dev, ILI9341_REG_PUMPRATIO, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x20, ILI9341_DATA); // Internal charge pump multiplier scaling

    // 9. Power Control 1 (0xC0)
    ILI9341_Write(dev, ILI9341_REG_PWCTR1, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x23, ILI9341_DATA); // Set GVDD voltage level reference to 4.60V

    // 10. Power Control 2 (0xC1)
    ILI9341_Write(dev, ILI9341_REG_PWCTR2, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x10, ILI9341_DATA); // Set internal operational step-up factor (VGH/VGL)

    // 11. VCOM Control 1 (0xC5)
    ILI9341_Write(dev, ILI9341_REG_VMCTR1, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x3E, ILI9341_DATA); // Set reference voltage VCOMH to 4.250V
    ILI9341_Write(dev, 0x28, ILI9341_DATA); // Set reference voltage VCOML to -1.500V

    // 12. VCOM Control 2 (0xC7)
    ILI9341_Write(dev, ILI9341_REG_VMCTR2, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x86, ILI9341_DATA); // VCOM offset voltage control configuration

    // 13. Memory Access Control (0x36) -> Set to Landscape Orientation (320x240)
    ILI9341_Write(dev, ILI9341_REG_MADCTL, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x28, ILI9341_DATA); // Bitmask: Column/Row Exchange=1, BGR Color Order=1

    // 14. COLMOD: Pixel Format Set (0x3A) -> Configure to standard 16-bit RGB565
    ILI9341_Write(dev, ILI9341_REG_COLMOD, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x55, ILI9341_DATA); // Sets both MCU and Frame Memory interfaces to 16 bits/pixel

    // 15. Frame Rate Control (0xB1) -> Normal Mode, 70Hz Frame Rate
    ILI9341_Write(dev, ILI9341_REG_FRMCTR1, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // Clocks per line division factor = 0
    ILI9341_Write(dev, 0x18, ILI9341_DATA); // Frame division rate = 24 (yields ~70Hz panel refresh)

    // 16. Display Function Control (0xB6)
    ILI9341_Write(dev, ILI9341_REG_DFC, ILI9341_COMMAND);
    ILI9341_Write(dev, 0x08, ILI9341_DATA); // Set hardware non-display scanning settings
    ILI9341_Write(dev, 0x82, ILI9341_DATA); // Gate driver scan direction configuration
    ILI9341_Write(dev, 0x27, ILI9341_DATA); // Set total panel scan lines (320 lines target)

    // 17. Exit Sleep Mode (0x11)
    ILI9341_Write(dev, ILI9341_REG_SLPOUT, ILI9341_COMMAND);
    HAL_Delay(120);

    // 18. Turn On Display (0x29)
    ILI9341_Write(dev, ILI9341_REG_DISPON, ILI9341_COMMAND);
}

// -----------------------------------------------------------------------------
void ILI9341_FillRectangle(ILI9341_Handle_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color_rgb565)
{
    // Defensive clipping checks
    if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) 
    {
        return;
    }

    if ((x + w - 1) >= ILI9341_WIDTH) 
    {
        w = ILI9341_WIDTH  - x;
    }

    if ((y + h - 1) >= ILI9341_HEIGHT)
    {
        h = ILI9341_HEIGHT - y;
    }

    ILI9341_SetAddressWindow(dev, x, y, x + w - 1, y + h - 1);

    HAL_GPIO_WritePin(dev->dc_port, dev->dc_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);

    // Block-write looping strategy (30 FPS or less)
    uint8_t buffer[2] = { (color_rgb565 >> 8) & 0xFF, color_rgb565 & 0xFF };
    uint32_t total_pixels = w * h;
    for (uint32_t i = 0; i < total_pixels; i++) 
    {
        HAL_SPI_Transmit(dev->hspi, buffer, 2, HAL_MAX_DELAY);
    }

    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

// -----------------------------------------------------------------------------
void ILI9341_DrawPixel(ILI9341_Handle_t *dev, uint16_t x, uint16_t y, uint16_t color_rgb565)
{
    if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) 
    {
        return;
    }

    ILI9341_SetAddressWindow(dev, x, y, x, y);

    uint8_t buffer[2] = { (color_rgb565 >> 8) & 0xFF, color_rgb565 & 0xFF };

    HAL_GPIO_WritePin(dev->dc_port, dev->dc_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(dev->hspi, buffer, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

/*
 * ============================================================================
 *                ##### PRIVATE FUNCTION IMPLEMENTATIONS #####
 * ============================================================================
 */

/**
 * @brief  Low-level driver utility to transmit a single byte over the SPI bus.
 * @note   Automatically manages the out-of-band Data/Command (D/C) line and Chip Select (CS).
 *
 * @param[in,out] dev  Pointer to the ILI9341 device handle containing hardware details.
 * @param[in]     byte The 8-bit command or data value to transmit.
 * @param[in]     type Designation flag determining if the payload is a Command or Data.
 */
static void ILI9341_Write(ILI9341_Handle_t *dev, uint8_t byte, ILI9341_WriteType_t type)
{
    // Set D/C Pin: Low for Data, High for Command
    HAL_GPIO_WritePin(dev->dc_port, dev->dc_pin, 
                     (type == ILI9341_DATA) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    // Select the display controller
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
    
    // Transmit the byte over SPI
    HAL_SPI_Transmit(dev->hspi, &byte, 1, HAL_MAX_DELAY);
    
    // De-select the display controller
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

/**
 * @brief Configures the active drawing bounding window on the display glass grid.
 *
 * This function sends boundary limits to the column and row registers to dictate where
 * subsequent streaming pixel colour sequences are painted.
 *
 * @param[in,out] dev Pointer to the hardware handle.
 * @param[in]     x0  Starting column coordinate (0 to @c ILI9341_WIDTH - 1).
 * @param[in]     y0  Starting row coordinate (0 to @c ILI9341_HEIGHT - 1).
 * @param[in]     x1  Ending column coordinate boundary.
 * @param[in]     y1  Ending row coordinate boundary.
 */
static void ILI9341_SetAddressWindow(ILI9341_Handle_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // Column Address Set
    ILI9341_Write(dev, ILI9341_REG_CASET, ILI9341_COMMAND);
    ILI9341_Write(dev, (x0 >> 8) & 0xFF, ILI9341_DATA);
    ILI9341_Write(dev, x0 & 0xFF,        ILI9341_DATA);
    ILI9341_Write(dev, (x1 >> 8) & 0xFF, ILI9341_DATA);
    ILI9341_Write(dev, x1 & 0xFF,        ILI9341_DATA);

    // Row Address Set
    ILI9341_Write(dev, ILI9341_REG_PASET, ILI9341_COMMAND);
    ILI9341_Write(dev, (y0 >> 8) & 0xFF, ILI9341_DATA);
    ILI9341_Write(dev, y0 & 0xFF,        ILI9341_DATA);
    ILI9341_Write(dev, (y1 >> 8) & 0xFF, ILI9341_DATA);
    ILI9341_Write(dev, y1 & 0xFF,        ILI9341_DATA);

    // RAM Write Command (Prepare internal buffer to swallow stream data)
    ILI9341_Write(dev, ILI9341_REG_RAMWR, ILI9341_COMMAND);
}

