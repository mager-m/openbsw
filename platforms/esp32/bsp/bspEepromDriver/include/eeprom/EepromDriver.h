// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspEepromDriver
 *
 * EEPROM emulation driver for ESP32-S3 using NVS (Non-Volatile Storage).
 */
#pragma once

#include "bsp/eeprom/IEepromDriver.h"

#include <cstdint>

namespace eeprom
{

class EepromDriver : public IEepromDriver
{
public:
    static constexpr uint32_t EEPROM_SIZE = 4096U;

    EepromDriver();

    ::bsp::BspReturnCode init() override;
    ::bsp::BspReturnCode
    read(uint32_t address, uint8_t* buffer, uint32_t length) override;
    ::bsp::BspReturnCode
    write(uint32_t address, uint8_t const* buffer, uint32_t length) override;

private:
    bool _initialized;
    uint8_t _cache[EEPROM_SIZE];
};

} // namespace eeprom
