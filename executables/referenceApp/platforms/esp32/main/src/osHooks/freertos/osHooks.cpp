// Copyright 2024 Accenture.

#include <FreeRTOS.h>
#include <task.h>

extern "C"
{

void vApplicationStackOverflowHook(TaskHandle_t /* xTask */, char* /* pcTaskName */)
{
    // Stack overflow detected - enter infinite loop for debugging
    for (;;) {}
}

void vApplicationMallocFailedHook(void)
{
    // Malloc failed - enter infinite loop for debugging
    for (;;) {}
}

} // extern "C"
