/*
 * Wrapper to include ESP-IDF's FreeRTOSConfig.h by absolute path,
 * avoiding recursion with our own FreeRTOSConfig.h in the parent dir.
 *
 * This is included by our FreeRTOSConfig.h to get the ESP-IDF struct
 * sizes and scheduler config correct.
 */
#pragma once

/* Prevent ESP-IDF's FreeRTOSConfig.h from being skipped by our #pragma once */
#include "sdkconfig.h"

/* ESP-IDF's FreeRTOS configuration — sets correct struct sizes for
 * dual-core ESP32-S3, TLS support, etc.
 *
 * We include by the config/xtensa path which has arch-specific settings.
 * The generic config at config/include/freertos/FreeRTOSConfig.h wraps
 * this plus sdkconfig.h. Since we already included sdkconfig.h, we go
 * directly to the arch config for the defines that matter. */

/* Include the generic ESP-IDF FreeRTOS config macros.
 * We use a relative path from the IDF component directory. The IDF_PATH
 * is set at compile time via -I flags. */

/* Core FreeRTOS config defines from ESP-IDF — extracted values.
 * These MUST match what ESP-IDF uses to size its internal structures. */

/* Number of cores (ESP32-S3 is dual-core) */
#ifndef configNUMBER_OF_CORES
#define configNUMBER_OF_CORES  CONFIG_FREERTOS_NUMBER_OF_CORES
#endif

#ifndef configNUM_CORES
#define configNUM_CORES  configNUMBER_OF_CORES
#endif

/* Thread Local Storage */
#if CONFIG_FREERTOS_ENABLE_STATIC_TASK_CLEAN_UP
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS (CONFIG_FREERTOS_THREAD_LOCAL_STORAGE_POINTERS * 2)
#else
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS CONFIG_FREERTOS_THREAD_LOCAL_STORAGE_POINTERS
#endif

/* Task notification array */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES CONFIG_FREERTOS_TASK_NOTIFICATION_ARRAY_ENTRIES

/* TLS / C runtime support — must match ESP-IDF for correct StaticTask_t size */
#define configUSE_C_RUNTIME_TLS_SUPPORT 1
#define configTLS_BLOCK_TYPE            void*

/* TLS block operations — stubs (OpenBSW uses dynamic task creation) */
#define configINIT_TLS_BLOCK(xTLSBlock, ...)  (void)(xTLSBlock)
#define configSET_TLS_BLOCK(xTLSBlock)        (void)(xTLSBlock)
#define configDEINIT_TLS_BLOCK(xTLSBlock)     (void)(xTLSBlock)

/* Trace facility */
#define configUSE_TRACE_FACILITY 1

/* Task name length */
#define configMAX_TASK_NAME_LEN CONFIG_FREERTOS_MAX_TASK_NAME_LEN

/* Static + dynamic allocation */
#define configSUPPORT_STATIC_ALLOCATION  1
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* Tick type */
#define configUSE_16_BIT_TICKS 0
#define configTICK_RATE_HZ     CONFIG_FREERTOS_HZ

/* Stack */
#define configMINIMAL_STACK_SIZE       (CONFIG_FREERTOS_IDLE_TASK_STACKSIZE / sizeof(StackType_t))
#define configRECORD_STACK_HIGH_ADDRESS 1

/* Preemption */
#define configUSE_PREEMPTION 1

/* Priorities */
#define configMAX_PRIORITIES 25

/* CO routines */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES 2

/* Application task tag */
#define configUSE_APPLICATION_TASK_TAG 0

/* Run time stats */
#define configGENERATE_RUN_TIME_STATS 0

/* Timers */
#define configUSE_TIMERS          1
#define configTIMER_TASK_PRIORITY 1
#define configTIMER_QUEUE_LENGTH  10
#define configTIMER_TASK_STACK_DEPTH (CONFIG_FREERTOS_TIMER_TASK_STACK_DEPTH / sizeof(StackType_t))

/* Mutexes */
#define configUSE_MUTEXES           1
#define configUSE_RECURSIVE_MUTEXES 1

/* Hooks */
#define configUSE_IDLE_HOOK         0
#define configUSE_TICK_HOOK         0
#define configCHECK_FOR_STACK_OVERFLOW 2

/* Misc */
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_TASK_NOTIFICATIONS  1
#define configUSE_POSIX_ERRNO         0
#define INCLUDE_xTaskAbortDelay       0
#define INCLUDE_vTaskSuspend          1
#define INCLUDE_vTaskDelay            1
#define INCLUDE_vTaskDelayUntil       1
#define INCLUDE_vTaskDelete           1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_xTaskGetIdleTaskHandle    1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_uxTaskPriorityGet     1
#define INCLUDE_xTimerPendFunctionCall 1

/* Critical nesting — Xtensa ESP32 uses portMUX, not nesting counter */
#define portCRITICAL_NESTING_IN_TCB 0

/* MPU */
#define portUSING_MPU_WRAPPERS 0
