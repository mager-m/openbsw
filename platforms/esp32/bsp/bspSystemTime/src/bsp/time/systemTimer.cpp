// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspSystemTime
 *
 * System time implementation for ESP32-S3 using esp_timer.
 */

#include <bsp/timer/SystemTimer.h>

#include "esp_timer.h"

#include <cstdint>

extern "C"
{

void initSystemTimer() {}

uint64_t getSystemTimeNs()
{
    return static_cast<uint64_t>(esp_timer_get_time()) * 1000ULL;
}

uint64_t getSystemTimeUs() { return static_cast<uint64_t>(esp_timer_get_time()); }

uint64_t getSystemTimeMs()
{
    return static_cast<uint64_t>(esp_timer_get_time() / 1000ULL);
}

uint32_t getSystemTimeUs32Bit()
{
    return static_cast<uint32_t>(esp_timer_get_time());
}

uint32_t getSystemTimeMs32Bit()
{
    return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
}

void sysDelayUs(uint32_t const delay)
{
    int64_t const start = esp_timer_get_time();
    while ((esp_timer_get_time() - start) < static_cast<int64_t>(delay))
    {
        // busy wait
    }
}

uint64_t getSystemTicks() { return getSystemTimeNs(); }

uint32_t getSystemTicks32Bit()
{
    return static_cast<uint32_t>(getSystemTimeNs());
}

uint64_t systemTicksToTimeNs(uint64_t ticks) { return ticks; }

uint64_t systemTicksToTimeUs(uint64_t ticks) { return ticks / 1000ULL; }

uint32_t getFastTicks() { return getSystemTimeUs32Bit(); }

uint32_t getFastTicksPerSecond() { return 1000000U; }

} // extern "C"
