---
title: "BME280 Driver + Testing"
date: 6-20-2026
status: in-progress
version: 0.1.0
---

## Project Setup
- Configured the STM32 project folder inside the GitHub repository
- Used an older “playground” project that already contained working pin configurations
- Recovered an older document containing sensor setup notes and wiring information

---

## Resources

- BME280 STM32 Guide:  
  https://controllerstech.com/interface-bme280-with-stm32/

- BME280 Datasheet (Bosch):  
  https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-stmicroelectronics.pdf

- STM32 Nucleo Board Manual:  
  https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-stmicroelectronics.pdf

---

# BME280 Sensor Setup (I2C Driver)

## 1. Project Configuration
- Create STM32 project in STM32CubeIDE
- Enable **I2C peripheral** in `.ioc` file
- CubeIDE generates:
  - SDA (data line)
  - SCL (clock line)

---

## 2. Sensor Address Configuration

- BME280 I2C address depends on the **SDO pin**
- Address format:
```

111011X0

```
where `X = SDO pin state`

### Configuration used:
- SDO grounded → `X = 0`
- Final address:
```
0xEC
```

---

## 3. Wiring (I2C Mode)

| Sensor Pin | STM32 Connection |
|------------|----------------|
| VCC        | 3.3V |
| GND        | GND |
| SCL        | PB6 |
| SDA        | PB7 |
| CSB        | 3.3V (HIGH for I2C mode) |
| SDO        | GND |

⚠️ Important:
- BME280 is **3.3V only (do not use 5V)**

---

## 4. Driver Development
- Implemented full BME280 driver in C
- Includes:
- Register-level I2C read/write
- Calibration data parsing
- Temperature/pressure/humidity compensation
- High-level API (`ReadAll`, etc.)
- Rewrote driver to be:
- More generic
- Handle-based (supports multiple sensors)
- Cleaner separation of public vs internal functions

---

## 5. Main Application Flow

- Initialize sensor in `main()`
- Read data in `while(1)` loop
- Use temperature threshold to control LED

### Example behavior:
- Read temperature continuously
- If temperature exceeds threshold:
- Turn ON LED
- Otherwise:
- Turn OFF LED

---

## 6. Debugging
- Add watch expressions in STM32CubeIDE
- Observe real-time temperature changes
- Validate by touching sensor (temperature increases)

---

# Final Status

- ✔ Rewritten BME280 driver (generic + handle-based design)
- ✔ Improved coding structure and modularity
- ✔ Sensor successfully reading temperature data
- ✔ System validated on STM32 hardware

---
