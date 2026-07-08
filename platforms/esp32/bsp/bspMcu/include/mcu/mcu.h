// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspMcu
 *
 * MCU header for ESP32-S3 (Arduino Nano ESP32).
 */
#pragma once

#include <cstdint>

// Interrupt control macros using FreeRTOS critical sections on ESP32
#ifdef REALTIME_OS
#include <FreeRTOS.h>
#include <task.h>
#define ENABLE_INTERRUPTS()  portENABLE_INTERRUPTS()
#define DISABLE_INTERRUPTS() portDISABLE_INTERRUPTS()
#else
#define ENABLE_INTERRUPTS()
#define DISABLE_INTERRUPTS()
#endif

// ESP32-S3 specific definitions
#define ESP32_FLASH_BASE      0x3C000000U
#define ESP32_SRAM_BASE       0x3FC88000U
#define ESP32_PERIPHERAL_BASE 0x60000000U

// ESP32-S3 uses Xtensa interrupt architecture (not ARM NVIC).
// Interrupt priority levels are managed by FreeRTOS port layer.
