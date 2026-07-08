// Copyright 2024 Accenture.

#pragma once

#include "bsp/Bsp.h"

#include <gmock/gmock.h>

#include <cstdint>

namespace bios
{

class IoMock
{
public:
    MOCK_METHOD(::bsp::BspReturnCode, setDefaultConfiguration, (uint16_t io));
    MOCK_METHOD(::bsp::BspReturnCode, getPin, (uint16_t io, bool& level));
    MOCK_METHOD(::bsp::BspReturnCode, setPin, (uint16_t io, bool level));

    static IoMock& instance();
};

} // namespace bios
