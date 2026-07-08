// Copyright 2024 Accenture.

#include "bsp/Adc.h"

#include "esp_adc/adc_oneshot.h"

namespace bsp
{

bool Adc::sInitialized = false;

static adc_oneshot_unit_handle_t sAdcHandles[2] = {nullptr, nullptr};

::bsp::BspReturnCode Adc::init()
{
    if (sInitialized)
    {
        return ::bsp::BSP_OK;
    }

    // Determine which ADC units are needed
    bool needUnit[2] = {false, false};
    for (uint8_t i = 0U; i < fNumChannels; ++i)
    {
        uint8_t const unit = fChannelConfigs[i].unit;
        if (unit < 2U)
        {
            needUnit[unit] = true;
        }
    }

    // Initialize required ADC units
    for (uint8_t u = 0U; u < 2U; ++u)
    {
        if (!needUnit[u])
        {
            continue;
        }
        adc_oneshot_unit_init_cfg_t initCfg = {};
        initCfg.unit_id = static_cast<adc_unit_t>(u);

        if (adc_oneshot_new_unit(&initCfg, &sAdcHandles[u]) != ESP_OK)
        {
            return ::bsp::BSP_ERROR;
        }
    }

    // Configure channels on their respective units
    for (uint8_t i = 0U; i < fNumChannels; ++i)
    {
        uint8_t const unit = fChannelConfigs[i].unit;
        adc_oneshot_chan_cfg_t chanCfg = {};
        chanCfg.atten    = static_cast<adc_atten_t>(fChannelConfigs[i].attenuation);
        chanCfg.bitwidth = ADC_BITWIDTH_DEFAULT;

        if (adc_oneshot_config_channel(
                sAdcHandles[unit],
                static_cast<adc_channel_t>(fChannelConfigs[i].channel),
                &chanCfg)
            != ESP_OK)
        {
            return ::bsp::BSP_ERROR;
        }
    }

    sInitialized = true;
    return ::bsp::BSP_OK;
}

::bsp::BspReturnCode Adc::read(uint8_t channelIdx, uint16_t& value)
{
    if (!sInitialized || (channelIdx >= fNumChannels))
    {
        return ::bsp::BSP_ERROR;
    }

    uint8_t const unit = fChannelConfigs[channelIdx].unit;
    int rawValue       = 0;
    if (adc_oneshot_read(
            sAdcHandles[unit],
            static_cast<adc_channel_t>(fChannelConfigs[channelIdx].channel),
            &rawValue)
        != ESP_OK)
    {
        return ::bsp::BSP_ERROR;
    }

    value = static_cast<uint16_t>(rawValue);
    return ::bsp::BSP_OK;
}

} // namespace bsp
