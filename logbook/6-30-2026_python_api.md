---
title: "Python weather API"
date: 6-30-2026
status: in-progress
version: 0.4.2
---

# Python CAN-to-Weather Gateway Documentation

This document covers the configuration, environment setup, and development notes for the Python gateway script (laptop_gateway.py). This script bridges an STM32 microcontroller and the Open-Meteo weather API over a CAN-to-USB adapter

## 📌 Project Overview
The laptop gateway performs three core actions in a continuous loop:

   1. Listens for incoming CAN data frames containing GPS telemetry from the STM32.
   2. Interprets the raw binary payload into geographic coordinates (Latitude and Longitude).
   3. Queries a free weather API using those coordinates and transmits the current weather state back to the STM32 nodes over the CAN bus.

⚠️ Architecture Note: Development of the STM32 CAN driver layer is intentionally paused until FreeRTOS is fully integrated into the microcontroller firmware. CAN frame processing is highly dependent on hardware interrupt service routines (ISRs) and precise task scheduling timelines.

------------------------------
## ⚙️ Environment Setup
To keep dependencies isolated and avoid disrupting system-wide Python installations, you must deploy a local virtual environment (.venv) in the project root directory.
## 1. Initialize and Activate Virtual Environment
Open your terminal, navigate to your project root, and execute the following commands:

- Navigate to the project root directory
  - `cd /path/to/freertos-can-telematics`
- Create the virtual environment folder
  - `python -m venv .venv`
- Activate the environment (Windows Command Prompt)
  - `.\.venv\Scripts\activate.bat`
- Alternative: Activate the environment (Windows PowerShell)
  - `.\.venv\Scripts\Activate.ps1`

## 2. Dependency Management
The project dependencies are tracked inside requirements.txt. Key additions include python-can, pyusb, and gs-usb for hardware bridging, alongside the official Open-Meteo SDK modules (openmeteo-requests, requests-cache, retry_requests).
To install all required libraries at once, run:

`pip install -r requirements.txt`

## 3. IDE Integration Tip
If your code editor (e.g., VS Code or PyCharm) highlights import can or other packages with unresolved reference errors, ensure your editor's active Python Interpreter is explicitly mapped to the path inside your local virtual environment:
`.\.venv\Scripts\python.exe`

## 🌤️ API Integration Notes & Evolution
### The Shift to Open-Meteo

* Initial Plan: The architecture originally targeted the OpenWeatherMap API under the assumption it provided a completely frictionless free tier.
* The Issue: OpenWeatherMap now mandates a "Pay-As-You-Go" subscription registration linked to a valid credit card even to access baseline free tier thresholds.
* The Solution: Pivot to Open-Meteo. It is entirely free, open-source, requires no API tokens or account registration, and seamlessly delivers the identical data footprints needed for this telemetry loop.

### Live Validation & Model Adjustments
During early live integration tests, real-time conditions reported by Open-Meteo slightly deviated from local consumer weather platforms (e.g., reporting a WMO code of 0 for Clear Sky when local conditions were visually partly cloudy).
Global simulation models refresh on multi-hour intervals which can create real-time cloud-cover data lag. Because this project operates within North America, accuracy can be optimized by forcing the Open-Meteo client parameters to pull telemetry directly from the Canadian Meteorological Centre's (CMC) model or Environment Canada's high-resolution systems (gem_seamless).

## 🚀 What's Next?

* Complete the FreeRTOS task architecture skeleton on the STM32 core.
* Configure the STM32 CAN peripheral hardware filters and RX/TX FIFO buffers.
* Map out the switch-case WMO weather condition code decoding matrix inside the embedded application layer.
