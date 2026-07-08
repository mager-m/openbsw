// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup safeBspMcuWatchdog
 *
 * Watchdog driver for ESP32-S3 using ESP-IDF task watchdog.
 */
#pragma once

#include <cstdint>

namespace safe
{

class SafeBspMcuWatchdog
{
public:
    static void init(uint32_t timeoutMs);
    static void feed();
    static void disable();
};

} // namespace safe
