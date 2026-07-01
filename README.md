![Project Status](https://img.shields.io/badge/status-in--progress-yellowgreen)
![Project Version](https://img.shields.io/badge/version-0.2.1-blue)
![https://shields.io/ClarkeNeedles/Autonomous-PiCar](https://img.shields.io/github/repo-size/ClarkeNeedles/freertos-can-telematics)

# CAN-Enabled Weather Telematics Dashboard

## High-Level Overview
This project focuses on developing a multi-threaded, FreeRTOS-based telematics dashboard for real-time weather and location monitoring. It integrates an industrial automotive CAN bus protocol with various real-time sensor inputs and cloud API data. The system is designed to provide concurrent processing of sensor data, efficient communication, and a responsive user interface.

## Table of Contents
*   [System Architecture](#system-architecture)
    *   [Hardware Architecture](#hardware-architecture)
    *   [Software Architecture](#software-architecture)
*   [Tech Stack and Dependencies](#tech-stack-and-dependencies)
    *   [Hardware Components](#hardware-components)
    *   [Software Libraries & Frameworks](#software-libraries--frameworks)
*   [Features](#features)
*   [Prerequisites / System Requirements](#prerequisites--system-requirements)
    *   [Hardware](#hardware)
    *   [Software](#software)
    *   [Electrical](#electrical)
*   [Step-by-Step Installation / Setup](#step-by-step-installation--setup)
    *   [Phase 1: Individual Bare-Metal Hardware Sandboxing](#phase-1-individual-bare-metal-hardware-sandboxing)
    *   [Phase 2: Combined Bare-Metal Integration](#phase-2-combined-bare-metal-integration)
*   [Configuration](#configuration)
    *   [BME280 I2C Address](#bme280-i2c-address)
    *   [CAN Bus Bit-Timing](#can-bus-bit-timing)
    *   [FreeRTOS Configuration](#freertos-configuration)
    *   [NVIC Interrupt Priority](#nvic-interrupt-priority)
*   [Quick Start / Usage Examples](#quick-start--usage-examples)
*   [Hardware Pinout / Interconnect Map](#hardware-pinout--interconnect-map)
    *   [BME280 Sensor Wiring (I2C Mode)](#bme280-sensor-wiring-i2c-mode)
    *   [General Peripheral Pin Assignments](#general-peripheral-pin-assignments)

## System Architecture

### Hardware Architecture
The system is built around an **STM32 Nucleo-F446RE** microcontroller (ARM Cortex-M4) serving as the central processing unit. It interfaces with several key peripherals:
*   **I2C Temperature Sensor (Waveshare BME280)**: For local environmental readings.
*   **UART GPS Module (GY-NEO6MV2)**: Streams NMEA sentences for location data.
*   **SPI Display (Elecrow 2.8-inch TFT LCD with ILI9341 driver)**: For visual output of telemetry and UI.
*   **CAN Controller (STM32 bxCAN1 via Waveshare SN65HVD230 Transceiver)**: For industrial automotive protocol communication.
*   **Desktop Bridge (DSD TECH SH-C30A USB-to-CAN Adapter)**: Connects the STM32's CAN lines to a laptop for external API integration and diagnostics.

### Software Architecture
The core software leverages **FreeRTOS** for concurrent task management, ensuring real-time performance and preventing blocking operations from impacting system responsiveness. The **ST HAL library** facilitates easy peripheral setup.
*   **Multi-threaded System**: FreeRTOS tasks manage GPS parsing, local telemetry sampling, CAN communication (sending and receiving), and display UI rendering.
*   **Custom NMEA GPS Parser**: A manual parser processes `$GPRMC` and `$GPGGA` sentences via UART IDLE interrupt or circular DMA.
*   **CAN Data Framing**: GPS coordinates and other telemetry are manually packed into 8-byte CAN data frames.
*   **Laptop/API Layer Node**: A companion Python (or Rust) script on the laptop reads CAN data, queries an OpenWeather API for outdoor conditions, and transmits results back to the STM32 via CAN.

## Tech Stack and Dependencies

### Hardware Components
| Component | Communication Interface | Function |
| :--------------------------------- | :----------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------- |
| STMicroelectronics NUCLEO-F446RE   | Master MCU Host                | Runs FreeRTOS, handles data, coordinates peripherals.                                                                                                  |
| Waveshare SN65HVD230 CAN Board     | CAN Protocol                   | Interface layer between STM32 logic pins and differential CAN network.                                                                                  |
| Elecrow 2.8-Inch TFT LCD Touch Shield (ILI9341) | 4-Wire SPI                     | Renders GUI, real-time sensor values, system statuses.                                                                                                 |
| GY-NEO6MV2 NEO-6M GPS Module       | UART                           | Parses NMEA sentences for latitude, longitude, elevation, timestamp, and speed.                                                                        |
| Waveshare BME280 Environmental Sensor | I2C (Default Address: `0x76`)  | Reads ambient temperature, relative humidity, and barometric pressure.                                                                                 |
| DSD TECH SH-C30A USB to CAN Bus Adapter | USB Type-A to CAN Bus          | Bridges desktop software for monitoring, debugging, and injecting/reading CAN packets.                                                                 |

### Software Libraries & Frameworks
*   **Real-Time Operating System**: FreeRTOS
*   **Microcontroller Abstraction Layer**: STMicroelectronics STM32 HAL library
*   **Development Environment**: STM32CubeIDE
*   **BME280 Driver (I2C)**: [Afebia/BME280-STM32-V2](https://github.com/Afebia/BME280-STM32-V2)
*   **NEO-6M GPS Driver (UART)**: [leech001/gps](https://github.com/leech001/gps)
*   **ILI9341 TFT Display Driver (SPI)**: [martnak/STM32-ILI9341](https://github.com/martnak/STM32-ILI9341)
*   **bxCAN Network Driver**: [WassimHedfi/CAN_Protocol_STM32f446re_V_STM32f407G](https://github.com/WassimHedfi/CAN_Protocol_STM32f446re_V_STM32f407G) (base framework)
*   **NMEA Parsing Library**: `minmea` C library (potential integration for thread-safe parsing)
*   **Desktop Companion Script**: Python (using `python-can`) or Rust for CAN-to-API bridge.

## Features
*   **Real-time Sensor Telemetry**:
    *   Precise GPS location tracking (latitude, longitude, elevation, speed) via NMEA sentence parsing.
    *   Local environmental sensing (temperature, humidity, barometric pressure) using the BME280 sensor.
*   **CAN Bus Communication**:
    *   Transmission of local telemetry (GPS, temperature) as custom 8-byte CAN packets.
    *   Reception and processing of incoming CAN packets from external sources (e.g., laptop).
    *   Hardware-level CAN filter banks to discard irrelevant network traffic efficiently.
*   **Multi-threaded System (FreeRTOS)**:
    *   `Task_GPS_Parser`: High-priority task for monitoring UART buffer and processing GPS data.
    *   `Task_Local_Telemetry`: Medium-priority task for periodic I2C temperature sampling.
    *   `Task_CAN_Comms`: High-priority task for CAN packet framing, transmission, and reception.
    *   `Task_Display_UI`: Low-priority task for driving the SPI display and rendering UI elements.
    *   Inter-task concurrency managed with FreeRTOS Queues and Mutexes.
    *   Optimized Interrupt Service Routines (ISRs) for CAN reception with task notification.
*   **User Interface**:
    *   SPI-driven TFT LCD display for rendering real-time indoor temperature, GPS coordinates, and external outdoor weather updates.
*   **Robustness & Optimization**:
    *   Hardware Watchdog Timer (IWDG) integration for automatic system reset on application freeze.
    *   Runtime stack diagnostics and memory allocation tuning for efficient RAM usage.
    *   Defensive programming within drivers (e.g., division-by-zero protection, data clamping).
*   **BME280 Driver Specific Features**:
    *   Generic and handle-based driver design for supporting multiple sensors.
    *   Optimized I2C transaction with multi-byte burst reads for data consistency and reduced bus overhead.
    *   Production-grade Doxygen/Javadoc documentation for API clarity and maintainability.
    *   Integration of calibration data parsing and compensation logic for accurate readings.
*   **Desktop Companion Application**:
    *   Python-based script to read CAN data (lat/long), query OpenWeather API, and transmit outdoor weather data back to the STM32 via CAN.

## Prerequisites / System Requirements

### Hardware
*   STMicroelectronics NUCLEO-F446RE Development Board
*   Waveshare SN65HVD230 CAN Board
*   Elecrow 2.8-Inch TFT LCD Touch Screen Shield Display (ILI9341 Driver)
*   GY-NEO6MV2 NEO-6M GPS Module with Ceramic Antenna
*   Waveshare BME280 Environmental Sensor
*   DSD TECH SH-C30A USB to CAN Bus Adapter
*   Breadboard, jumper wires, 120-ohm termination resistor for CAN bus.

### Software
*   **STM32CubeIDE**: For project initialization, configuration, compilation, and debugging.
    *   Ensure C/C++ build settings include necessary driver directories in compiler include paths.
*   **Desktop Operating System**: Linux, Windows, or macOS for running STM32CubeIDE and the companion Python script.
*   **Python**: With `python-can` library installed for the laptop-side API integration.
*   **Packet Logging Utilities**: Such as Cangaroo or native Linux SocketCAN utilities for CAN bus monitoring.

### Electrical
*   **BME280 Sensor**: Requires 3.3V power supply (do not use 5V).
*   **CAN Bus**: Ensure a 120-ohm termination resistor is present across the CAN_H and CAN_L lines.

## Step-by-Step Installation / Setup

This section outlines the iterative development roadmap to set up and integrate the system's components.

### Phase 1: Individual Bare-Metal Hardware Sandboxing
Goal: Verify individual hardware module functionality and communication with the STM32F446RE using blocking drivers.

1.  **Baseline Toolchain Setup & Clock Configuration**:
    *   Initialize a clean, non-RTOS C project in STM32CubeIDE for NUCLEO-F446RE.
    *   Configure CPU clock to 180 MHz (using HSE).
    *   Set APB1 clock prescaler to keep the peripheral bus at or below 45 MHz.

2.  **Environmental Sensing Sandbox (Waveshare BME280)**:
    *   **Pin Mapping**: Wire I2C1_SCL (PB6) and I2C1_SDA (PB7). Ensure internal pull-ups are enabled if the breakout board lacks them.
    *   **Configuration**: Initialize STM32 I2C1 peripheral in Standard (100 kHz) or Fast Mode (400 kHz) with 7-bit addressing.
    *   **Verification**: Execute `HAL_I2C_Mem_Read()` to read BME280 ID register (0xD0) and confirm `0x60`.
    *   **Driver Setup**: Integrate the BME280 driver (e.g., `Afebia/BME280-STM32-V2`).
    *   **Functional Test**: Loop data fetches, parse, and print live temperature values to a serial terminal.

3.  **Satellite Location Sandbox (GY-NEO6MV2 GPS)**:
    *   **Pin Mapping**: Wire USART1_RX and USART1_TX to the GPS module.
    *   **Configuration**: Configure STM32 USART1 to 9600 baud, 8N1.
    *   **Functional Test**: Use `HAL_UART_Receive()` to stream incoming bytes to PC terminal.
    *   **Verification**: Confirm NMEA sentences (`$GPRMC`, `$GPGGA`) are cleanly flowing.

4.  **Display Subsystem Sandbox (Elecrow 2.8" LCD)**:
    *   **Pin Mapping**: Connect SPI1_SCK, SPI1_MISO, SPI1_MOSI. Allocate three GPIOs for LCD_DC, LCD_CS, and LCD_RST.
    *   **Configuration**: Initialize STM32 SPI1 as Master, clock prescaler for 10-22.5 MHz (Mode 0: CPOL=0, CPHA=0).
    *   **Driver Setup**: Integrate ILI9341 display core port (e.g., `martnak/STM32-ILI9341`).
    *   **Verification**: Execute initialization sequence and paint the LCD panel a solid color to confirm functionality.

5.  **Vehicle Network Interface Sandbox (STM32 bxCAN1)**:
    *   **Pin Mapping**: Map CAN1_RX and CAN1_TX to the SN65HVD230 transceiver.
    *   **Physical Network Setup**: Bridge transceiver outputs (CAN_H/CAN_L) to the DSD TECH SH-C30A USB-to-CAN Adapter. Ensure a 120-ohm termination resistor is present. Connect the adapter to a PC running Cangaroo.
    *   **Bit-Timing Calculations**: Configure CAN registers for 250 kbps or 500 kbps (e.g., 500 kbps: 45 MHz clock, Prescaler 5, BS1 13tq, BS2 2tq, SJW 1tq for an 87.5% sample point).
    *   **Verification**: Send a hardcoded 8-byte diagnostic frame using `HAL_CAN_AddTxMessage()` and confirm reception in PC software without bus errors.

### Phase 2: Combined Bare-Metal Integration
Goal: Combine devices in a single standard bare-metal `main()` loop, resolving resource conflicts before RTOS introduction.

1.  **Non-Blocking Sensor Background Ingestion**:
    *   **GPS Background Stream**: Configure USART1 with a dedicated DMA channel in Circular Buffer Mode (e.g., 512-byte array). Invoke `HAL_UART_Receive_DMA()` once at boot.
    *   **NMEA Parsing Implementation**: Integrate NMEA engine (e.g., `minmea` or `leech001/gps`) to detect newline tokens, extract sentences, and parse latitude, longitude, and movement metrics from the circular DMA buffer.

2.  **Local Bus Orchestration & Single Loop Execution**:
    *   Structure the `while(1)` loop to sequentially read BME280 data via I2C and pull GPS positions from the DMA routine.
    *   Package computed live variables into formatted strings and draw them onto the ILI9341 display using the SPI display driver.
    *   Assemble temperature and GPS location fields into 8-byte CAN data arrays. Schedule an outgoing packet loop to continuously broadcast these custom payloads over the CAN network.

## Configuration

### BME280 I2C Address
*   The BME280 I2C address depends on the SDO pin state.
*   **Configuration used**: SDO grounded (`X = 0`).
*   **Final address**: `0xEC` (binary `11101100`).

### CAN Bus Bit-Timing
*   **Example (500 kbps via a 45 MHz Clock)**:
    *   Clock Prescaler factor: `5` (yielding a time quanta $t_q$ of 111.11 ns).
    *   Phase Segment 1 (BS1): `13 t_q`
    *   Phase Segment 2 (BS2): `2 t_q`
    *   Sync Jump Width (SJW): `1 t_q`
    *   Total quanta sum up to 16, anchoring the data bit evaluation point at the 87.5% sample point.

### FreeRTOS Configuration
*   **Heap Size & Tick Timer**: Configure `configTOTAL_HEAP_SIZE`, `configTICK_RATE_HZ`, and other system variables within `FreeRTOSConfig.h`.

### NVIC Interrupt Priority
*   **Interrupt Preemption Levels**: Reconfigure the processor's Nested Vectored Interrupt Controller (NVIC) settings. Ensure hardware interrupt preemption levels are below `configMAX_SYSCALL_INTERRUPT_PRIORITY` to prevent kernel timing loop compromises.

## Quick Start / Usage Examples
Once the system is set up and running, you can expect the following operational flow:

1.  **Power On**: The STM32 board initializes, starting all FreeRTOS tasks.
2.  **Sensor Data Acquisition**:
    *   The BME280 sensor continuously reads internal cabin temperature, humidity, and pressure.
    *   The GPS module streams NMEA data, which is parsed to extract latitude, longitude, and other movement metrics.
3.  **Local Display**:
    *   The 2.8-inch TFT LCD will display the current internal cabin temperature and the parsed GPS coordinates.
4.  **CAN Bus Telemetry**:
    *   The STM32 periodically packages the collected temperature and GPS data into standard 8-byte CAN frames and broadcasts them over the CAN network.
5.  **External Weather API Integration**:
    *   On a connected PC, a companion script (e.g., Python `python-can`) monitors the CAN bus, reads the GPS coordinates, and then queries an online weather API (like OpenWeather).
    *   The script then transmits the retrieved outdoor temperature back to the STM32 board via the CAN bus.
6.  **Interactive Display (Example)**:
    *   The BME280 driver was tested with an example where an onboard LED turns ON if the measured temperature exceeds a predefined threshold, and OFF otherwise. This demonstrates the reactive capabilities of the system.

## Hardware Pinout / Interconnect Map

### BME280 Sensor Wiring (I2C Mode)

| Sensor Pin | STM32 Connection | Notes |
| :--------- | :--------------- | :------------------------------- |
| VCC        | 3.3V             | **Important**: BME280 is 3.3V only. |
| GND        | GND              |                                  |
| SCL        | PB6 (I2C1_SCL)   |                                  |
| SDA        | PB7 (I2C1_SDA)   |                                  |
| CSB        | 3.3V             | Connect HIGH for I2C mode.       |
| SDO        | GND              | Connect LOW for I2C address `0xEC`. |

### General Peripheral Pin Assignments
Specific GPIOs for other peripherals (UART, SPI, CAN) should be assigned and physically wired to the NUCLEO-F446RE board headers as per the STM32CubeIDE project configuration and component datasheets.
*   **UART GPS Module**: Typically uses `USART1_RX` and `USART1_TX` lines.
*   **SPI Display**: Uses `SPI1_SCK`, `SPI1_MISO`, `SPI1_MOSI` along with dedicated digital GPIO pins for `LCD_DC`, `LCD_CS`, and `LCD_RST`.
*   **CAN Controller**: Uses `CAN1_RX` and `CAN1_TX` functional pins to link to the SN65HVD230 transceiver board.
