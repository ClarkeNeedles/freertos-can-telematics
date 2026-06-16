---
title: "Driver Sources + Notes"
date: 6-16-2026
status: in-progress
version: 0.0.0
---

## 🗺️ Peripheral Driver Reference Mapping
Investigated and selected the primary open-source driver repositories to integrate with the STM32 HAL framework:

* BME280 Sensor (I2C): Using [Afebia/BME280-STM32-V2](https://github.com/Afebia/BME280-STM32-V2) to handle compensation math and register extraction.
* NEO-6M GPS (UART): Using [leech001/gps](https://github.com/leech001/gps) for asynchronous NMEA sentence parsing.
* ILI9341 TFT Display (SPI): Using [martnak/STM32-ILI9341](https://github.com/martnak/STM32-ILI9341) for low-overhead GUI and typography rendering.
* bxCAN Network (CAN): Using [WassimHedfi/CAN_Protocol_STM32f446re_V_STM32f407G](https://github.com/WassimHedfi/CAN_Protocol_STM32f446re_V_STM32f407G) as the base framework for frame mailbox management.

------------------------------
## 🧠 Engineering Evaluation: Zambretti Forecaster Algorithm
Evaluated the feasibility of implementing the Zambretti Algorithm to derive local weather trend predictions (e.g., Sunny, Cloudy, Humid) directly on the firmware without internet access.
Decision: Dropped / Will Not Implement

* Reasoning: The environmental telemetry module will reside inside the vehicle cabin 100% of the time. Local climate control, cabin sealing, and vehicle motion will distort barometric trends.
* Conclusion: Implementing a 3-hour historical tracking buffer creates unnecessary extra firmware work for data that will not be accurate or useful for an indoor cabin environment.

## Folder Structure:

*  Dedicated STM32 folder
*  Dedicated Python API folder
