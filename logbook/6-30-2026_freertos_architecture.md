---
title: "FreeRTOS task architecture"
date: 6-30-2026
status: in-progress
version: 0.4.2
---

# FreeRTOS Telematics Firmware Design

This document details the task architecture, concurrency safeguards, and display strategy for the STM32-side FreeRTOS telemetry firmware.

## 📌 Architecture Overview
The system splits firmware execution into two independent loops to balance crisp real-time display responsiveness with low network bandwidth consumption.

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
│ • Read BME280 (I2C)   	  │   │ • TX GPS via CAN                │
│ • Drive LCD Screen instantly    │   │ • RX Packed Weather             │
└─────────────────────────────────┘   └─────────────────────────────────┘
```

------------------------------
## 🕒 Task Planning Details
### 1. Task 1: Live Sensor Loop (Inside Conditions)

* Priority: High (osPriorityNormal)
* Timing: Runs quickly (every 500 milliseconds)
* Action: Reads the local hardware BME280 sensor via I2C and instantly updates the top half of the LCD with live indoor ambient telemetry.

### 2. Task 2: Weather & GPS Telemetry Loop (Outside Conditions)

* Priority: Low (osPriorityBelowNormal)
* Timing: Runs slowly (once every 5 to 15 minutes)
* Action: Transmits the current GPS coordinates over the CAN bus network layer, blocks execution while waiting for the laptop's API response, and updates the bottom half of the LCD with parsed outside data.

------------------------------
## 🛠️ The Three Concurrency Safeguards
To prevent hardware peripheral lockups and protect data integrity, the design relies on three standard FreeRTOS synchronization mechanisms:

* Preemption (Zero Display Stutter): Because the Sensor task possesses a higher priority hierarchy, the FreeRTOS scheduler instantly pauses the low-priority GPS task the exact millisecond the 500ms sensor timer expires. Local screen updates remain completely seamless, even if the 15-minute telemetry network cycle is currently processing data.
* Mutex (Screen Asset Protection): Both tasks share a single FreeRTOS Mutex which acts as the exclusive access key to the LCD screen lines. A task must successfully request the mutex before pulsing commands to the display and release it immediately when finished. This strict sequencing blocks simultaneous writes that would otherwise scramble display characters.
* Binary Semaphore (Asynchronous Waiting): After broadcasting its GPS coordinates, the Telemetry task does not waste execution cycles spinning in a heavy while() loop waiting for a response. Instead, it enters a 0% CPU sleep state. The moment the laptop sends the weather data frame back down the bus, the STM32 CAN Receive Interrupt Service Routine (ISR) intercepts the payload and flashes a Binary Semaphore to instantly wake the task up.

------------------------------
## 🖥️ Display Layout & Update Strategy
The display area is split horizontally into an "Inside Data" zone (top half) and an "Outside Data" zone (bottom half) using an optimized partial-redraw engine.
### 1. Draw the Static Template Once at Startup

* Action: Clear the screen frame and paint all permanent visual items (dividing grid lines, borders, and constant text text labels like "Temp:", "Humid:", or "Wind:") exactly once on boot-up before launching the FreeRTOS scheduler.
* Benefit: Minimizes SPI/I2C communication overhead by entirely eliminating the need to re-transmit background pixels that never change.

### 2. Update Only the Text Fields ("Dirty Rectangles")

* The Problem: Writing a fresh numeric text value directly on top of an old one mixes the characters together into an unreadable block of overlapping pixels.
* The Solution: Right before printing a fresh sensor update, command the display driver to draw a tiny, solid black rectangle over the exact coordinate boundaries of that specific variable textbox field. This safely wipes out the old characters, providing a clean canvas for the new string.

## ⚠️ Implementation Note: State Caching (Shadow Variables)
While tracking previously printed metrics using "shadow variables" to skip identical updates is a valuable long-term optimization tool, it is not necessary for initial development.
Because the text boxes and weather icons occupy tiny pixel arrays, the display driver is fast enough to blindly repaint the text fields and status symbols inside the bottom half of the screen every time the 15-minute telemetry routine completes without impacting system performance.

Pro Tip for FreeRTOS Setup: When initializing your tasks in STM32CubeMX, verify that you assign osPriorityNormal to your Live Sensor Loop and osPriorityBelowNormal to your Telemetry Loop to match this document's hardware requirements exactly.
Would you like help writing the C language function declarations or the FreeRTOS template creation boilerplate code for these specific configurations next?

