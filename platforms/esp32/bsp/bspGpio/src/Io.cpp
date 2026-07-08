// Copyright 2024 Accenture.

#include "bsp/Io.h"

#include "driver/gpio.h"

namespace bios
{

::bsp::BspReturnCode Io::setDefaultConfiguration(PinId io)
{
    if (io >= fPinConfigurationSize)
    {
        return ::bsp::BSP_ERROR;
    }
    return setConfiguration(io, fPinConfiguration[io]);
}

::bsp::BspReturnCode Io::setConfiguration(PinId io, PinConfiguration const& cfg)
{
    if (io >= fPinConfigurationSize)
    {
        return ::bsp::BSP_ERROR;
    }

    gpio_config_t gpioCfg = {};
    gpioCfg.pin_bit_mask  = (1ULL << cfg.gpioNum);

    if (cfg.dir == _OUT)
    {
        gpioCfg.mode = GPIO_MODE_OUTPUT;
    }
    else if (cfg.dir == _IN)
    {
        gpioCfg.mode = GPIO_MODE_INPUT;
    }
    else if (cfg.dir == _IN_OUT)
    {
        gpioCfg.mode = GPIO_MODE_INPUT_OUTPUT;
    }
    else
    {
        gpioCfg.mode = GPIO_MODE_DISABLE;
    }

    gpioCfg.pull_up_en   = cfg.pullUp ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    gpioCfg.pull_down_en = cfg.pullDown ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    gpioCfg.intr_type    = GPIO_INTR_DISABLE;

    if (gpio_config(&gpioCfg) != ESP_OK)
    {
        return ::bsp::BSP_ERROR;
    }
    return ::bsp::BSP_OK;
}

::bsp::BspReturnCode Io::getPin(PinId io, bool& level)
{
    if (io >= fPinConfigurationSize)
    {
        return ::bsp::BSP_ERROR;
    }
    level = (gpio_get_level(static_cast<gpio_num_t>(fPinConfiguration[io].gpioNum)) != 0);
    return ::bsp::BSP_OK;
}

::bsp::BspReturnCode Io::setPin(PinId io, bool level)
{
    if (io >= fPinConfigurationSize)
    {
        return ::bsp::BSP_ERROR;
    }
    if (gpio_set_level(
            static_cast<gpio_num_t>(fPinConfiguration[io].gpioNum), level ? 1 : 0)
        != ESP_OK)
    {
        return ::bsp::BSP_ERROR;
    }
    return ::bsp::BSP_OK;
}

} // namespace bios
