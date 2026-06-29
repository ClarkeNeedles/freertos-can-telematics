---
title: "Embedded coding guidelines"
date: 6-29-2026
status: in-progress
version: 0.1.1
---

- These are general coding guidelines that should apply to all drivers coded in this repository. 
- These guidelines can also apply to other projects as well.
- They are good general rules to follow for all embedded software and firmware applications.

# 📂 1. File Structure & Architecture
## 1.1 Header File (.h) Layout
Every header file must follow a strict modular structure:

   1. Top Banner: File name, brief description, and author attribution.
   2. Include Guards: Standard `#ifndef MODULE_H` block.
   3. C++ Compiling Guards: `#ifdef __cplusplus` block to prevent name mangling.
   4. Hardware Definitions & Register Maps: Grouped using macro groups with inline Doxygen syntax.
   5. Type Definitions: Structs and custom Enums.
   6. Public API: Function declarations paired with rich Doxygen blocks.

## 1.2 Source File (.c) Layout
Source files must isolate user-facing code from low-level register manipulation:

   1. Private Prototypes: All local static forward declarations must reside at the very top of the file. This satisfies one-pass compilers and eliminates sequence-dependent errors.
   2. Public API: Implement global user-facing functions directly beneath the prototypes. Separator lines (// ---) must separate adjacent functions instead of noisy block banners.
   3. Private Implementations: Place local helper operations at the very bottom of the file, preceded by a concise, high-density Doxygen documentation block.

------------------------------
# 🏷️ 2. Naming Conventions & Scope
## 2.1 Prefix Namespaces
To avoid global identifier namespace collisions, every file macro, definition, type declaration, and public API function must be prefixed with the specific driver string name.

* Correct: `BME280_Init()`, `ILI9341_REG_SWRESET`, `BME280_Handle_t`
* Incorrect: `InitBME280()`, `REG_SWRESET`, `DisplayHandle`

## 2.2 Object Scoping Enforcements

* Public Interfaces: Exposed in the header file and globally linkable.
* Internal Helpers: Functions used strictly within a single .c file must be declared static and kept hidden from the global linker table.
* Constants: Never use raw numbers ("magic numbers") inside active application routines. Convert values into explicit `#define` macros or strict enums.

## 2.3 Variable Typing

* Always implement explicit-width types from `<stdint.h>` (uint8_t, int32_t) when mapping variables to hardware components or data packets to guarantee cross-platform memory alignment.

------------------------------
# 📐 3. Formatting, Braces & Page Geometry
## 3.1 Strict Brace Placement (Allman Style)
Braces {} are mandatory for all control flow blocks (if, else, for, while, switch), even if the block contains only a single line of code. This completely eliminates accidental parsing bugs when adding lines during future refactors. 
The opening brace must always begin on a new line directly underneath the controlling statement, aligned at the same indentation level.

* Correct:
```
if (status != HAL_OK)
{
    return status;
}
```

* Incorrect:
```
if (status != HAL_OK) return status; // Missing braces and new line
if (status != HAL_OK) {              // Brace on same line
    return status;
}
```

## 3.2 Code Density & Printable Line Limits
To maintain high readability on standard desktop monitors and ensure code prints cleanly on a physical page or shifts perfectly into standard code-review windows, enforce an 80-character maximum line limit.
If a statement or function call exceeds 80 characters, wrap it cleanly onto the next line using a 4-space hanging indentation.

* Correct (Wrapped Parameter Lists):
```
status = HAL_I2C_Mem_Write(
    dev->hi2c, 
    dev->address, 
    BME280_REG_CTRL_HUM, 
    I2C_MEMADD_SIZE_8BIT, 
    &cfg->osrs_h, 
    1, 
    HAL_MAX_DELAY
);
```
* Correct (Wrapped Conditional Logic):
```
if (BME280_CheckID(dev) != HAL_OK ||
    BME280_Reset(dev) != HAL_OK)
{
    return HAL_ERROR; 
}
```

------------------------------
# 💬 4. Commenting Strategies & Documentation
## 4.1 Documenting Tricky or Complex Logic
Inline comments must explain why something is done, not what is done. Assume the person reading the code already understands standard C syntax.

* `Bad: count++; // Increment count by one`
* `Good: count++; // Accounts for the mandatory start-frame byte delimiter`

For multi-step mathematical calculations (like the Bosch BME280 compensation formulas) or hardware quirks, place a short descriptive text block directly above the code to map the implementation to its physical or mathematical theory:
```
// Step 1: Calculate the intermediate fine-temperature variable (t_fine).
// This value anchors the sensor's thermal baseline and is a hard mathematical
// prerequisite for running subsequent pressure and humidity calculations.
var1 = (((adc_T / 16384.0) - (dev->calib.dig_T1 / 1024.0)) * dev->calib.dig_T2);
```
## 4.2 Tiered Doxygen Architecture

* Public Interface Functions: Place full Javadoc-style blocks (`/** ... */`) directly inside the header file above the function prototype. This exposes clear tooltips across the IDE environment.
* Private Helpers: Place condensed, high-density blocks directly inside the source file right above the active function definition body.
* Parameter Explicit Tags: Mandate data flow direction attributes via `@param[in]`, `@param[out]`, or `@param[in,out]` identifiers. Bind hardware responses to specific code descriptors using the `@retval` keyword.

## 4.3 Code Block End-Markers

* No Redundant Braces: Do not add comments explaining the closing brace of standard blocks or routines (`} /* BME280_Init */` is prohibited).
* Preprocessor Exceptions: Appending short trailing tracking markers to deep or non-contiguous preprocessor directives is encouraged to maintain logic transparency (`#endif /* ILI9341_H */`).

------------------------------
## 🛡️ 5. Defensive Programming & Safety
## 5.1 Interface Isolation

* Never declare public variable configurations globally. Always encapsulate runtime parameters inside localized device handle structures (`BME280_Handle_t *dev`) and pass them into functions via pointers.

## 5.2 Parameter Validation & Clamping

* Pointer Guards: Public driver routines must validate incoming device context handle pointers before executing pointer operations.
* Algorithmic Safety Gates: Insert safety checks to handle zero-value parameters or potential division-by-zero errors explicitly before processing calculations to prevent runtime exceptions.
* Physical Boundary Clamping: Math pipelines converting raw sensor data to physical units must apply hard ceiling and floor boundaries to clamp theoretical anomalies within normal physical boundaries (e.g., restricting humidity measurements strictly between 0.0% and 100.0%).

## 5.3 Transaction Optimization

* Burst Transfers: Group consecutive peripheral data register fetches into single-transaction multi-byte burst blocks rather than sequential single-byte commands. This ensures atomic data snapshot frames and reduces bus locking overhead.