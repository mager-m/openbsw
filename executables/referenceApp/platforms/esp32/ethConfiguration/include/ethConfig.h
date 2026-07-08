// Copyright 2024 Accenture.

#pragma once

#include "etl/array.h"

namespace ethX
{
// Ethernet not supported on Arduino Nano ESP32 (no Ethernet PHY).
// WiFi-based networking would require a separate implementation.
static constexpr size_t NUM_NETIFS = 0;
} // namespace ethX
