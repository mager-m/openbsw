/*
 * ESP-IDF glue for OpenBSW.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "openbsw";

extern void asyncInitialized(void);

/* OpenBSW's idle hook — runs console + logger processing.
 * Normally called by FreeRTOS idle task via vApplicationIdleHook,
 * but we localized that symbol. Call it from our idle loop instead. */
extern void vApplicationIdleHook(void);

/* The real vTaskStartScheduler provided by --wrap */
extern void __real_vTaskStartScheduler(void);

/* Track whether the scheduler has been started by ESP-IDF */
static int s_scheduler_started = 0;

/*
 * vTaskStartScheduler wrapper.
 *
 * Called TWICE:
 * 1. By ESP-IDF during system boot (esp_startup_start_app) — pass through
 * 2. By OpenBSW's FreeRtosAdapter::run() — skip, call asyncInitialized
 */
void __wrap_vTaskStartScheduler(void)
{
    if (!s_scheduler_started)
    {
        /* First call — ESP-IDF boot. Start the real scheduler. */
        s_scheduler_started = 1;
        __real_vTaskStartScheduler();
        /* Never reaches here — scheduler takes over */
    }
    else
    {
        /* Second call — from OpenBSW. Scheduler already running. */
        ESP_LOGI(TAG, "Scheduler already running — calling asyncInitialized()");
        asyncInitialized();

        ESP_LOGI(TAG, "OpenBSW started. Running idle handler.");

        /* Drop to lowest priority so ESP-IDF's IDLE task can run
         * (feeds the task watchdog). */
        vTaskPrioritySet(NULL, 0);

        for (;;)
        {
            /* Run OpenBSW's idle processing (console, logger, etc.) */
            vApplicationIdleHook();
            /* Yield to let IDLE task feed the watchdog */
            taskYIELD();
        }
    }
}

TaskHandle_t __wrap_xTaskGetIdleTaskHandle(void)
{
    return xTaskGetCurrentTaskHandle();
}

/*
 * xTaskCreateStaticPinnedToCore wrapper — use dynamic allocation.
 *
 * OpenBSW's Task<> template allocates stack as a static array in .bss,
 * but ESP-IDF's xPortcheckValidStackMem() rejects this memory region.
 * We use xTaskCreatePinnedToCore (dynamic) instead, ignoring the
 * static stack/TCB buffers.
 */
TaskHandle_t __wrap_xTaskCreateStaticPinnedToCore(
    TaskFunction_t pxTaskCode,
    const char * const pcName,
    const uint32_t ulStackDepth,
    void * const pvParameters,
    UBaseType_t uxPriority,
    StackType_t * const puxStackBuffer,
    StaticTask_t * const pxTaskBuffer,
    const BaseType_t xCoreID)
{
    (void)puxStackBuffer;
    (void)pxTaskBuffer;

    TaskHandle_t handle = NULL;
    BaseType_t ret = xTaskCreatePinnedToCore(
        pxTaskCode, pcName, ulStackDepth * sizeof(StackType_t),
        pvParameters, uxPriority, &handle, xCoreID);

    if (ret != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create task: %s (stack=%lu)", pcName,
                 (unsigned long)(ulStackDepth * sizeof(StackType_t)));
        return NULL;
    }
    ESP_LOGI(TAG, "Created task: %s (prio=%lu, stack=%lu)",
             pcName, (unsigned long)uxPriority,
             (unsigned long)(ulStackDepth * sizeof(StackType_t)));
    return handle;
}
