// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspConfiguration
 *
 * ADC channel configuration for Arduino Nano ESP32.
 */
#pragma once

#include "bsp/Adc.h"

namespace bsp
{

/**
 * ADC channels on Arduino Nano ESP32:
 *   A0 = ADC1_CH0 (GPIO1)
 *   A1 = ADC1_CH1 (GPIO2)
 *   A2 = ADC1_CH2 (GPIO3)
 *   A3 = ADC1_CH3 (GPIO4)
 *   A4 = ADC2_CH0 (GPIO11)
 *   A5 = ADC2_CH1 (GPIO12)
 *   A6 = ADC2_CH2 (GPIO13)
 *   A7 = ADC2_CH3 (GPIO14)
 */
inline Adc::ChannelConfig const Adc::fChannelConfigs[] = {
    {0, 0, 3}, // A0: ADC1_CH0, 11dB attenuation
    {0, 1, 3}, // A1: ADC1_CH1
    {0, 2, 3}, // A2: ADC1_CH2
    {0, 3, 3}, // A3: ADC1_CH3
};

inline uint8_t const Adc::fNumChannels = 4U;

} // namespace bsp
