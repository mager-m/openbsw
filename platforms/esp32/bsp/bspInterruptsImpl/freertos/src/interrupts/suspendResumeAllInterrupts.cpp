// Copyright 2024 Accenture.

#include "interrupts/suspendResumeAllInterrupts.h"

#include <FreeRTOS.h>
#include <task.h>

static portMUX_TYPE sMux = portMUX_INITIALIZER_UNLOCKED;

void main_thread_setup(void)
{
    // No special setup needed on ESP32 - FreeRTOS is initialized by ESP-IDF
}

OldIntEnabledStatusValueType getOldIntEnabledStatusValueAndSuspendAllInterrupts(void)
{
    taskENTER_CRITICAL(&sMux);
    return static_cast<OldIntEnabledStatusValueType>(1);
}

void resumeAllInterrupts(OldIntEnabledStatusValueType const oldIntEnabledStatusValue)
{
    if (oldIntEnabledStatusValue != 0)
    {
        taskEXIT_CRITICAL(&sMux);
    }
}
