// Copyright 2024 Accenture.

#include "bsp/LedcPwm.h"

#include "driver/ledc.h"

namespace bsp
{

bool LedcPwm::sInitialized = false;

::bsp::BspReturnCode LedcPwm::init()
{
    if (sInitialized)
    {
        return ::bsp::BSP_OK;
    }

    for (uint8_t i = 0U; i < fNumChannels; ++i)
    {
        ledc_timer_config_t timerCfg = {};
        timerCfg.speed_mode          = LEDC_LOW_SPEED_MODE;
        timerCfg.timer_num    = static_cast<ledc_timer_t>(fChannelConfigs[i].timer);
        timerCfg.duty_resolution
            = static_cast<ledc_timer_bit_t>(fChannelConfigs[i].resolution);
        timerCfg.freq_hz = fChannelConfigs[i].frequency;
        timerCfg.clk_cfg = LEDC_AUTO_CLK;

        if (ledc_timer_config(&timerCfg) != ESP_OK)
        {
            return ::bsp::BSP_ERROR;
        }

        ledc_channel_config_t chanCfg = {};
        chanCfg.speed_mode            = LEDC_LOW_SPEED_MODE;
        chanCfg.channel  = static_cast<ledc_channel_t>(fChannelConfigs[i].channel);
        chanCfg.timer_sel = static_cast<ledc_timer_t>(fChannelConfigs[i].timer);
        chanCfg.intr_type = LEDC_INTR_DISABLE;
        chanCfg.gpio_num  = fChannelConfigs[i].gpioNum;
        chanCfg.duty      = 0;
        chanCfg.hpoint    = 0;

        if (ledc_channel_config(&chanCfg) != ESP_OK)
        {
            return ::bsp::BSP_ERROR;
        }
    }

    sInitialized = true;
    return ::bsp::BSP_OK;
}

::bsp::BspReturnCode LedcPwm::setDuty(uint8_t channelIdx, uint32_t duty)
{
    if (!sInitialized || (channelIdx >= fNumChannels))
    {
        return ::bsp::BSP_ERROR;
    }

    auto const chan = static_cast<ledc_channel_t>(fChannelConfigs[channelIdx].channel);
    if (ledc_set_duty(LEDC_LOW_SPEED_MODE, chan, duty) != ESP_OK)
    {
        return ::bsp::BSP_ERROR;
    }
    if (ledc_update_duty(LEDC_LOW_SPEED_MODE, chan) != ESP_OK)
    {
        return ::bsp::BSP_ERROR;
    }
    return ::bsp::BSP_OK;
}

::bsp::BspReturnCode LedcPwm::getDuty(uint8_t channelIdx, uint32_t& duty)
{
    if (!sInitialized || (channelIdx >= fNumChannels))
    {
        return ::bsp::BSP_ERROR;
    }

    auto const chan = static_cast<ledc_channel_t>(fChannelConfigs[channelIdx].channel);
    duty            = ledc_get_duty(LEDC_LOW_SPEED_MODE, chan);
    return ::bsp::BSP_OK;
}

} // namespace bsp
