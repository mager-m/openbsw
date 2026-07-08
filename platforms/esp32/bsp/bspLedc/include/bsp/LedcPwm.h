// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspLedc
 *
 * PWM driver for ESP32-S3 using LEDC peripheral.
 */
#pragma once

#include "bsp/Bsp.h"

#include <cstdint>

namespace bsp
{

class LedcPwm
{
public:
    struct ChannelConfig
    {
        uint8_t gpioNum;
        uint8_t channel;
        uint8_t timer;
        uint32_t frequency;
        uint8_t resolution;
    };

    static ::bsp::BspReturnCode init();
    static ::bsp::BspReturnCode setDuty(uint8_t channelIdx, uint32_t duty);
    static ::bsp::BspReturnCode getDuty(uint8_t channelIdx, uint32_t& duty);

    static ChannelConfig const fChannelConfigs[];
    static uint8_t const fNumChannels;

private:
    static bool sInitialized;
};

} // namespace bsp
