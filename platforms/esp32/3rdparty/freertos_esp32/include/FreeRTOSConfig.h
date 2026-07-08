// Copyright 2024 Accenture.

/**
 * ESP32-specific FreeRTOSConfig.h
 *
 * This provides the FreeRTOS configuration for ESP-IDF ABI compatibility.
 * The ASYNC framework macros (ASYNC_CONFIG_TASK_COUNT, etc.) are added
 * by the freeRtosConfiguration target's include path, which includes
 * asyncFreeRtos/freeRtosConfiguration/ AFTER this file.
 */
#pragma once

#include "esp_idf_freertos/FreeRTOSConfig_idf.h"

/* Trace macro stubs — ESP-IDF's portmacro.h references these */
#ifndef traceISR_EXIT_TO_SCHEDULER
#define traceISR_EXIT_TO_SCHEDULER()
#endif

#ifndef traceISR_EXIT
#define traceISR_EXIT()
#endif

#ifndef traceISR_ENTER
#define traceISR_ENTER(_n)
#endif

/* --- OpenBSW async framework macros ---
 * These are normally set by asyncFreeRtos/freeRtosConfiguration/FreeRTOSConfig.h
 * but that file is shadowed by this one. Provide defaults here; the actual
 * async/Config.h values are used when the asyncFreeRtos target is linked. */
#ifndef ASYNC_CONFIG_NESTED_INTERRUPTS
#define ASYNC_CONFIG_NESTED_INTERRUPTS (1)
#endif

#ifndef ASYNC_CONFIG_TASK_CONFIG
#define ASYNC_CONFIG_TASK_CONFIG (0)
#endif

#ifndef ASYNC_CONFIG_TICK_HOOK
#define ASYNC_CONFIG_TICK_HOOK (0)
#endif

#ifndef ASYNC_TASK_CONFIG_TYPE
#define ASYNC_TASK_CONFIG_TYPE void
#endif

/* Include async/Config.h if available (defines ASYNC_CONFIG_TASK_COUNT etc.)
 * and provide async::Config struct for C++ consumers. This is normally done
 * by asyncFreeRtos/freeRtosConfiguration/FreeRTOSConfig.h which we shadow. */
#if __has_include("async/Config.h")
#include "async/Config.h"
#include "async/Hook.h"

/* Override configMAX_PRIORITIES with async task count */
#undef configMAX_PRIORITIES
#define configMAX_PRIORITIES (ASYNC_CONFIG_TASK_COUNT + 1)

#undef configTICK_RATE_HZ
#define configTICK_RATE_HZ (1000000U / ASYNC_CONFIG_TICK_IN_US)

#define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H (1)
#define FREERTOS_TASKS_C_ADDITIONS_INIT           asyncInitialized

#ifdef __cplusplus
extern "C"
{
#endif
void const* asyncGetTaskConfig(size_t taskIdx);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace async
{
struct Config
{
    static size_t const TASK_COUNT = static_cast<size_t>(ASYNC_CONFIG_TASK_COUNT);
    static size_t const TICK_IN_US = static_cast<size_t>(ASYNC_CONFIG_TICK_IN_US);
};
} // namespace async
#endif

#include "os/FreeRtosPlatformConfig.h"

#endif /* __has_include("async/Config.h") */
