// Copyright 2024 Accenture.

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Performs a software system reset on the ESP32.
 */
[[noreturn]] void softwareSystemReset(void);

/**
 * Performs a destructive reset on the ESP32.
 */
void softwareDestructiveReset(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
