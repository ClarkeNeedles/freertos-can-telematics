---
title: "Project Development Plan"
date: 6-15-2026
status: planning
version: 0.0.0
---

## Multi-Phase Iterative Development Roadmap

[Phase 1: Basic Sandboxing] ──► Isolate & validate hardware 
[Phase 2: Bare-Metal Integration] ──► Combine devices without an RTOS
[Phase 3: FreeRTOS Migration] ──► Introduce RTOS kernel scheduler
[Phase 4: Optimization] ──► Stress-test memory & bus

------------------------------
## Phase 1: Individual Bare-Metal Hardware Sandboxing
Goal: Prove that each individual hardware module is alive, correctly wired to your breadboard, and communicating with the STM32F446RE using simple, sequential, blocking drivers. Do not share pins or combine code files yet.
### Step 1: Baseline Toolchain Setup & Clock Configuration

* 1.1 Initialize a clean, non-RTOS C project inside STM32CubeIDE specifically for the NUCLEO-F446RE.
* 1.2 Configure the main Phase-Locked Loop (PLL) configuration registers to run the Core CPU clock at its maximum 180 MHz using the external crystal oscillator (HSE) as its reference source.
* 1.3 Set the APB1 clock prescaler divider to keep the low-speed peripheral bus operating at or below its safe maximum speed threshold (45 MHz for bxCAN1 and I2C1).

### Step 2: Environmental Sensing Sandbox (Waveshare BME280)

* 2.1 Pin Mapping: Assign and physically wire the I2C1_SCL and I2C1_SDA lines on the Nucleo board headers. Ensure internal pull-up options are active if your breakout board lacks them.
* 2.2 Configuration: Initialize the STM32 I2C1 peripheral in Standard Mode (100 kHz) or Fast Mode (400 kHz) with standard 7-bit addressing.
* 2.3 Verification: Execute a simple blocking HAL_I2C_Mem_Read() routine targeting the BME280 device address to read the fixed identification register (0xD0). Assert that the returned byte value is exactly 0x60 to prove the hardware connection is electrically flawless.
* 2.4 Driver Setup: Import the open-source Bosch BME280 API Driver. Map its internal hardware read/write function pointers to thin wrapper functions that invoke your verified STM32 HAL I2C transfer functions.
* 2.5 Functional Test: Loop a simple blocking data fetch routine, parsing raw bytes into readable metrics using the Bosch API equations. Print the live temperature values out to an external PC serial terminal to confirm accuracy.

### Step 3: Satellite Location Sandbox (GY-NEO6MV2 GPS)

* 3.1 Pin Mapping: Assign and wire the USART1_RX and USART1_TX lines on the Nucleo board headers to connect the GPS module.
* 3.2 Configuration: Configure the STM32 USART1 peripheral structure to 9600 baud, 8 data bits, 1 stop bit, and no parity (8N1).
* 3.3 Functional Test: Write a temporary test loop utilizing a simple blocking HAL_UART_Receive() statement. Stream the incoming bytes from the receiver straight out through a debug logging link to your PC screen.
* 3.4 Verification: Visually confirm that formatting blocks of text strings starting with standard NMEA sequence markers (such as $GPRMC or $GPGGA) are cleanly flowing onto your terminal window.

### Step 4: Display Subsystem Sandbox (Elecrow 2.8" LCD)

* 4.1 Pin Mapping: Assign and connect the dedicated hardware pins for SPI1_SCK, SPI1_MISO, and SPI1_MOSI. Allocate three standard digital GPIO pins to serve as control signals: Data/Command selection (LCD_DC), Chip Select (LCD_CS), and Hardware Reset (LCD_RST).
* 4.2 Configuration: Initialize the STM32 SPI1 peripheral as a Master, setting the clock prescaler value to drop the SPI line frequency safely between 10 MHz and 22.5 MHz (using Mode 0: CPOL=0, CPHA=0).
* 4.3 Driver Setup: Import a lightweight ILI9341 display core port (such as the afiskon/stm32-ili9341 C library) and update its target platform config header to directly address your allocated control GPIO lines.
* 4.4 Verification: Execute the initialization sequence command macro to clear the screen controller out of sleep mode. Run a simple bare-metal SPI loop to issue a canvas fill layout parameter, painting the entire LCD panel a solid color (e.g., bright green) to confirm the glass matrix works.

