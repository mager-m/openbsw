// Copyright 2024 Accenture.

#include "watchdog/Watchdog.h"

#include "esp_task_wdt.h"

namespace safety
{
namespace bsp
{

uint32_t Watchdog::watchdogServiceCounter = 0U;

void Watchdog::enableWatchdog(uint32_t timeout, bool, uint32_t)
{
    esp_task_wdt_config_t config = {};
    config.timeout_ms            = timeout;
    config.idle_core_mask        = 0x03U;
    config.trigger_panic         = true;
    esp_task_wdt_reconfigure(&config);
    esp_task_wdt_add(nullptr);
}

void Watchdog::disableWatchdog()
{
    esp_task_wdt_delete(nullptr);
}

void Watchdog::serviceWatchdog()
{
    esp_task_wdt_reset();
    ++watchdogServiceCounter;
}

bool Watchdog::checkWatchdogConfiguration(uint32_t, uint32_t)
{
    // ESP-IDF manages watchdog configuration; always report OK
    return true;
}

uint32_t Watchdog::getWatchdogServiceCounter() { return watchdogServiceCounter; }

} // namespace bsp
} // namespace safety
