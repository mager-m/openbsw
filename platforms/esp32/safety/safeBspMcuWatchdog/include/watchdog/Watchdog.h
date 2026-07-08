// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace safety
{
namespace bsp
{

/**
 * ESP32-S3 watchdog abstraction using ESP-IDF Task Watchdog Timer.
 * Provides the interface expected by SafeWatchdog.
 */
class Watchdog
{
public:
    Watchdog() { disableWatchdog(); }

    explicit Watchdog(uint32_t const timeout, uint32_t const = 0U)
    {
        enableWatchdog(timeout);
    }

    static void enableWatchdog(
        uint32_t timeout, bool interruptActive = false, uint32_t clockSpeed = 0U);
    static void disableWatchdog();
    static void serviceWatchdog();
    static bool checkWatchdogConfiguration(uint32_t timeout, uint32_t clockSpeed = 0U);
    static uint32_t getWatchdogServiceCounter();

    static uint32_t const DEFAULT_TIMEOUT = 500U;

private:
    static uint32_t watchdogServiceCounter;
};

} // namespace bsp
} // namespace safety
