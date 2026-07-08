---
title: "Project Init: CAN-Enabled Weather Telematics Dashboard"
date: 06-10-2026
status: planning
version: 0.0.0
---

# Log Notes
Initial system architecture design for the FreeRTOS-based telematics dashboard. The goal is to build a multi-threaded system that bridges an industrial automotive protocol (CAN bus) with real-time sensor processing and cloud API data integration. 

Hardware architecture details:
- Microcontroller: STM32 Nucleo-F446RE (ARM Cortex-M4).
- Primary Peripherals: I2C temperature sensor, UART GPS Module (streaming NMEA sentences), 2.8-inch SPI Display, CAN Controller.
- Desktop Bridge: CAN-to-USB hardware adapter connecting the STM32 CAN lines to the laptop.

Software architecture constraints:
- Must use the ST HAL library layer to easily manage peripheral setups alongside FreeRTOS.
- System concurrency will be managed via FreeRTOS tasks to prevent blocking delays from stalling the UI or losing serial data.
- Custom code requirement: Write a manual NMEA GPS string parser for the `$GPRMC` and `$GPGGA` sentences instead of downloading an existing library. This will parse raw coordinates via a UART IDLE interrupt or circular DMA.
- Data structures for the GPS coordinates must be packed into raw 8-byte CAN data frames manually.

FreeRTOS Task Breakdown:
1. `Task_GPS_Parser` (High Priority): Monitors UART buffer, processes raw data on incoming newline characters, and pushes parsed coordinates to a FreeRTOS Queue.
2. `Task_Local_Telemetry` (Medium Priority): Wakes up at a fixed interval via `vTaskDelayUntil` to sample the I2C temperature sensor. Pushes local data to the display queue.
3. `Task_CAN_Comms` (High Priority): Pops coordinates from the GPS queue, frames them into CAN packets, and sends them across the bus. It also handles incoming CAN packets transmitted by the laptop.
4. `Task_Display_UI` (Low Priority): Drives the SPI display using an automated graphics framework. Uses a FreeRTOS Mutex to secure the SPI bus, rendering indoor temperature, GPS location, and outdoor API weather updates.

Laptop/API Layer Node:
- Write a companion script (Python using `python-can` or Rust) on the laptop.
- It will read raw CAN data via the USB-to-CAN adapter, pull out the lat/long coordinates, query an online OpenWeather API, and transmit the current outdoor temperature back down over the CAN bus to the STM32 board.
