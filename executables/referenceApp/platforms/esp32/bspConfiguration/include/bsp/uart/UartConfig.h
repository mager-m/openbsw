// Copyright 2024 Accenture.

#pragma once

#include "bsp/Uart.h"

#include <cstddef>
#include <cstdint>

namespace bsp
{

/**
 * UART IDs for Arduino Nano ESP32.
 * UART0 is a hardware UART on GPIO43(TX)/GPIO44(RX) header pins.
 * Note: USB console uses native USB-OTG on GPIO19/GPIO20, not UART0.
 */
enum class Uart::Id : size_t
{
    TERMINAL,
    INVALID,
};

static constexpr size_t NUMBER_OF_UARTS = static_cast<size_t>(Uart::Id::INVALID);

} // namespace bsp
