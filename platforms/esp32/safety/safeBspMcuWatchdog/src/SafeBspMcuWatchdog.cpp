// Copyright 2024 Accenture.

#include "safe/SafeBspMcuWatchdog.h"

#include "esp_task_wdt.h"

namespace safe
{

void SafeBspMcuWatchdog::init(uint32_t timeoutMs)
{
    esp_task_wdt_config_t config = {};
    config.timeout_ms            = timeoutMs;
    config.idle_core_mask        = 0x03U; // Watch both cores
    config.trigger_panic         = true;
    esp_task_wdt_reconfigure(&config);
    esp_task_wdt_add(nullptr);
}

void SafeBspMcuWatchdog::feed() { esp_task_wdt_reset(); }

void SafeBspMcuWatchdog::disable()
{
    esp_task_wdt_delete(nullptr);
    esp_task_wdt_deinit();
}

} // namespace safe
