// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspGpio
 *
 * GPIO driver for ESP32-S3 using ESP-IDF GPIO driver.
 */
#pragma once

#include "bsp/Bsp.h"

#include <cstdint>

namespace bios
{

class Io
{
public:
    using PinId = uint16_t;

    enum Direction
    {
        _DISABLED = 0,
        _IN       = (1 << 0),
        _OUT      = (1 << 1),
        _IN_OUT   = _IN | _OUT
    };

    struct PinConfiguration
    {
        uint8_t gpioNum;
        uint8_t dir;
        uint8_t pullUp;
        uint8_t pullDown;
    };

    static ::bsp::BspReturnCode setDefaultConfiguration(PinId io);
    static ::bsp::BspReturnCode setConfiguration(PinId io, PinConfiguration const& cfg);
    static ::bsp::BspReturnCode getPin(PinId io, bool& level);
    static ::bsp::BspReturnCode setPin(PinId io, bool level);

    static PinConfiguration const fPinConfiguration[];
    static uint16_t const fPinConfigurationSize;
};

} // namespace bios
