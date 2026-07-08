// Copyright 2024 Accenture.

#include "esp_system.h"

extern "C"
{

[[noreturn]] void softwareSystemReset(void) { esp_restart(); }

void softwareDestructiveReset(void) { softwareSystemReset(); }

} // extern "C"
