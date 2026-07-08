// Copyright 2024 Accenture.

#include "bsp/pwm/LedcPwm.h"

namespace bsp
{

LedcPwm::ChannelConfig LedcPwm::sChannels[LedcPwm::MAX_CHANNELS] = {};
size_t LedcPwm::sNumChannels                                     = 0U;
bool LedcPwm::sInitialized                                       = false;

::bsp::BspReturnCode LedcPwm::init(ChannelConfig const* const channels, size_t const n)
{
    if (sInitialized)
    {
        return ::bsp::BSP_OK;
    }

    if ((channels == nullptr) || (n == 0U) || (n > MAX_CHANNELS))
    {
        return ::bsp::BSP_ERROR;
    }

    // Configure each distinct timer once, taking its frequency and resolution
    // from the first channel that references it.
    bool timerConfigured[LEDC_TIMER_MAX] = {};
    for (size_t i = 0U; i < n; ++i)
    {
        auto const timerIdx = static_cast<size_t>(channels[i].timer);
        if (timerIdx >= static_cast<size_t>(LEDC_TIMER_MAX))
        {
            return ::bsp::BSP_ERROR;
        }
        if (timerConfigured[timerIdx])
        {
            continue;
        }

        ledc_timer_config_t timerCfg = {};
        timerCfg.speed_mode          = LEDC_LOW_SPEED_MODE;
        timerCfg.timer_num           = channels[i].timer;
        timerCfg.duty_resolution     = channels[i].resolution;
        timerCfg.freq_hz             = channels[i].frequencyHz;
        timerCfg.clk_cfg             = LEDC_AUTO_CLK;

        if (ledc_timer_config(&timerCfg) != ESP_OK)
        {
            return ::bsp::BSP_ERROR;
        }
        timerConfigured[timerIdx] = true;
    }

    // Bind every channel to its GPIO with an initial duty of 0.
    for (size_t i = 0U; i < n; ++i)
    {
        ledc_channel_config_t chanCfg = {};
        chanCfg.speed_mode            = LEDC_LOW_SPEED_MODE;
        chanCfg.channel               = channels[i].channel;
        chanCfg.timer_sel             = channels[i].timer;
        chanCfg.intr_type             = LEDC_INTR_DISABLE;
        chanCfg.gpio_num              = static_cast<int>(channels[i].gpioNum);
        chanCfg.duty                  = 0U;
        chanCfg.hpoint                = 0;

        if (ledc_channel_config(&chanCfg) != ESP_OK)
        {
            return ::bsp::BSP_ERROR;
        }

        sChannels[i] = channels[i];
    }

    sNumChannels = n;
    sInitialized = true;
    return ::bsp::BSP_OK;
}

::bsp::BspReturnCode LedcPwm::setDutyRaw(size_t const chanIdx, uint32_t const duty)
{
    if ((!sInitialized) || (chanIdx >= sNumChannels))
    {
        return ::bsp::BSP_ERROR;
    }

    ledc_channel_t const chan = sChannels[chanIdx].channel;
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

::bsp::BspReturnCode LedcPwm::setDutyPercent(size_t const chanIdx, uint16_t perMille)
{
    if ((!sInitialized) || (chanIdx >= sNumChannels))
    {
        return ::bsp::BSP_ERROR;
    }

    if (perMille > 1000U)
    {
        perMille = 1000U;
    }

    uint32_t const res     = static_cast<uint32_t>(sChannels[chanIdx].resolution);
    uint32_t const maxDuty = (static_cast<uint32_t>(1U) << res) - 1U;
    uint32_t const duty
        = static_cast<uint32_t>((static_cast<uint64_t>(maxDuty) * perMille) / 1000ULL);

    return setDutyRaw(chanIdx, duty);
}

::bsp::BspReturnCode LedcPwm::setPulseMicroseconds(size_t const chanIdx, uint16_t const us)
{
    if ((!sInitialized) || (chanIdx >= sNumChannels))
    {
        return ::bsp::BSP_ERROR;
    }

    uint32_t const res     = static_cast<uint32_t>(sChannels[chanIdx].resolution);
    uint32_t const freqHz  = sChannels[chanIdx].frequencyHz;
    uint32_t const maxDuty = (static_cast<uint32_t>(1U) << res) - 1U;

    // duty = us * 2^res * freqHz / 1e6, computed in 64-bit to avoid overflow.
    uint64_t duty = (static_cast<uint64_t>(us) * (static_cast<uint64_t>(1U) << res)
                     * static_cast<uint64_t>(freqHz))
                    / 1000000ULL;
    if (duty > maxDuty)
    {
        duty = maxDuty;
    }

    return setDutyRaw(chanIdx, static_cast<uint32_t>(duty));
}

::bsp::BspReturnCode LedcPwm::setServoAngle(
    size_t const chanIdx, uint8_t angle0to180, uint16_t const minUs, uint16_t const maxUs)
{
    if (maxUs < minUs)
    {
        return ::bsp::BSP_ERROR;
    }

    if (angle0to180 > 180U)
    {
        angle0to180 = 180U;
    }

    uint16_t const span = static_cast<uint16_t>(maxUs - minUs);
    uint16_t const us   = static_cast<uint16_t>(
        static_cast<uint32_t>(minUs)
        + ((static_cast<uint32_t>(span) * static_cast<uint32_t>(angle0to180)) / 180U));

    return setPulseMicroseconds(chanIdx, us);
}

} // namespace bsp
