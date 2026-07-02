```markdown
---
badges:
  - type: status
    label: Status
    message: In Progress
    color: yellow
  - type: version
    label: Version
    message: 0.5.3
    color: blue
  - type: size
    label: Project Size
    message: Small (6+ components)
    color: orange
---

# CAN-Enabled Weather Telematics Dashboard

This project develops a multi-threaded, FreeRTOS-based telematics dashboard for the STM32 Nucleo-F446RE. It integrates industrial automotive protocol (CAN bus) communication with real-time sensor processing and cloud API data integration. The system captures local environmental data and GPS coordinates, which are transmitted over CAN to a companion Python gateway script. The Python script queries an online weather API and sends back outdoor conditions to be displayed on the embedded device.

## Table of Contents

- [System Architecture](#system-architecture)
  - [Hardware Architecture](#hardware-architecture)
  - [Firmware Architecture](#firmware-architecture)
  - [FreeRTOS Task Design](#freertos-task-design)
  - [Laptop Gateway (Python)](#laptop-gateway-python)
- [Tech Stack and Dependencies](#tech-stack-and-dependencies)
  - [Hardware Components](#hardware-components)
  - [Firmware Software](#firmware-software)
  - [Laptop Gateway Software](#laptop-gateway-software)
- [Features](#features)
- [Prerequisites / System Requirements](#prerequisites--system-requirements)
  - [Hardware Requirements](#hardware-requirements)
  - [Software Requirements](#software-requirements)
- [Step-by-Step Installation / Setup](#step-by-step-installation--setup)
  - [Firmware Setup (STM32CubeIDE)](#firmware-setup-stm32cubeide)
  - [Laptop Gateway Setup (Python)](#laptop-gateway-setup-python)
- [Configuration & Environment Variables](#configuration--environment-variables)
  - [Firmware Configuration](#firmware-configuration)
  - [Laptop Gateway Configuration](#laptop-gateway-configuration)
- [Quick Start / Usage Examples](#quick-start--usage-examples)
- [Hardware Pinout / Interconnect Map](#hardware-pinout--interconnect-map)
  - [BME280 Environmental Sensor (I2C)](#bme280-environmental-sensor-i2c)
  - [General Peripheral Interconnects](#general-peripheral-interconnects)

## System Architecture

The core of the system is an STM32 Nucleo-F446RE microcontroller running FreeRTOS. It manages multiple peripherals for data acquisition and display, while communicating with a desktop application over a CAN bus.

### Hardware Architecture

-   **Microcontroller**: STM32 Nucleo-F446RE (ARM Cortex-M4)
-   **Local Sensing**: I2C Waveshare BME280 environmental sensor (temperature, humidity, pressure)
-   **Location Tracking**: UART GY-NEO6MV2 GPS Module (NMEA sentences)
-   **User Interface**: 2.8-inch SPI Display (Elecrow ILI9341 Driver)
-   **Vehicle Network Interface**: STM32 bxCAN1 peripheral connected to a Waveshare SN65HVD230 CAN Transceiver.
-   **Diagnostic Bridge**: DSD TECH SH-C30A USB-to-CAN Adapter connects the STM32's CAN lines to a laptop.

### Firmware Architecture

The firmware utilizes the ST HAL library for peripheral management and FreeRTOS for concurrency. Key architectural components include:

-   **Multi-threaded System**: FreeRTOS tasks manage system concurrency, preventing blocking delays.
-   **Custom NMEA GPS Parser**: Manually parses `$GPRMC` and `$GPGGA` sentences via UART IDLE interrupt or circular DMA.
-   **CAN Data Framing**: GPS coordinates are manually packed into 8-byte CAN data frames.
-   **Modular Driver Design**: Adheres to strict coding guidelines, using custom status enums and handle-based designs for peripherals.

### FreeRTOS Task Design

The system employs a two-task architecture to balance display responsiveness and network bandwidth efficiency:

```
		┌────────────────────────────────────────────────────────┐
		│                   FreeRTOS Scheduler                   │
		└───────────────────────────┬────────────────────────────┘
									│
				 ┌──────────────────┴──────────────────┐
				 ▼                                     ▼
