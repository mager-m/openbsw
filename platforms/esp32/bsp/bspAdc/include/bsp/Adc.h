// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspAdc
 *
 * ADC driver for ESP32-S3 using ESP-IDF ADC oneshot driver.
 */
#pragma once

#include "bsp/Bsp.h"

#include <cstdint>

namespace bsp
{

class Adc
{
public:
    static constexpr uint8_t MAX_CHANNELS = 10U;

    struct ChannelConfig
    {
        uint8_t unit;
        uint8_t channel;
        uint8_t attenuation;
    };

    static ::bsp::BspReturnCode init();
    static ::bsp::BspReturnCode read(uint8_t channelIdx, uint16_t& value);

    static ChannelConfig const fChannelConfigs[];
    static uint8_t const fNumChannels;

private:
    static bool sInitialized;
};

} // namespace bsp
