// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspConfiguration
 *
 * GPIO pin configuration for Arduino Nano ESP32.
 * Maps Arduino Nano ESP32 header pins to ESP32-S3 GPIO numbers.
 */
#pragma once

#include "bsp/Io.h"

namespace bios
{

/**
 * Arduino Nano ESP32 pin mapping:
 *   D0  = GPIO44 (RX)    D1  = GPIO43 (TX)
 *   D2  = GPIO5          D3  = GPIO6
 *   D4  = GPIO7          D5  = GPIO8
 *   D6  = GPIO9          D7  = GPIO10
 *   D8  = GPIO17         D9  = GPIO18
 *   D10 = GPIO21 (SS)    D11 = GPIO38 (MOSI)
 *   D12 = GPIO47 (MISO)  D13 = GPIO48 (SCK/LED)
 *   A0  = GPIO1          A1  = GPIO2
 *   A2  = GPIO3          A3  = GPIO4
 *   A4  = GPIO11 (SDA)   A5  = GPIO12 (SCL)
 *   A6  = GPIO13         A7  = GPIO14
 */
enum PinId : uint16_t
{
    D0 = 0U,
    D1,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    D9,
    D10,
    D11,
    D12,
    D13,
    A0,
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7,
    PIN_COUNT
};

inline Io::PinConfiguration const Io::fPinConfiguration[] = {
    {44, Io::_IN, 0, 0},     // D0 (RX)
    {43, Io::_OUT, 0, 0},    // D1 (TX)
    {5, Io::_DISABLED, 0, 0},  // D2
    {6, Io::_DISABLED, 0, 0},  // D3
    {7, Io::_DISABLED, 0, 0},  // D4
    {8, Io::_DISABLED, 0, 0},  // D5
    {9, Io::_DISABLED, 0, 0},  // D6
    {10, Io::_DISABLED, 0, 0}, // D7
    {17, Io::_DISABLED, 0, 0}, // D8
    {18, Io::_DISABLED, 0, 0}, // D9
    {21, Io::_DISABLED, 0, 0}, // D10 (SS)
    {38, Io::_DISABLED, 0, 0}, // D11 (MOSI)
    {47, Io::_DISABLED, 0, 0}, // D12 (MISO)
    {48, Io::_DISABLED, 0, 0}, // D13 (SCK) - GPIO48 is WS2812 RGB LED, not simple GPIO
    {1, Io::_IN, 0, 0},      // A0
    {2, Io::_IN, 0, 0},      // A1
    {3, Io::_IN, 0, 0},      // A2
    {4, Io::_IN, 0, 0},      // A3
    {11, Io::_IN, 0, 0},     // A4 (SDA)
    {12, Io::_IN, 0, 0},     // A5 (SCL)
    {13, Io::_IN, 0, 0},     // A6
    {14, Io::_IN, 0, 0},     // A7
};

inline uint16_t const Io::fPinConfigurationSize = PIN_COUNT;

} // namespace bios
