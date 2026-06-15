# CAN-Enabled Weather Telematics Dashboard

## Project Overview

The CAN-Enabled Weather Telematics Dashboard is a FreeRTOS-based embedded system designed to provide real-time environmental and location data within an automotive context. It serves as a bridge between industrial CAN bus networks, local sensor inputs (temperature, humidity, pressure, GPS), and cloud-based weather information. The system leverages an STM32 microcontroller to manage concurrent tasks, process sensor data, display vital information on an SPI-driven LCD, and interact with a PC-based application for external data integration via the CAN bus.

## Features

### Core System & RTOS Management

*   **High-Performance Core Configuration:** STM32F446RE microcontroller configured for optimal performance with a 180 MHz CPU clock and 45 MHz APB1 bus.
*   **FreeRTOS Integration:** Full FreeRTOS kernel deployment including heap and tick timer configurations, and NVIC interrupt priority adjustments for robust multi-tasking.
*   **Multi-Tasking Architecture:** Implemented FreeRTOS tasks for dedicated GPS data parsing, local telemetry sampling, CAN bus communication, and display UI management.
*   **Inter-Task Communication & Synchronization:** Utilizes FreeRTOS queues for secure data transfer between tasks and mutexes for protecting shared resources like the SPI display bus.
*   **System Stability & Watchdog:** Integrated Independent Hardware Watchdog Timer (IWDG) for automatic system resets, complemented by software task check-ins for comprehensive fault recovery.
*   **Memory Optimization:** Task stack sizes are tuned using `uxTaskGetStackHighWaterMark()` to ensure efficient RAM utilization.

### CAN Bus Communication

*   **CAN Controller Initialization:** STM32 bxCAN1 peripheral configured for standard bus frequencies (e.g., 500 kbps) with precise bit-timing and an 87.5% sample point.
*   **Physical Layer Interface:** Seamless integration with the Waveshare SN65HVD230 CAN Transceiver, ensuring robust physical layer connectivity with a 120-ohm termination resistor.
*   **CAN Packet Transmission:** Capable of sending standard 8-byte diagnostic CAN frames.
*   **Efficient CAN Reception:** Optimized Interrupt Service Routine (ISR) for CAN RX FIFO 0/1, enabling instant data ingestion, immediate buffer release, and non-blocking task notification (`xTaskNotifyFromISR()`).
*   **Hardware Filtering:** Configured STM32 hardware filter banks to automatically discard irrelevant CAN frames at the hardware level, maintaining task responsiveness under high network load.
*   **PC Diagnostic Connectivity:** Interfaced with a DSD TECH SH-C30A USB-to-CAN adapter for external bus monitoring, debugging, and packet injection from a PC.

### Sensor Integration

*   **Environmental Sensing (BME280):**
    *   I2C1 peripheral configured for Standard (100 kHz) or Fast Mode (400 kHz).
    *   Integrated Bosch BME280 API driver to read ambient temperature, relative humidity, and barometric pressure.
    *   Live temperature data verified through serial output.
*   **GPS Location Tracking (GY-NEO6MV2):**
    *   USART1 peripheral configured at 9600 baud (8N1).
    *   Non-blocking data acquisition via Direct Memory Access (DMA) in circular buffer mode for continuous NMEA string streaming.
    *   Integrated NMEA parser (e.g., `minmea` C library) to extract precise latitude, longitude, and movement metrics from `$GPRMC` and `$GPGGA` sentences.

### Display & User Interface

*   **SPI Display Control:** Elecrow 2.8-inch TFT LCD (ILI9341 driver) connected via SPI1, configured for optimal clock frequencies (10 MHz to 22.5 MHz).
*   **Graphics Driver Integration:** Utilizes a lightweight ILI9341 display core port for rendering graphical elements.
*   **Dynamic UI Rendering:** Capable of clearing the screen, filling the canvas with solid colors, and drawing formatted string segments to display live sensor values and GPS positions.
*   **Thread-Safe Display Access:** SPI bus access is guarded by a FreeRTOS Mutex, preventing data corruption and bus collisions during concurrent UI updates.

### External Integration

*   **CAN-to-Cloud Bridge (PC Node):** A companion script (Python/Rust) on a connected PC reads CAN-transmitted GPS coordinates, queries an online OpenWeather API for current outdoor conditions, and transmits external temperature data back to the STM32 over the CAN bus.

## Tech Stack

*   **Microcontroller:** STMicroelectronics STM32 Nucleo-F446RE (ARM Cortex-M4)
*   **Real-time Operating System (RTOS):** FreeRTOS
*   **Embedded Frameworks/Libraries:**
    *   ST HAL Library
    *   Bosch BME280 API Driver
    *   ILI9341 Display Driver (e.g., afiskon/stm32-ili9341)
    *   minmea C library (for NMEA parsing)
*   **Communication Protocols:** CAN Bus, I2C, UART, SPI
*   **External Hardware:**
    *   Waveshare SN65HVD230 CAN Transceiver
    *   Elecrow 2.8-Inch TFT LCD Touch Screen Shield (ILI9341 Driver)
    *   GY-NEO6MV2 NEO-6M GPS Module
    *   Waveshare BME280 Environmental Sensor
    *   DSD TECH SH-C30A USB to CAN Bus Adapter
*   **Programming Languages:** C/C++ (Embedded Firmware), Python / Rust (PC Bridge Application)

## Current Status

**Status:** in-progress
**Version:** 0.0.0