---
title: "CAN bus driver development"
date: 7-1-2026
status: in-progress
version: 0.5.3
---

# Firmware Project Status & Architecture Notes

## 🚀 Accomplishments & Milestones
* **RTOS Integration**: Configured FreeRTOS via the `.ioc` device configuration file and generated the foundational kernel code.
* **Task Architecture**: Established a solid understanding of how tasks are instantiated and generated within the STM32CubeIDE ecosystem.
* **Driver Development**: All target device drivers are complete, fully documented using consistent Doxygen blocks, and formatting to a strict 2-space indentation style.
* **Compilation Status**: The entire codebase compiles and builds cleanly without warnings or static analyzer errors.

## 🛠️ Architectural Design Decisions

### 1. Pointer Validation & Efficiency
* **Decision**: Purged defensive `NULL` pointer checks from internal driver logic hot-paths.
* **Rationale**: Pointer validation is an application-layer responsibility. Removing these runtime conditional checks eliminates CPU branch overhead and guarantees a highly visible `HardFault` crash during testing if an uninitialized device handle is passed.

### 2. Custom Status Enums
* **Decision**: Implemented isolated `_Status_t` custom enums across all drivers (`can_bus`, `bme280`, `ili9341`, `neo6m`) rather than forwarding generic `HAL_StatusTypeDef` identifiers.
* **Rationale**: 
  * Decentralizes the application layer from vendor-specific HAL return types.
  * Provides granular error diagnostic information (e.g., distinguishing between a normal empty buffer and a physical hardware crash).
  * Promotes highly readable, self-documenting application logic blocks.

---

## 🗃️ Driver Status Matrix

| Driver | File Name | Context Control | Error Mapping Status |
| :--- | :--- | :--- | :--- |
| **CAN Network Wrapper** | `can_bus.c` / `.h` | Transceiver `RS` Pin / Filters | Complete (`CAN_Bus_Status_t`) |
| **BME280 Telemetry** | `bme280.c` / `.h` | I2C Calibration & Math | Complete (`BME280_Status_t`) |
| **ILI9341 Screen** | `ili9341.c` / `.h` | SPI Burst Array Buffering | Complete (`ILI9341_Status_t`) |
| **NEO-6M GPS** | `neo6m.c` / `.h` | Non-Destructive UART Parsing | Complete (`NEO6M_Status_t`) |

---

## 📝 Hardware Procurement & Lab Checklists

### Parts & Consumables to Evaluate
* [ ] **Jumper Wires**: Verify total quantity for final device physical wiring. Buy additional wire kits if lengths run short.
* [ ] **Breadboards**: Audit current bench real-estate. One board should suffice, but source a secondary breadboard if bus lines become over-congested.
* [ ] **Screw Terminals**: Prepare bare wire leads to mate into the CAN transceiver board screw attachments.

### Lab Tasks (Post-Work Integration)
* [ ] Check the ILI9341 display packaging for included raw male pin headers.
* [ ] If pin headers are missing from the display shipment, acquire standard 2.54mm pitch breakable header rows.
* [ ] Utilizing the company electronics lab after hours, solder the header pins directly onto the ILI9341 display module PCB.

---

## 📅 Next Steps & Execution Strategy

### Phase 1: Hardware-in-the-Loop Isolation Testing
* Deploy firmware tests targeting one hardware peripheral at a time.
* Isolate all execution code inside the pre-existing FreeRTOS **Default Task** (`StartDefaultTask`) loop.
* *Note: This validates driver behavior inside an active FreeRTOS scheduling engine without managing inter-task data dependencies.*

### Phase 2: Full Task Multi-Threading
* Once standalone hardware stability is confirmed across all modules, proceed with building out the final architecture.
* Generate and implement the unique operational tasks and queues discussed during architectural design phases.
