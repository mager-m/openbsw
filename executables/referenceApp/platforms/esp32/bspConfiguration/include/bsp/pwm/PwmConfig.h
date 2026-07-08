// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspConfiguration
 *
 * PWM (LEDC) channel configuration for Arduino Nano ESP32.
 */
#pragma once

#include "bsp/LedcPwm.h"

namespace bsp
{

/**
 * LEDC PWM channel configuration.
 * Default: D2 (GPIO5) as PWM output at 5 kHz, 13-bit resolution.
 * Note: GPIO48 (D13) is the WS2812 RGB LED and cannot be used with plain PWM.
 */
inline LedcPwm::ChannelConfig const LedcPwm::fChannelConfigs[] = {
    {5, 0, 0, 5000, 13}, // D2 (GPIO5): channel 0, timer 0, 5kHz, 13-bit
};

inline uint8_t const LedcPwm::fNumChannels = 1U;

} // namespace bsp
