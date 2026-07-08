// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup freeRtosCoreConfiguration
 *
 * FreeRTOS platform configuration for ESP32-S3 (Arduino Nano ESP32).
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef INCLUDE_uxTaskPriorityGet
#define INCLUDE_uxTaskPriorityGet (1)
#endif

#undef configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW 2

#ifndef MINIMUM_STACK_SIZE
#define MINIMUM_STACK_SIZE (4096U)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
