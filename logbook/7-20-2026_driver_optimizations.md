---
title: "Driver optimization"
date: 7-20-2026
status: in-progress
version: 0.5.4
---

# STM32F4 Sensor Driver Optimization Guide

This guide outlines code-level, memory, and peripheral optimization strategies for STM32F4 driver development (specifically targeting sensors like the BME280, NEO-6M GPS, and ILI LCD displays). The goal is to maximize execution speed (clock cycles) and minimize memory footprint (Flash and SRAM).

---

## 1. Code-Level Optimizations (CPU Cycles & Flash Space)

### Avoid Redundant Boolean Comparisons
* **Rule**: Remove explicit `== TRUE` or `== 1` checks. 
* **Impact**: Saves Flash space and prevents unnecessary assembly instructions.
* **Example**:
  ```c
  // Bad
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) { ... }
  // Good
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)) { ... }
  ```

### Use Ternary Operators for Branchless Code
* **Rule**: Use `(condition) ? a : b` for simple conditional assignments.
* **Impact**: Allows the ARM Cortex-M4 compiler to generate branchless conditional instructions (like `IT` blocks), avoiding pipeline flushes.
* **Example**:
  ```c
  // Optimizes directly to a conditional move instruction
  uint8_t mode = (oversampling > 4) ? BME280_HIGH_POWER : BME280_LOW_POWER;
  ```

### Pass Big Data by Pointer, Small Data by Value
* **Rule**: Pass structures (like calibration data blocks or frame buffers) via pointers. Pass basic data types (like registers or 8/16-bit IDs) by value.
* **Impact**: Eliminates expensive stack-copying operations for arrays/structs, while avoiding overhead pointer-dereferencing for tiny basic types.
* **Example**:
  ```c
  // Good: Struct passed by pointer to save stack space
  void BME280_Read_Calibration(BME280_CalibData *calib); 
  
  // Good: Passed by value because a 16-bit address fits directly into a CPU register
  void ILI9341_WriteCommand(uint16_t cmd); 
  ```

---

## 2. Memory & Structure Layout (SRAM Efficiency)

### Struct Alignment and Padding Optimization
* **Rule**: Arrange members of structures in decreasing order of their size (32-bit `uint32_t`/pointers first, then 16-bit `uint16_t`, then 8-bit `uint8_t`).
* **Impact**: Prevents the compiler from adding silent "padding bytes" to satisfy the ARM 32-bit alignment rules, heavily reducing struct sizes in SRAM.
* **Example**:
  ```c
  // Bad (Saves 12 bytes but occupies 16 bytes due to padding)
  struct BadLayout {
      uint8_t sensor_id;    // 1 byte (+ 3 bytes padding)
      uint32_t pressure;    // 4 bytes
      uint16_t humidity;    // 2 bytes (+ 2 bytes padding)
  };

  // Good (Occupies exactly 8 bytes)
  struct GoodLayout {
      uint32_t pressure;    // 4 bytes
      uint16_t humidity;    // 2 bytes
      uint8_t sensor_id;    // 1 byte
                            // (+ 1 byte padding at the very end)
  };
  ```

### Group State Flags into Bitfields
* **Rule**: Condense multiple boolean status flags into a single byte using standard bitfields.
* **Impact**: Drastically minimizes SRAM usage for sensor state management.
* **Example**:
  ```c
  typedef struct {
      uint8_t bme_ready    : 1;
      uint8_t gps_has_fix  : 1;
      uint8_t lcd_refresh  : 1;
      uint8_t reserved     : 5;
  } SystemStatus; // Total size: exactly 1 byte
  ```

---

## 3. Peripheral-Specific Optimizations (BME280, NEO-6M, ILI LCD)

### ILI LCD: Bulk Writes & DMA (Direct Memory Access)
* **Rule**: Never use single-pixel blocking functions (`ILI9341_DrawPixel`) to refresh screen regions. Fill a local RAM array buffer and push it out via **SPI + DMA**.
* **Impact**: Offloads data transmission completely from the Cortex-M4 CPU, letting the CPU calculate the next frame or process sensor readings while the screen draws.

### NEO-6M GPS: Non-Blocking NMEA Parsing
* **Rule**: Do not wait or poll inside an unbuffered UART function for GPS data. Use a **UART Circular Buffer via DMA** paired with the **UART Idle Line Interrupt**.
* **Impact**: The driver is only woken up when a complete NMEA sentence (`$GPRMC...`) has fully arrived in memory, preventing the CPU from stalling during slow 9600-baud UART streams.

### BME280: Fixed-Point Arithmetic over Floating-Point
* **Rule**: Use the fixed-point (integer-based) compensation formulas provided in the Bosch BME280 datasheet instead of double/float math.
* **Impact**: Although the STM32F4 has a hardware Floating Point Unit (FPU), integer/fixed-point calculations execute significantly faster and don't pull in heavy floating-point library overhead into your Flash binary.

---

## 4. Compiler & Toolchain Knobs

* **Optimization Flag `-Os`**: Instructs GCC to optimize specifically for size. Use this if your code is bumping against the STM32F4 Flash memory ceiling.
* **Optimization Flag `-O2` / `-O3`**: Instructs GCC to maximize execution speed (useful for intensive math or manual LCD frame pushing).
* **Enable LTO (`-flto`)**: Turn on Link-Time Optimization. It allows the compiler to inline functions across different `.c` source files, eliminating function-call overhead for driver helper abstractions.

updated the BME280_Handle_t packing
updated the BME280_Config_t so that it uses bit packing instead
	we can pass by value instead because it is smaller
consider switching BME280_Data_t to use integers instead of floats in the future
	getting rid of floats entirely removes the massive float binary from flash RAM
	not yet implemented, but something to consider if it causes issues in the future