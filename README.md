# CAN-Enabled Weather Telematics Dashboard

## Overview
This project develops a FreeRTOS-based telematics dashboard utilizing an STM32 microcontroller. It integrates real-time sensor data, specifically GPS and local temperature, with industrial automotive CAN bus communication and cloud-based weather information. The system is designed to display critical environmental and positional data on a local SPI-driven screen while interacting with a desktop companion application for external API data retrieval.

## Features

### Embedded System Core
*   **Microcontroller**: STM32 Nucleo-F446RE (ARM Cortex-M4) serves as the central processing unit.
*   **Real-Time Operating System**: Implements FreeRTOS for robust multi-threaded task management, ensuring concurrent operation and responsiveness.
*   **Peripheral Integration**:
    *   **I2C**: Interface for accurate temperature sensor data acquisition.
    *   **UART**: Dedicated for communication with a GPS module, processing incoming NMEA sentences.
    *   **SPI**: Drives a 2.8-inch display for user interface output.
    *   **CAN**: Manages industrial automotive CAN bus communication.
*   **User Interface**: Renders indoor temperature, GPS location, and dynamic outdoor weather updates on the SPI display using an integrated graphics framework.

### Communication & Data Processing
*   **Custom GPS Parser**: Features a manual NMEA GPS string parser specifically for `$GPRMC` and `$GPGGA` sentences, utilizing UART IDLE interrupt or circular DMA for efficient data reception.
*   **GPS Data Encoding**: Efficiently packs parsed raw GPS coordinates (latitude/longitude) into standard 8-byte CAN data frames for transmission.
*   **CAN Bus Telemetry**:
    *   Transmits processed local telemetry, including GPS coordinates, across the CAN bus.
    *   Receives external weather data, specifically outdoor temperature, from the companion desktop application via the CAN bus.
*   **Local Sensor Data**: Samples the I2C temperature sensor at predefined, fixed intervals.

### Desktop Companion
*   **CAN-to-USB Bridge**: Establishes communication between the STM32's CAN bus and a host laptop through a hardware adapter.
*   **CAN Data Ingestion**: Reads raw CAN data frames, extracting GPS coordinates transmitted by the embedded system.
*   **Cloud API Integration**: Queries the OpenWeather API using the extracted GPS coordinates to obtain real-time outdoor weather conditions.
*   **External Data Transmission**: Transmits the retrieved current outdoor temperature back to the STM32 embedded board over the CAN bus.

## Tech Stack
*   **Embedded Hardware**: STM32 Nucleo-F446RE (ARM Cortex-M4), I2C Temperature Sensor, UART GPS Module, 2.8-inch SPI Display, CAN Controller, CAN-to-USB Hardware Adapter.
*   **Embedded Software**: C/C++, ST HAL Library, FreeRTOS.
*   **Desktop Software**: Python (with `python-can` library) or Rust.
*   **Cloud Services**: OpenWeather API.
*   **Protocols**: CAN Bus, I2C, UART (NMEA), SPI.

## Current Status
*   **Status**: In-Progress
*   **Version**: 0.0.0