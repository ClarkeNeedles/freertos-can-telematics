![Status](https://img.shields.io/badge/status-in--progress-yellowgreen) ![Version](https://img.shields.io/badge/version-0.0.0-blue)

# CAN-Enabled Weather Telematics Dashboard

## High-Level Overview

This project is an embedded telematics dashboard designed to bridge industrial automotive CAN bus data with real-time sensor processing and cloud API integration. Built on FreeRTOS, the system orchestrates multiple threads to manage hardware peripherals, ensuring non-blocking operation for UI rendering, GPS data acquisition, and CAN communication. A companion script running on a host PC queries an external weather API based on transmitted GPS coordinates, then relays the external weather back to the embedded device via CAN.

## Table of Contents

*   [System Architecture](#system-architecture)
*   [Tech Stack and Dependencies](#tech-stack-and-dependencies)
*   [Features](#features)
*   [Prerequisites / System Requirements](#prerequisites--system-requirements)
*   [Step-by-Step Installation / Setup](#step-by-step-installation--setup)
*   [Configuration & Environment Variables](#configuration--environment-variables)
*   [Quick Start / Usage Examples](#quick-start--usage-examples)
*   [Hardware Pinout / Interconnect Map](#hardware-pinout--interconnect-map)

## System Architecture

The **STM32 F446RE Nucleo** serves as the central processing unit, executing a FreeRTOS-based multi-threaded application. It reads environmental data via I2C and location metrics via UART, processing these inputs alongside incoming vehicular CAN Bus messages. The system visually updates data on an SPI-driven TFT LCD, and interfaces with diagnostic tools using an external USB-to-CAN Adapter.

### Hardware Architecture

*   **Microcontroller**: STM32 Nucleo-F446RE (ARM Cortex-M4)
*   **Primary Peripherals**:
    *   I2C Temperature Sensor (Waveshare BME280)
    *   UART GPS Module (GY-NEO6MV2 NEO-6M, streaming NMEA sentences)
    *   2.8-inch SPI Display (Elecrow 2.8-Inch TFT LCD Touch Screen Shield with ILI9341 Driver)
    *   CAN Controller (STM32 bxCAN1, interfaced via Waveshare SN65HVD230 CAN Board)
*   **Desktop Bridge**: DSD TECH SH-C30A USB-to-CAN Adapter connects the STM32 CAN lines to a laptop for external data processing and diagnostics.

### Software Architecture

The embedded software utilizes the ST HAL library for peripheral management and FreeRTOS for concurrency, preventing blocking delays.

**FreeRTOS Task Breakdown:**

*   **`Task_GPS_Parser` (High Priority)**: Monitors the UART buffer, processes raw NMEA data (specifically `$GPRMC` and `$GPGGA` sentences) via a UART IDLE interrupt or circular DMA, and pushes parsed coordinates to a FreeRTOS Queue.
*   **`Task_Local_Telemetry` (Medium Priority)**: Wakes up at fixed intervals using `vTaskDelayUntil` to sample the I2C temperature sensor and pushes local data to the display queue.
*   **`Task_CAN_Comms` (High Priority)**: Pops coordinates from the GPS queue, frames them into 8-byte CAN packets for transmission, and handles incoming CAN packets (e.g., outdoor temperature from the laptop).
*   **`Task_Display_UI` (Low Priority)**: Drives the SPI display using a graphics framework, using a FreeRTOS Mutex to secure the SPI bus, and renders indoor temperature, GPS location, and outdoor API weather updates.

**Laptop/API Layer Node:**

*   A companion script (Python-based) reads raw CAN data via the USB-to-CAN adapter, extracts latitude/longitude coordinates, queries the OpenWeather API, and transmits the current outdoor temperature back over the CAN bus to the STM32 board.

## Tech Stack and Dependencies

### Hardware Components

*   **Main Processor**: STMicroelectronics NUCLEO-F446RE Development Board
*   **CAN Transceiver**: Waveshare SN65HVD230 CAN Board
*   **Display**: Elecrow 2.8-Inch TFT LCD Touch Screen Shield (ILI9341 Driver)
*   **GPS Module**: GY-NEO6MV2 NEO-6M GPS Module with Ceramic Antenna
*   **Environmental Sensor**: Waveshare BME280 Environmental Sensor
*   **CAN Adapter**: DSD TECH SH-C30A USB to CAN Bus Adapter

### Software Frameworks & Libraries

*   **RTOS**: FreeRTOS Kernel
*   **HAL**: STMicroelectronics STM32 HAL Library
*   **IDE**: STM32CubeIDE (for C project development)
*   **BME280 Driver**: [Afebia/BME280-STM32-V2](https://github.com/Afebia/BME280-STM32-V2)
*   **GPS NMEA Parser**: [leech001/gps](https://github.com/leech001/gps) (for asynchronous NMEA sentence parsing)
*   **ILI9341 Display Driver**: [martnak/STM32-ILI9341](https://github.com/martnak/STM32-ILI9341)
*   **CAN Protocol Base**: [WassimHedfi/CAN_Protocol_STM32f446re_V_STM32f407G](https://github.com/WassimHedfi/CAN_Protocol_STM32f446re_V_STM32f407G)
*   **PC Side (Python)**: `python-can` library

## Features

The system is designed to provide real-time telematics data, featuring:

**Data Acquisition & Processing**
*   **Real-time Environmental Sensing**: Acquisition of ambient temperature, humidity, and barometric pressure via I2C.
*   **Precise GPS Tracking**: Streaming satellite data parsing (NMEA `$GPRMC` and `$GPGGA`) for latitude, longitude, elevation, timestamp, and speed vectors via UART.
*   **Non-Blocking Sensor Ingestion**: Utilization of Direct Memory Access (DMA) in circular buffer mode for continuous, CPU-offloaded GPS string capture.
*   **NMEA Sentence Parsing**: Integration of a thread-safe NMEA engine to extract relevant location and movement metrics.
*   **CAN Bus Communication**: Transmitting 8-byte CAN frames containing packaged temperature values and GPS coordinates.
*   **External Weather Integration**: Companion Python script on a PC to query OpenWeather API based on transmitted GPS data and relay outdoor temperature back to the STM32 via CAN.

**Embedded System Management**
*   **Multi-Tasking RTOS**: FreeRTOS kernel for managing concurrent operations, scheduling tasks, and prioritizing execution.
*   **Inter-Task Communication**: Secure data exchange between FreeRTOS tasks using queues for parsed sensor and CAN data.
*   **Resource Guarding**: Implementation of FreeRTOS Mutexes to protect shared resources, specifically the SPI bus for display operations, preventing data corruption.
*   **Interrupt-Driven CAN RX**: Highly optimized Interrupt Service Routine (ISR) for incoming CAN frames, utilizing task notifications for efficient ISR deferral.
*   **Watchdog Integration**: Independent Hardware Watchdog Timer (IWDG) to automatically reset the microcontroller in case of application freezes, with software task check-ins for health monitoring.
*   **Memory Management**: Runtime stack diagnostics (High Water Mark) and stack size tuning for optimal RAM utilization.
*   **Hardware CAN Filtering**: Configuration and validation of STM32's hardware filter banks to drop irrelevant CAN traffic at the peripheral level, ensuring efficient processing.

**User Interface & Diagnostics**
*   **Graphical Display**: Rendering of live sensor values, GPS positions, and external weather updates on a 2.8-inch SPI TFT LCD.
*   **PC Diagnostics**: Monitoring, debugging, and packet injection capabilities through a USB-to-CAN adapter and PC software (e.g., Cangaroo).

## Prerequisites / System Requirements

### Hardware Components

Ensure you have the following hardware components:

*   STMicroelectronics NUCLEO-F446RE Development Board
*   Waveshare SN65HVD230 CAN Board
*   Elecrow 2.8-Inch TFT LCD Touch Screen Shield Display (ILI9341 Driver)
*   GY-NEO6MV2 NEO-6M GPS Module with Ceramic Antenna
*   Waveshare BME280 Environmental Sensor
*   DSD TECH SH-C30A USB to CAN Bus Adapter
*   Breadboard and jumper wires
*   120-ohm termination resistor for CAN bus
*   PC with USB ports

### Software and Toolchain

*   **STM32CubeIDE**: For project initialization, configuration, and embedded C development.
*   **FreeRTOS Kernel**: The standard distribution source tracks.
*   **PC CAN Logging Utility**: Software like Cangaroo or native Linux SocketCAN utilities for CAN bus monitoring.
*   **Python Environment**: For the companion laptop script (ensure `python-can` is installed: `pip install -r requirements.txt` if a `requirements.txt` is provided in the Python API folder).

## Step-by-Step Installation / Setup

This project follows a multi-phase iterative development roadmap.

### Phase 1: Individual Bare-Metal Hardware Sandboxing

**Goal**: Validate each individual hardware module, wiring, and basic communication with the STM32F446RE using simple, sequential, blocking drivers.

1.  **Baseline Toolchain Setup & Clock Configuration**
    *   Initialize a clean, non-RTOS C project in STM32CubeIDE for the NUCLEO-F446RE.
    *   Configure the main Phase-Locked Loop (PLL) to run the Core CPU clock at 180 MHz using the external crystal oscillator (HSE).
    *   Set the APB1 clock prescaler to keep the low-speed peripheral bus at or below 45 MHz (for bxCAN1 and I2C1).
2.  **Environmental Sensing Sandbox (Waveshare BME280)**
    *   **Pin Mapping**: Assign and wire I2C1_SCL and I2C1_SDA lines. Ensure internal pull-ups are active if the breakout board lacks them.
    *   **Configuration**: Initialize STM32 I2C1 in Standard Mode (100 kHz) or Fast Mode (400 kHz) with 7-bit addressing.
    *   **Verification**: Execute `HAL_I2C_Mem_Read()` to target BME280 device address (0xD0). Confirm returned byte is `0x60`.
    *   **Driver Setup**: Import [Afebia/BME280-STM32-V2](https://github.com/Afebia/BME280-STM32-V2) and map its read/write functions to STM32 HAL I2C transfer functions.
    *   **Functional Test**: Loop data fetches, parse raw bytes into metrics, and print live temperature values to a PC serial terminal.
3.  **Satellite Location Sandbox (GY-NEO6MV2 GPS)**
    *   **Pin Mapping**: Assign and wire USART1_RX and USART1_TX lines to the GPS module.
    *   **Configuration**: Configure STM32 USART1 to 9600 baud, 8 data bits, 1 stop bit, no parity (8N1).
    *   **Functional Test**: Use a blocking `HAL_UART_Receive()` loop to stream incoming bytes to a PC screen.
    *   **Verification**: Visually confirm NMEA sentences (`$GPRMC`, `$GPGGA`) flow cleanly on the terminal.
4.  **Display Subsystem Sandbox (Elecrow 2.8" LCD)**
    *   **Pin Mapping**: Assign SPI1_SCK, SPI1_MISO, SPI1_MOSI. Allocate three GPIOs for LCD_DC, LCD_CS, and LCD_RST.
    *   **Configuration**: Initialize STM32 SPI1 as Master, setting clock prescaler for 10-22.5 MHz (Mode 0: CPOL=0, CPHA=0).
    *   **Driver Setup**: Import [martnak/STM32-ILI9341](https://github.com/martnak/STM32-ILI9341) and update its config header for allocated GPIOs.
    *   **Verification**: Execute initialization sequence to clear sleep mode. Run a bare-metal SPI loop to fill the LCD panel with a solid color (e.g., bright green).
5.  **Vehicle Network Interface Sandbox (STM32 bxCAN1)**
    *   **Pin Mapping**: Map CAN1_RX and CAN1_TX to the Waveshare SN65HVD230 transceiver board.
    *   **Physical Network Setup**: Bridge transceiver outputs (CAN_H / CAN_L) to the DSD TECH SH-C30A USB-to-CAN Adapter. Ensure a 120-ohm termination resistor is present. Connect the USB adapter to a PC running Cangaroo.
    *   **Bit-Timing Calculations**: Configure CAN registers for 250 kbps or 500 kbps (e.g., for 500 kbps via 45 MHz Clock: Prescaler 5, BS1=13, BS2=2, SJW=1, Sample Point at 87.5%).
    *   **Verification**: Send a hardcoded 8-byte diagnostic frame using `HAL_CAN_AddTxMessage()`. Confirm packet registers in PC software without bus-error flags.

### Phase 2: Combined Bare-Metal Integration

**Goal**: Integrate all devices in a single standard bare-metal `main()` loop, resolving resource conflicts before introducing an RTOS kernel.

1.  **Non-Blocking Sensor Background Ingestion**
    *   **GPS Background Stream**: Configure USART1 with a dedicated Direct Memory Access (DMA) channel in Circular Buffer Mode. Allocate a volatile 512-byte array for memory.
    *   **DMA Triggering**: Invoke `HAL_UART_Receive_DMA()` once at bootup to continuously stream GPS strings into RAM.
    *   **NMEA Parsing Implementation**: Integrate [leech001/gps](https://github.com/leech001/gps) to detect newline tokens, extract complete sentences, and parse latitude, longitude, and movement metrics from the circular buffer.
2.  **Local Bus Orchestration & Single Loop Execution**
    *   **Synchronous Data Readout**: Structure `while(1)` loop to sequentially read BME280 data via I2C and pull GPS positions from the DMA routine.
    *   **Display Rendering Integration**: Package computed variables into formatted strings and draw them onto specific pixel zones of the ILI9341 display using SPI driver functions.
    *   **CAN Payload Packaging**: Assemble temperature and GPS location fields into 8-byte data arrays and broadcast these payloads continuously over CAN.

### Phase 3: The FreeRTOS Transition

**Goal**: Deploy integrated drivers as independent threads within a managed, preemptive multi-task application layout.

1.  **Kernel Deployment & Priority Infrastructure**
    *   **Engine Importation**: Add FreeRTOS kernel source tracks to the build framework.
    *   **System Configuration**: Set system heap size and tick timer in `FreeRTOSConfig.h`.
    *   **NVIC Interrupt Optimization**: Reconfigure NVIC settings to move hardware interrupt preemption levels below `configMAX_SYSCALL_INTERRUPT_PRIORITY`.
2.  **Interrupt Service Routine (ISR) Deferral Logic**
    *   **Event Vector Definition**: Assign active global event tracking targets for CAN RX FIFO 0 and FIFO 1 channels in the vector table.
    *   **Optimized CAN ISR**: Write a dedicated ISR to:
        *   Ingest raw identifier, length, and payload bytes from bxCAN registers into a fast C-struct.
        *   Release hardware FIFO mailbox buffer space immediately.
        *   Issue a non-blocking `xTaskNotifyFromISR()` to signal the primary data-handling thread.
        *   Invoke `portYIELD_FROM_ISR()` to trigger an immediate scheduler context switch.
3.  **Inter-Task Concurrency & Mutex Guarding**
    *   **Safe Packet Queues**: Instantiate thread-safe messaging structures (`QueueHandle_t`) to move parsed CAN structures to downstream logic.
    *   **Resource Bus Locks**: Generate a FreeRTOS Mutex instance to guard the SPI communication channel, requiring tasks to acquire the lock before updating screen properties.
4.  **Task Implementation & Scheduler Activation**
    *   **High-Priority Task (CAN Processor)**: Create a task blocked until woken by the CAN ISR, then handles parsing incoming network traffic and pushing frames to internal processing.
    *   **Medium-Priority Task (Display Master)**: Create a task for UI management, updating text readouts and telemetry on the LCD, acquiring the SPI Mutex lock before writing.
    *   **Low-Priority Task (Sensor Aggregator)**: Create a cyclic task to sleep for defined intervals (e.g., `vTaskDelay(pdMS_TO_TICKS(500))`), polling the BME280 and processing new GPS coordinates from the DMA buffer every 500 ms.
    *   **Launch**: Call `vTaskStartScheduler()` to transfer execution control to the FreeRTOS kernel.

### Phase 4: Stability Optimization & Stress Testing

**Goal**: Harden the telemetry system against real-world electrical noise, network dropouts, or message collisions.

1.  **Watchdog Integration & Fault Recovery**
    *   **Hardware Watchdog Setup**: Initialize the Independent Hardware Watchdog Timer (IWDG) to reset the microcontroller if the application freezes.
    *   **Software Task Check-Ins**: Implement a diagnostic monitoring loop requiring all active FreeRTOS threads to check in and register healthy execution flags before resetting the system watchdog.
2.  **Memory Management & Performance Tuning**
    *   **Stack Diagnostics**: Simulate high-traffic environments and run `uxTaskGetStackHighWaterMark()` on all active threads.
    *   **Memory Allocation Tuning**: Adjust stack size variables in allocation statements to reclaim unused RAM bytes based on diagnostics.
3.  **Network Filtering Validation & Verification**
    *   **Hardware Filter Testing**: Use the external PC USB-to-CAN tool to inject thousands of random CAN frames with irrelevant identifiers onto the bus.
    *   **Drop Verification**: Verify that the STM32's hardware filter banks instantly drop unwanted background noise, keeping the high-priority FreeRTOS processing task responsive.
    *   **Visual Synchronization Check**: Confirm the LCD display continues to render changing GPS positions and ambient metrics smoothly under extreme network load.

## Configuration & Environment Variables

Key configurations and settings essential for the project:

*   **System Clock**: Core CPU clock configured at 180 MHz, APB1 clock at or below 45 MHz.
*   **I2C1 Peripheral**: Initialized in Standard Mode (100 kHz) or Fast Mode (400 kHz) with standard 7-bit addressing.
*   **USART1 Peripheral**: Configured to 9600 baud, 8 data bits, 1 stop bit, and no parity (8N1).
*   **SPI1 Peripheral**: Initialized as a Master, with a clock prescaler resulting in a frequency between 10 MHz and 22.5 MHz (using Mode 0: CPOL=0, CPHA=0).
*   **CAN Bit-Timing**: Configured for standard bus frequencies (e.g., 250 kbps or 500 kbps).
    *   *Example (500 kbps via a 45 MHz Clock)*: Prescaler of 5, Phase Segment 1 (BS1) as 13 time quanta ($t_q$), Phase Segment 2 (BS2) as 2 $t_q$, and Sync Jump Width (SJW) as 1 $t_q$. The sample point is verified at 87.5%.
*   **FreeRTOS Kernel**: Heap size and tick timer configurations defined within `FreeRTOSConfig.h`.
*   **NVIC Interrupt Priorities**: Reconfigured such that hardware interrupt preemption levels are below `configMAX_SYSCALL_INTERRUPT_PRIORITY`.
*   **CAN Hardware Filters**: Configured to filter out irrelevant CAN frames at the hardware level.

## Quick Start / Usage Examples

The core functionality involves a symbiotic interaction between the embedded STM32 device and a companion Python script running on a host PC.

1.  **Embedded Device Operation**:
    *   Upon power-up, the STM32 board initializes all peripherals and starts the FreeRTOS scheduler.
    *   The `Task_Local_Telemetry` continuously samples the BME280 sensor for indoor temperature.
    *   The `Task_GPS_Parser` continuously acquires and processes NMEA data from the GPS module.
    *   The `Task_CAN_Comms` packages parsed GPS coordinates (latitude, longitude) and sends them as 8-byte CAN frames onto the bus.
    *   The `Task_Display_UI` updates the LCD screen with the current indoor temperature and GPS location.

2.  **Companion PC Script Operation**:
    *   Run the Python companion script on a PC connected to the CAN bus via the DSD TECH SH-C30A USB-to-CAN adapter.
    *   The script (`python-can` based) will monitor the CAN bus for incoming GPS coordinate frames transmitted by the STM32.
    *   Upon receiving GPS data, the script extracts the latitude and longitude.
    *   It then queries an online service (e.g., OpenWeather API) using these coordinates to retrieve the current outdoor temperature.
    *   Finally, the script constructs a new CAN frame containing the outdoor temperature and transmits it back onto the CAN bus, which the STM32's `Task_CAN_Comms` will receive and the `Task_Display_UI` will render.

This setup enables real-time weather telematics, showing both local cabin conditions and external weather updates dynamically.

## Hardware Pinout / Interconnect Map

The following pin mappings are used for interconnecting the STM32 Nucleo-F446RE with the various peripherals:

*   **I2C1 (BME280 Environmental Sensor)**:
    *   `I2C1_SCL`
    *   `I2C1_SDA`
    *   *(Note: Internal pull-up options should be active if the BME280 breakout board lacks them.)*

*   **USART1 (GY-NEO6MV2 NEO-6M GPS Module)**:
    *   `USART1_RX`
    *   `USART1_TX`

*   **SPI1 (Elecrow 2.8" TFT LCD Display - ILI9341)**:
    *   `SPI1_SCK`
    *   `SPI1_MISO`
    *   `SPI1_MOSI`
    *   `LCD_DC` (GPIO pin for Data/Command selection)
    *   `LCD_CS` (GPIO pin for Chip Select)
    *   `LCD_RST` (GPIO pin for Hardware Reset)

*   **CAN1 (Waveshare SN65HVD230 CAN Board)**:
    *   `CAN1_RX`
    *   `CAN1_TX`
    *   *(Note: The SN65HVD230 outputs connect to the CAN_H / CAN_L lines of the DSD TECH SH-C30A USB-to-CAN Adapter. A 120-ohm termination resistor is required across the CAN bus.)*