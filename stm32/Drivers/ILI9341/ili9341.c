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

static ILI9341_Status_t ILI9341_Write(ILI9341_Handle_t *dev, uint8_t byte, ILI9341_WriteType_t type);
static void ILI9341_SetAddressWindow(ILI9341_Handle_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/*
 * ============================================================================
 *                     	   ##### PUBLIC API #####
 * ============================================================================
 */

// -----------------------------------------------------------------------------
ILI9341_Status_t ILI9341_Init(ILI9341_Handle_t *dev)
{
    // Perform Hardware Reset if reset pins are designated
    if (dev->rst_port != NULL) 
    {
        HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, GPIO_PIN_RESET);
        HAL_Delay(10);
        HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, GPIO_PIN_SET);
        HAL_Delay(120);
    }

    // Soft Reset Command
    if (ILI9341_Write(dev, ILI9341_REG_SWRESET, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    HAL_Delay(120);

    // Power Control A (0xCB)
    if (ILI9341_Write(dev, ILI9341_REG_PWCTRA, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x39, ILI9341_DATA); // Fixed corporate code byte 1
    ILI9341_Write(dev, 0x2C, ILI9341_DATA); // Fixed corporate code byte 2
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // ESD protection control
    ILI9341_Write(dev, 0x34, ILI9341_DATA); // Vcore power control (VREG1OUT = 1.55V)
    ILI9341_Write(dev, 0x02, ILI9341_DATA); // DDVDD supply voltage control

    // Power Control B (0xCF)
    if (ILI9341_Write(dev, ILI9341_REG_PWCTRB, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // Standby/Normal mode power optimization
    ILI9341_Write(dev, 0xC1, ILI9341_DATA); // Power control parameter 2
    ILI9341_Write(dev, 0x30, ILI9341_DATA); // ESD control parameter

    // Driver Timing Control A (0xE8)
    if (ILI9341_Write(dev, ILI9341_REG_TIMCTRA, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x85, ILI9341_DATA); // Non-overlap period / Gate driver timing
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // Source driver timing control
    ILI9341_Write(dev, 0x78, ILI9341_DATA); // EQ (equalization) timing period

    // Driver Timing Control B (0xEA)
    if (ILI9341_Write(dev, ILI9341_REG_TIMCTRB, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // Gate driver timing adjustment
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // Source timing margin parameter

    // Power On Sequence Control (0xED)
    if (ILI9341_Write(dev, ILI9341_REG_POWERON, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x64, ILI9341_DATA); // Soft start enable parameter
    ILI9341_Write(dev, 0x03, ILI9341_DATA); // Power on sequence parameter 1
    ILI9341_Write(dev, 0x12, ILI9341_DATA); // Power on sequence parameter 2
    ILI9341_Write(dev, 0x81, ILI9341_DATA); // Pump internal clock selection

    // Pump Ratio Control (0xF7)
    if (ILI9341_Write(dev, ILI9341_REG_PUMPRATIO, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x20, ILI9341_DATA); // Internal charge pump multiplier scaling

    // Power Control 1 (0xC0)
    if (ILI9341_Write(dev, ILI9341_REG_PWCTR1, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x23, ILI9341_DATA); // Set GVDD voltage level reference to 4.60V

    // Power Control 2 (0xC1)
    if (ILI9341_Write(dev, ILI9341_REG_PWCTR2, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x10, ILI9341_DATA); // Set internal operational step-up factor (VGH/VGL)

    // VCOM Control 1 (0xC5)
    if (ILI9341_Write(dev, ILI9341_REG_VMCTR1, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x3E, ILI9341_DATA); // Set reference voltage VCOMH to 4.250V
    ILI9341_Write(dev, 0x28, ILI9341_DATA); // Set reference voltage VCOML to -1.500V

    // VCOM Control 2 (0xC7)
    if (ILI9341_Write(dev, ILI9341_REG_VMCTR2, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x86, ILI9341_DATA); // VCOM offset voltage control configuration

    // Memory Access Control (0x36) -> Set to Landscape Orientation (320x240)
    if (ILI9341_Write(dev, ILI9341_REG_MADCTL, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x28, ILI9341_DATA); // Bitmask: Column/Row Exchange=1, BGR Color Order=1

    // COLMOD: Pixel Format Set (0x3A) -> Configure to standard 16-bit RGB565
    if (ILI9341_Write(dev, ILI9341_REG_COLMOD, ILI9341_COMMAND)!= ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x55, ILI9341_DATA); // Sets both MCU and Frame Memory interfaces to 16 bits/pixel

    // Frame Rate Control (0xB1) -> Normal Mode, 70Hz Frame Rate
    if (ILI9341_Write(dev, ILI9341_REG_FRMCTR1, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x00, ILI9341_DATA); // Clocks per line division factor = 0
    ILI9341_Write(dev, 0x18, ILI9341_DATA); // Frame division rate = 24 (yields ~70Hz panel refresh)

    // Display Function Control (0xB6)
    if (ILI9341_Write(dev, ILI9341_REG_DFC, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    ILI9341_Write(dev, 0x08, ILI9341_DATA); // Set hardware non-display scanning settings
    ILI9341_Write(dev, 0x82, ILI9341_DATA); // Gate driver scan direction configuration
    ILI9341_Write(dev, 0x27, ILI9341_DATA); // Set total panel scan lines (320 lines target)

    // Exit Sleep Mode (0x11)
    if (ILI9341_Write(dev, ILI9341_REG_SLPOUT, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }
    HAL_Delay(120);

    // Turn On Display (0x29)
    if (ILI9341_Write(dev, ILI9341_REG_DISPON, ILI9341_COMMAND) != ILI9341_OK)
    {
        return ILI9341_ERR_SPI;
    }

    return ILI9341_OK;
}

// -----------------------------------------------------------------------------
ILI9341_Status_t ILI9341_FillRectangle(ILI9341_Handle_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color_rgb565)
{
    // Defensive clipping checks
    if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) 
    {
        return ILI9341_ERR_BOUNDS;
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

    // High-Performance Burst Strategy (Drastically speeds up screen clears)
    uint8_t pixel_chunk[64];
    uint8_t hi_byte = (color_rgb565 >> 8) & 0xFF;
    uint8_t lo_byte = color_rgb565 & 0xFF;

    for (uint8_t i = 0; i < 64; i += 2)
    {
        pixel_chunk[i]     = hi_byte;
        pixel_chunk[i + 1] = lo_byte;
    }

    uint32_t total_pixels = w * h;
    uint32_t total_bytes  = total_pixels * 2;

    while (total_bytes > 0)
    {
        uint16_t bytes_to_send = (total_bytes > 64) ? 64 : total_bytes;

        if (HAL_SPI_Transmit(dev->hspi, pixel_chunk, bytes_to_send, HAL_MAX_DELAY) != HAL_OK)
        {
          HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
          return ILI9341_ERR_SPI;
        }

        total_bytes -= bytes_to_send;
    }

    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
    return ILI9341_OK;
}

// -----------------------------------------------------------------------------
ILI9341_Status_t ILI9341_DrawPixel(ILI9341_Handle_t *dev, uint16_t x, uint16_t y, uint16_t color_rgb565)
{
    if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) 
    {
        return ILI9341_ERR_BOUNDS;
    }

    ILI9341_SetAddressWindow(dev, x, y, x, y);

    uint8_t buffer[2] = { (color_rgb565 >> 8) & 0xFF, color_rgb565 & 0xFF };

    HAL_GPIO_WritePin(dev->dc_port, dev->dc_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);

    if (HAL_SPI_Transmit(dev->hspi, buffer, 2, HAL_MAX_DELAY) != HAL_OK)
    {
        return ILI9341_ERR_SPI;
    }

    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
    return ILI9341_OK;
}

/*
 * ============================================================================
 *                ##### PRIVATE FUNCTION IMPLEMENTATIONS #####
 * ============================================================================
 */

/*******************************************************************************
 * @brief Low-level driver utility to transmit a single byte over the SPI bus.
 *
 * @param[in,out] dev Pointer to the ILI9341 device handle containing hardware details.
 * @param[in] byte The 8-bit command or data value to transmit.
 * @param[in] type Designation flag determining if data byte is COMMAND or DATA.
 *
 * @retval ILI9341_OK Byte packed and flushed over SPI lines cleanly.
 * @retval ILI9341_ERR_SPI SPI hardware configuration block timed out.
 ******************************************************************************/
static ILI9341_Status_t ILI9341_Write(ILI9341_Handle_t *dev, uint8_t byte, ILI9341_WriteType_t type)
{
    uint8_t local_byte = byte;

    // Set D/C Pin: low for data, high for command
    if (type == ILI9341_COMMAND)
    {
        HAL_GPIO_WritePin(dev->dc_port, dev->dc_pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(dev->dc_port, dev->dc_pin, GPIO_PIN_SET);
    }

    // Select the display controller
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);

    // Transmit the byte over SPI
    if (HAL_SPI_Transmit(dev->hspi, &local_byte, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return ILI9341_ERR_SPI;
    }

    // De-select the display controller
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
    return ILI9341_OK;
}

/*******************************************************************************
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
 *******************************************************************************/
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

