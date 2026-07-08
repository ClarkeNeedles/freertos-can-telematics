---
title: "Project Shopping Cart"
date: 6-15-2026
status: planning
version: 0.0.0
---

# System Architecture Overview
The **STM32 F446RE Nucleo** acts as the central processing unit, reading real-time environmental data via **I2C** and location metrics via **UART**. It processes these inputs alongside incoming vehicular **CAN Bus** messages isolated through the **SN65HVD230 Transceiver**, visually updates the data on the **SPI-driven TFT LCD Shield**, and interfaces with diagnostic tools using the external **SH-C30A USB-to-CAN Adapter**.

---

## Component Breakdown

### 1. Main Processor
* **Component**: STMicroelectronics NUCLEO-F446RE Development Board
* **Communication Interface**: Master MCU Host (manages SPI, I2C, UART, and native bxCAN peripherals)
* **Function**: Runs the FreeRTOS kernel scheduler, handles data filtering tasks, executes calculations, and coordinates concurrent peripheral operations.
* **Estimated Price (CAD)**: ~$27.53 CAD
* **Purchase Link**: Available at [DigiKey Canada](https://www.digikey.ca/en/products/detail/stmicroelectronics/NUCLEO-F446RE/5347712).

### 2. Physical Transceiver
* **Component**: Waveshare SN65HVD230 CAN Board
* **Communication Interface**: CAN Protocol (Logic level RX/TX to differential CAN_H/CAN_L lines)
* **Function**: Acts as an interface layer between the STM32's micro-volt logic pins and the automotive standard differential network, protecting the microcontroller from high-voltage spikes.
* **Estimated Price (CAD)**: ~$7.34 CAD
* **Purchase Link**: Available at [RobotShop Canada](https://ca.robotshop.com/products/waveshare-can-board-sn65hvd230).

### 3. Display Subsystem
* **Component**: Elecrow 2.8-Inch TFT LCD Touch Screen Shield Display (ILI9341 Driver)
* **Communication Interface**: 4-Wire SPI (Serial Peripheral Interface)
* **Function**: Renders graphical user interfaces (GUIs), real-time sensor text values, system statuses, and captures physical tap coordinates on the resistive touch matrix.
* **Estimated Price (CAD)**: ~$15.99 CAD
* **Purchase Link**: Available at [Elecrow Store](https://elecrow.com).

### 4. Global Positioning System
* **Component**: GY-NEO6MV2 NEO-6M GPS Module with Ceramic Antenna
* **Communication Interface**: UART (Universal Asynchronous Receiver-Transmitter)
* **Function**: Parses real-time satellite data strings (NMEA sentences) to track precise latitude, longitude, elevation, timestamp metrics, and speed vectors.
* **Estimated Price (CAD)**: ~$14.74 CAD
* **Purchase Link**: Available at [Amazon Canada](https://www.amazon.ca/GY-NEO6MV2-NEO6MV2-Control-Antenna-Arduino/dp/B0CTG63TYF).

### 5. Environmental Telemetry
* **Component**: Waveshare BME280 Environmental Sensor
* **Communication Interface**: I2C (Inter-Integrated Circuit - Default Address: `0x76`)
* **Function**: Reads internal cabin atmosphere profiles including true ambient temperature, relative humidity levels, and local barometric pressure figures.
* **Estimated Price (CAD)**: ~$11.00 CAD
* **Purchase Link**: Available at [Canada Robotix](https://www.canadarobotix.com/collections/weather-and-environment-sensors).

### 6. Diagnostics & Diagnostic Testing Terminal
* **Component**: DSD TECH SH-C30A USB to CAN Bus Adapter
* **Communication Interface**: USB Type-A (PC side) to CAN Bus (3-Pin Terminal Block connecting to the system lines)
* **Function**: Bridges standard desktop software (e.g., Cangaroo or native Linux SocketCAN utilities) onto your physical controller environment to monitor, debug, inject, or read raw packet activity during validation testing.
* **Estimated Price (CAD)**: ~$19.99 CAD
* **Purchase Link**: Available at [Amazon Canada](https://www.amazon.ca/DSD-TECH-Adapter-Hardware-Canable/dp/B0BQ5G3KLR).

---

## Project Cost Summary

| Component | Rough Price (CAD) |
| :--- | :--- |
| STMicroelectronics NUCLEO-F446RE | $27.53 |
| Waveshare SN65HVD230 CAN Board | $7.34 |
| Elecrow 2.8" SPI TFT LCD Shield | $15.99 |
| GY-NEO6MV2 NEO-6M GPS Module | $14.74 |
| Waveshare BME280 Sensor Module | $11.00 |
| DSD TECH SH-C30A USB to CAN Adapter | $19.99 |
| **Total Rough Project Cost** | **$96.59 CAD** |

*Note: Total costs do not include applicable local taxes, shipping fees, breadboards, or jumper hookup wires.*
