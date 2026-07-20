---
title: "NEO-6M driver verification"
date: 7-3-2026
status: in-progress
version: 0.5.3
---

# Progress Summary

* BME280: Verified functioning properly with FreeRTOS default task post-update.
* NEO-6M Hardware: Wiring and hardware interrupts are functional.
* Current Bottleneck: Receiving bits but failing to parse proper NMEA sentences due to suspected baud rate timing issues.

------------------------------
# Hardware & Pin Configuration
## UART Settings

* Baud Rate: 9600 Bits/S
* Word Length: 8 bits (including parity)
* Parity: None
* Stop Bits: 1
* Interrupts: Global UART interrupts enabled

## STM32 Pin Assignment

* UART RX: PA10 (Connects to GPS TX)
* UART TX: PA9 (Connects to GPS RX)

------------------------------
# Known Issues & Bug Fixes
## Resolving UART Freeze
The UART interrupt entered an unresponsive state. Resolved by adding a de-initialization step before restarting the peripheral:

HAL_UART_DeInit(&huart1); // Clears weird interrupt states
HAL_UART_Init(&huart1);   // Re-initialize UART

## Signal Validation

* Symptoms: Incoming characters point to an incorrect baud rate mismatch.
* Action Required: Verify clock configurations to ensure the timing matches the 9600 baud rate exactly.

------------------------------
## Next Steps

   1. Fix Timing: Resolve the baud rate mismatch to get clean data strings.
   2. Apply GPS Driver Fix: Implement the validation bug fix in the code.
   3. Establish Satellite Lock: Move the GPS module near a window or outside to ensure it gets a solid fix.
   4. Final Integration: Complete this task last, as it requires the most complex hardware calibration.
