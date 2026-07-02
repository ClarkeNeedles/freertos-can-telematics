---
title: "FreeRTOS task structure"
date: 7-2-2026
status: in-progress
version: 0.5.3
---

# Firmware Task Architecture & Modular Design
This document details the FreeRTOS task structure chosen for the CAN-Enabled Weather Telematics Dashboard. To maintain a clean separation of concerns and prevent code bloat within the auto-generated `freertos.c` file, the system utilizes STM32CubeMX's **"As External"** code generation strategy.

## Architecture Overview
Instead of placing core application logic inside the auto-generated `freertos.c` file, tasks are decoupled into dedicated domain files (`task_live_sensor.c` and `task_telemetry.c`). 

```
┌─────────────────────────┐
│ STM32CubeMX Tool        │
│ Task Gen: [As External] │
└────────────┬────────────┘
             │
             ▼
┌─────────────────────────┐
│ freertos.c              │
│ (Auto-Generated)        │
├─────────────────────────┤
│ extern void StartTask() │
│ osThreadNew(StartTask)  │
└─────────────────────────┘
             │
  (Linked at Compile Time)
             │
             ▼
┌─────────────────────────┐
│ task_live_sensor.c      │
│ (Custom Application)    │
├─────────────────────────┤
│ void StartTask(arg) {   │
│ // Implementation       │
│ }                       │
└─────────────────────────┘
```

---

## Configuration Steps (STM32CubeMX)

1. Open the project `.ioc` configuration file.
2. Navigate to **Middleware and Software Packs** > **FREERTOS**.
3. Select the **Tasks and Queues** tab.
4. Double-click the target task (e.g., `LiveSensorTask`).
5. Change the **Code Generation Option** dropdown from **Default** to **As external**.
6. Regenerate the code.

---

## File Structure Implementation

### 1. The Kernel Configuration (`freertos.c`)
When set to **As External**, CubeMX strictly acts as a system registry. It declares the function prototypes as external elements and registers them with the scheduler, leaving the function body completely out of the file.

```c
/* freertos.c (Partial view of auto-generated code) */

/* Hooks declaring that the actual logic lives in external application files */
extern void StartSensorTask(void *argument);
extern void StartTelemetryTask(void *argument);

void MX_FREERTOS_Init(void) {
  /* ... Hardware attributes setup ... */

  // System creation remains centralized here
  SensorTaskHandle = osThreadNew(StartSensorTask, NULL, &SensorTask_attributes);
  TelemetryTaskHandle = osThreadNew(StartTelemetryTask, NULL, &TelemetryTask_attributes);
}
```

### 2. Live Sensor Application Loop (`Core/Src/task_live_sensor.c`)
This file houses all interactions with local physical hardware via I2C and SPI.

```c
#include "cmsis_os2.h"
#include "bme280.h"
#include "ili9341.h"

/**
 * @brief High Priority Task: Reads BME280 every 500ms and updates LCD instantly.
 */
void StartSensorTask(void *argument)
{
    /* Initialize BME280 Sensor & Local Buffers */
    BME280_Init();
    
    for(;;)
    {
        /* Read local environmental data (I2C) */
        BME280_ReadData();
        
        /* Mutex lock SPI display and write to top half of screen */
        ILI9341_UpdateLocalDisplay();
        
        osDelay(500); 
    }
}
```

### 3. Telemetry & Cloud Gateway Loop (`Core/Src/task_telemetry.c`)
This file isolates heavy network waiting parsing logic away from the critical sensor updates.

```c
#include "cmsis_os2.h"
#include "can_bus.h"
#include "neo6m.h"

/**
 * @brief Low Priority Task: Handles background CAN communication and GPS telemetry.
 */
void StartTelemetryTask(void *argument)
{
    /* Initialize GPS Module UART DMA registers */
    GPS_Init();

    for(;;)
    {
        /* Pack and transmit GPS coordinates over CAN */
        CAN_TxGPSCoordinates();
        
        /* Block and wait asynchronously via binary semaphore for Python API reply */
        if(osSemaphoreAcquire(CanRxSemaphoreHandle, osWaitForever) == osOK)
        {
            /* Mutex lock SPI display and write weather data to bottom half of screen */
            ILI9341_UpdateWeatherDisplay();
        }
        
        /* Rest task for 5-15 minutes before the next cloud sync poll */
        osDelay(osKernelGetTickFreq() * 60 * 10); 
    }
}
```

---

## Architectural Advantages

* **No Code Boilerplate:** Removes the need for wrapping or "forwarding" dummy functions within `freertos.c`.
* **Immunity to Code Overwrites:** Application logic completely bypasses `freertos.c`, making updates or clock changes in the `.ioc` tool perfectly safe.
* **Domain Isolation:** High-frequency rendering loops are fully isolated from slow, blocking communication busses.