### Step 5: Vehicle Network Interface Sandbox (STM32 bxCAN1)

* 5.1 Pin Mapping: Map the CAN1_RX and CAN1_TX functional pins to link your Nucleo processor core out to the Waveshare SN65HVD230 transceiver board transceiver inputs.
* 5.2 Physical Network Setup: Bridge the transceiver output terminals (CAN_H / CAN_L) over to your external DSD TECH SH-C30A USB-to-CAN Adapter using short twisted wires. Verify a 120-ohm termination resistor sits across the bus. Connect the USB adapter to a development PC running packet logging utilities (such as Cangaroo).
* 5.3 Bit-Timing Calculations: Configure the CAN operational registers to target standard bus frequencies (250 kbps or 500 kbps):
* Example (500 kbps via a 45 MHz Clock): Apply a clock Prescaler factor of 5 (yielding a time quanta $t_q$ of 111.11 ns). Define Phase Segment 1 (BS1) as 13 $t_q$, Phase Segment 2 (BS2) as 2 $t_q$, and the Sync Jump Width (SJW) as 1 $t_q$.
   * Sample Point Verification: Confirm total quanta sum up to 16, anchoring your data bit evaluation point exactly at the 87.5% sample point demanded by automotive network specifications.
* 5.4 Verification: Send a simple, hardcoded 8-byte diagnostic frame sequentially using standard blocking commands (HAL_CAN_AddTxMessage()). Confirm the test packet registers correctly inside your PC software trace panel without triggering bus-error status flags.

------------------------------
## Phase 2: Combined Bare-Metal Integration
Goal: Create a multi-device environment entirely in a single standard bare-metal main() loop. This step eliminates sensor line layout resource conflicts before introducing an RTOS kernel.
### Step 6: Non-Blocking Sensor Background Ingestion

* 6.1 GPS Background Stream: Alter your USART1 initialization plan to allocate a dedicated Direct Memory Access (DMA) channel working in Circular Buffer Mode. Set up a volatile 512-byte array tracking memory workspace in your primary source files.
* 6.2 DMA Triggering: Invoke the background command sequence (HAL_UART_Receive_DMA()) once at bootup. The internal MCU hardware will now continuously stream GPS tracking strings straight into your RAM structure background space without demanding CPU processing overhead.
* 6.3 NMEA Parsing Implementation: Integrate a thread-safe NMEA engine (such as the minmea C library) into your execution path. Write a routine that detects newline tokens inside your circular memory space, extracts complete sentence slices, and passes them over to parse latitude, longitude, and movement metrics.

### Step 7: Local Bus Orchestration & Single Loop Execution

* 7.1 Synchronous Data Readout: Structure your central execution block (while(1)) to read ambient profiles sequentially out from the BME280 sensor via I2C, while pulling the background GPS positions extracted via your DMA routine.
* 7.2 Display Rendering Integration: Package these computed live variables into formatted string segments and draw them onto specific textual pixel zones of your ILI9341 display matrix using your imported SPI display driver functions.
* 7.3 CAN Payload Packaging: Assemble those exact temperature values and GPS location fields into standard 8-byte diagnostic data array structures. Schedule an outgoing packet loop to broadcast these custom payloads continuously across the CAN network lines to your monitoring computer.

------------------------------
## Phase 3: The FreeRTOS Transition
Goal: Take your integrated driver collection and safely deploy them as independent threads within a managed, preemptive multi-task application layout.
### Step 8: Kernel Deployment & Priority Infrastructure

* 8.1 Engine Importation: Add the standard FreeRTOS kernel distribution source tracks into your active build framework directory.
* 8.2 System Configuration: Set up your system heap size variables and tick timer configurations inside the central FreeRTOSConfig.h operational control profile.
* 8.3 NVIC Interrupt Optimization: Reconfigure your processor's Nested Vectored Interrupt Controller (NVIC) settings. Move your hardware interrupt preemption levels down beneath the system-wide threshold parameter configMAX_SYSCALL_INTERRUPT_PRIORITY to ensure your driver calls do not compromise kernel timing loops.