┌─────────────────────────────────┐   ┌─────────────────────────────────┐
│     Task 1: Live Sensor Loop    │   │   Task 2: Weather/GPS Telemetry │
├─────────────────────────────────┤   ├─────────────────────────────────┤
│ • Frequency: Every 500ms        │   │ • Frequency: Every 5-15 minutes │
│ • Priority: Normal (Higher)     │   │ • Priority: Low                 │
│ • Read BME280 (I2C)   	  	  │   │ • TX GPS via CAN                │
│ • Drive LCD Screen instantly    │   │ • RX Packed Weather             │
└─────────────────────────────────┘   └─────────────────────────────────┘
```

-   **Task 1: Live Sensor Loop (High Priority)**: Runs every 500ms, reads local BME280 sensor data, and instantly updates the top half of the LCD.
-   **Task 2: Weather & GPS Telemetry Loop (Low Priority)**: Runs every 5-15 minutes, transmits GPS coordinates over CAN, waits for the laptop's API response, and updates the bottom half of the LCD with outside data.
-   **Concurrency Safeguards**:
    -   **Preemption**: Higher-priority Sensor task can preempt the low-priority GPS task.
    -   **Mutex**: A FreeRTOS Mutex protects the SPI display, preventing simultaneous writes.
    -   **Binary Semaphore**: The Telemetry task uses a Binary Semaphore to asynchronously wait for CAN bus responses, preventing CPU waste.

### Laptop Gateway (Python)

A companion Python script (`laptop_gateway.py`) on a desktop PC performs the following:

-   Reads raw CAN data frames (latitude/longitude) from the STM32 via the USB-to-CAN adapter.
-   Queries the Open-Meteo API using the received coordinates.
-   Transmits the current outdoor temperature and weather condition back to the STM32 over the CAN bus.

## Tech Stack and Dependencies

### Hardware Components

-   **Microcontroller**: STMicroelectronics NUCLEO-F446RE Development Board (ARM Cortex-M4)
-   **CAN Transceiver**: Waveshare SN65HVD230 CAN Board
-   **Display**: Elecrow 2.8-Inch TFT LCD Touch Screen Shield Display (ILI9341 Driver)
-   **GPS Module**: GY-NEO6MV2 NEO-6M GPS Module with Ceramic Antenna
-   **Environmental Sensor**: Waveshare BME280 Environmental Sensor
-   **USB-to-CAN Adapter**: DSD TECH SH-C30A USB to CAN Bus Adapter

### Firmware Software

-   **RTOS**: FreeRTOS Kernel
-   **HAL**: STMicroelectronics STM32 HAL Library
-   **Programming Language**: C
-   **IDE**: STM32CubeIDE
-   **Embedded Drivers (Custom Implementations)**:
    -   BME280 Sensor (I2C)
    -   NEO-6M GPS (UART NMEA Parsing)
    -   ILI9341 TFT Display (SPI)
    -   STM32 bxCAN Controller

### Laptop Gateway Software

-   **Programming Language**: Python 3.x
-   **CAN Communication Library**: `python-can`, `pyusb`, `gs-usb`
-   **Weather API Client**: `openmeteo-requests`, `requests-cache`, `retry_requests`
-   **Weather API**: Open-Meteo API

## Features

-   **Real-time Local Environmental Monitoring**: Captures and displays indoor temperature, humidity, and barometric pressure using the BME280 sensor.
-   **GPS Location Tracking**: Parses NMEA sentences from the NEO-6M module to provide live latitude and longitude.
-   **CAN Bus Telemetry**: Bi-directional communication over CAN bus for transmitting GPS data and receiving outdoor weather updates.
-   **Interactive Display**: Renders real-time sensor values, GPS coordinates, and cloud-sourced outdoor weather conditions on an ILI9341 TFT display.
-   **Multi-threaded RTOS Architecture**: Ensures responsive operation through FreeRTOS task management, mutexes, and semaphores.
-   **Custom Driver Set**: Ground-up implementations for all key peripherals (BME280, NEO-6M, ILI9341, CAN bus) for full control and optimization.
-   **Python API Gateway**: Bridges the embedded system to the internet for fetching external weather data via the Open-Meteo API.
-   **Robustness & Fault Recovery**: Planned integration of Independent Hardware Watchdog Timer (IWDG) for system resets on application freezes.
-   **Memory Optimization**: Stack diagnostics and memory allocation tuning to reclaim unused RAM.
-   **CAN Network Filtering**: Hardware filter banks configured to drop irrelevant CAN frames, ensuring high-priority tasks remain responsive under network load.

## Prerequisites / System Requirements

### Hardware Requirements

The following hardware components are required for this project:

-   STMicroelectronics NUCLEO-F446RE Development Board
-   Waveshare SN65HVD230 CAN Board
-   Elecrow 2.8-Inch TFT LCD Touch Screen Shield Display (ILI9341 Driver)
-   GY-NEO6MV2 NEO-6M GPS Module with Ceramic Antenna
-   Waveshare BME280 Environmental Sensor
-   DSD TECH SH-C30A USB to CAN Bus Adapter
-   Breadboard, jumper wires, and 120-ohm CAN bus termination resistor.

### Software Requirements

-   **Development PC**: Windows, macOS, or Linux operating system.
-   **Firmware Development**:
    -   STM32CubeIDE (latest version recommended)
-   **Laptop Gateway Development**:
    -   Python 3.x installed (e.g., Python 3.8+)
    -   Virtual environment tool (`venv` or `conda`)

## Step-by-Step Installation / Setup

### Firmware Setup (STM32CubeIDE)

1.  **Initialize STM32CubeIDE Project**: Create a new C project for the NUCLEO-F446RE.
2.  **Clock Configuration**: Configure the main PLL to run the Core CPU at 180 MHz using HSE. Set APB1 clock prescaler to keep the bus at or below 45 MHz (for bxCAN1, I2C1).
3.  **Peripheral Configuration**:
    -   **I2C1**: Enable and configure for BME280 (Standard or Fast Mode, 7-bit addressing).
    -   **USART1**: Enable and configure for NEO-6M GPS (9600 baud, 8N1). Allocate a dedicated DMA channel in Circular Buffer Mode for non-blocking reception.
    -   **SPI1**: Enable as Master for ILI9341 Display (Mode 0: CPOL=0, CPHA=0), set clock prescaler for 10-22.5 MHz.
    -   **CAN1**: Configure for target bus frequency (e.g., 500 kbps, 45 MHz clock, Prescaler 5, BS1 13 $t_q$, BS2 2 $t_q$, SJW 1 $t_q$).
    -   **FreeRTOS**: Enable FreeRTOS, configure heap size and tick timer. Set `configMAX_SYSCALL_INTERRUPT_PRIORITY` for NVIC interrupt optimization.
    -   **IWDG**: Initialize the independent Hardware Watchdog Timer.
4.  **Integrate Drivers**:
    -   Place custom driver code (e.g., `bme280.c/.h`, `neo6m.c/.h`, `ili9341.c/.h`, `can_bus.c/.h`) into a `Drivers/` directory.
    -   Update C/C++ build settings to include driver directories in compiler include paths.

### Laptop Gateway Setup (Python)

1.  **Navigate to Project Root**: Open your terminal and change directory to the project's Python API folder.
2.  **Create Virtual Environment**:
    ```bash
    python -m venv .venv
    ```
3.  **Activate Virtual Environment**:
    -   **Windows Command Prompt**: `.\.venv\Scripts\activate.bat`
    -   **Windows PowerShell**: `.\.venv\Scripts\Activate.ps1`
    -   **Linux/macOS**: `source ./.venv/bin/activate`
4.  **Install Dependencies**:
    ```bash
    pip install -r requirements.txt
    ```
5.  **IDE Configuration**: Ensure your IDE's Python Interpreter is set to the virtual environment's Python executable (e.g., `.\.venv\Scripts\python.exe`).

## Configuration & Environment Variables

### Firmware Configuration

-   **BME280 I2C Address**: The BME280 sensor is configured with I2C address `0xEC` (SDO pin grounded).
-   **GPS UART**: USART1 configured to 9600 baud, 8N1.
-   **SPI Display**: SPI1 as Master, Mode 0 (CPOL=0, CPHA=0).
-   **CAN Bus Bit-Timing**: Configured for 500 kbps with a 45 MHz APB1 clock, using a prescaler of 5, BS1 = 13 $t_q$, BS2 = 2 $t_q$, SJW = 1 $t_q$.
-   **FreeRTOS Task Priorities**: `Task_Live_Sensor_Loop` set to `osPriorityNormal`, `Task_Weather_GPS_Telemetry_Loop` set to `osPriorityBelowNormal`.

### Laptop Gateway Configuration

-   **Open-Meteo Model**: For optimized accuracy in North America, the Open-Meteo client parameters are configured to pull telemetry data from the Canadian Meteorological Centre's (CMC) model or Environment Canada's high-resolution systems (`gem_seamless`).

## Quick Start / Usage Examples

-   **Local Temperature Monitoring**: Upon firmware deployment, the BME280 sensor reads are displayed on the top half of the LCD, updating every 500ms. Touching the sensor should visibly increase the displayed temperature.
-   **GPS & Remote Weather Telemetry**: The system periodically transmits GPS coordinates to the laptop gateway. The laptop queries the Open-Meteo API and sends outdoor temperature/weather conditions back to the STM32, which are then displayed on the bottom half of the LCD.
-   **Diagnostic Monitoring**: The DSD TECH SH-C30A USB-to-CAN Adapter allows external PC software (e.g., Cangaroo) to monitor CAN bus traffic for debugging and validation.

## Hardware Pinout / Interconnect Map

### BME280 Environmental Sensor (I2C)

| Sensor Pin | STM32 Nucleo-F446RE Connection |
| :--------- | :----------------------------- |
| VCC        | 3.3V                           |
| GND        | GND                            |
| SCL        | PB6                            |
| SDA        | PB7                            |
| CSB        | 3.3V (HIGH for I2C mode)       |
| SDO        | GND                            |

### General Peripheral Interconnects

-   **GY-NEO6MV2 GPS**:
    -   `USART1_RX` connected to GPS TX
    -   `USART1_TX` connected to GPS RX
-   **Elecrow 2.8-Inch TFT LCD (ILI9341)**:
    -   `SPI1_SCK`
    -   `SPI1_MISO`
    -   `SPI1_MOSI`
    -   GPIO pins for `LCD_DC` (Data/Command), `LCD_CS` (Chip Select), `LCD_RST` (Hardware Reset)
-   **Waveshare SN65HVD230 CAN Board**:
    -   `CAN1_RX`
    -   `CAN1_TX`
    -   Connects to DSD TECH SH-C30A USB-to-CAN Adapter (CAN_H/CAN_L lines)