### Step 9: Interrupt Service Routine (ISR) Deferral Logic

* 9.1 Event Vector Definition: Assign active global event tracking targets for your CAN RX FIFO 0 and FIFO 1 channels within the central vector table allocations.
* 9.2 Optimized CAN ISR: Write a dedicated, highly optimized Interrupt Service Routine handler for incoming frames. This function must only perform the following operations:
* Ingest raw identifier attributes, lengths, and payload bytes instantly from the bxCAN hardware registers into a fast local C-struct.
   * Release the hardware FIFO mailbox buffer space immediately to accept the next incoming wire frame.
   * Issue a non-blocking FreeRTOS macro notification (xTaskNotifyFromISR()) to signal your primary data-handling thread that information is waiting.
   * Invoke portYIELD_FROM_ISR() to trigger an immediate, deterministic scheduler context switch.

### Step 10: Inter-Task Concurrency & Mutex Guarding

* 10.1 Safe Packet Queues: Instantiate thread-safe messaging structures (QueueHandle_t) to move parsed CAN structures out from the receiver capture zones to your downstream logic applications.
* 10.2 Resource Bus Locks: Generate a dedicated FreeRTOS Mutex instance to guard your SPI communication channel. Force any task that wishes to update screen properties or touch coordinates to acquire this lock, preventing memory corruption or bus collisions.

### Step 11: Task Implementation & Scheduler Activation

* 11.1 High-Priority Task (CAN Processor): Create a thread that remains blocked until woken up by the CAN ISR. Once unblocked, it handles parsing out incoming network traffic and pushing frames into internal processing tracks.
* 11.2 Medium-Priority Task (Display Master): Create a thread tasked with managing the user interface. It updates numeric text readouts and system telemetry indicators on the LCD screen, acquiring the SPI Mutex lock before writing to the display.
* 11.3 Low-Priority Task (Sensor Aggregator): Create a cyclic thread configured to sleep for defined intervals (e.g., vTaskDelay(pdMS_TO_TICKS(500))). Every 500 ms, it polls the I2C BME280 sensor and processes new GPS coordinates from the background DMA ring buffer.
* 11.4 Launch: Call vTaskStartScheduler() to transfer execution control over to the FreeRTOS kernel engine.

------------------------------
## Phase 4: Stability Optimization & Stress Testing
Goal: Harden your telemetry system to survive real-world electrical noise, unexpected network dropouts, or message collisions.
### Step 12: Watchdog Integration & Fault Recovery

* 12.1 Hardware Watchdog Setup: Initialize the processor's independent Hardware Watchdog Timer (IWDG) to automatically reset the microcontroller if the application freezes.
* 12.2 Software Task Check-Ins: Write a diagnostic monitoring loop within your application. Require all active FreeRTOS threads to check in and register healthy execution flags before resetting the system watchdog timer. If your display thread hangs or the CAN bus stalls, the watchdog will trip and safely reboot your system.

### Step 13: Memory Management & Performance Tuning

* 13.1 Stack Diagnostics: Simulate a high-traffic environment and run the runtime inspection API (uxTaskGetStackHighWaterMark()) on all active threads.
* 13.2 Memory Allocation Tuning: Identify exactly how close each task comes to breaking its memory boundaries. Adjust your stack size variables inside your allocation statements to reclaim unused RAM bytes.

### Step 14: Network Filtering Validation & Verification

* 14.1 Hardware Filter Testing: Activate your external PC USB-to-CAN tool to inject thousands of random CAN frames with irrelevant identifiers onto the bus network.
* 14.2 Drop Verification: Verify that the STM32's hardware filter banks drop this unwanted background noise instantly at the hardware level, keeping your high-priority FreeRTOS processing task responsive and free of data backlogs.
* 14.3 Visual Synchronization Check: Confirm that your LCD display continues to render changing GPS positions and ambient metrics smoothly, ensuring the system remains responsive even under extreme network load.

