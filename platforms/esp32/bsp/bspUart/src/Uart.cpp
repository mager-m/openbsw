// Copyright 2024 Accenture.

#include "bsp/Uart.h"

#include "bsp/uart/UartConfig.h"

#include "driver/uart.h"

#include <cstring>

namespace bsp
{

Uart::Uart(Uart::Id id)
: _uartConfig(_uartConfigs[static_cast<size_t>(id)]), _initialized(false)
{}

Uart& Uart::getInstance(Id id)
{
    static Uart instance(Id::TERMINAL);
    (void)id;
    return instance;
}

void Uart::init()
{
    if (_initialized)
    {
        return;
    }

    uart_config_t uartCfg = {};
    uartCfg.baud_rate     = static_cast<int>(_uartConfig.baudRate);
    uartCfg.data_bits     = UART_DATA_8_BITS;
    uartCfg.parity        = UART_PARITY_DISABLE;
    uartCfg.stop_bits     = UART_STOP_BITS_1;
    uartCfg.flow_ctrl     = UART_HW_FLOWCTRL_DISABLE;
    uartCfg.source_clk    = UART_SCLK_DEFAULT;

    uart_port_t const port = static_cast<uart_port_t>(_uartConfig.uartNum);

    uart_driver_install(port, 1024 /* rx buf */, 1024 /* tx buf */, 0, nullptr, 0);
    uart_param_config(port, &uartCfg);
    uart_set_pin(
        port,
        _uartConfig.txPin,
        _uartConfig.rxPin,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE);

    _initialized = true;
}

size_t Uart::write(::etl::span<uint8_t const> const data)
{
    if (!_initialized)
    {
        return 0U;
    }
    int const written = uart_write_bytes(
        static_cast<uart_port_t>(_uartConfig.uartNum), data.data(), data.size());
    return (written > 0) ? static_cast<size_t>(written) : 0U;
}

size_t Uart::read(::etl::span<uint8_t> data)
{
    if (!_initialized)
    {
        return 0U;
    }
    int const bytesRead = uart_read_bytes(
        static_cast<uart_port_t>(_uartConfig.uartNum),
        data.data(),
        data.size(),
        0 /* no wait */);
    return (bytesRead > 0) ? static_cast<size_t>(bytesRead) : 0U;
}

bool Uart::isInitialized() const { return _initialized; }

bool Uart::waitForTxReady()
{
    if (!_initialized)
    {
        init();
    }
    return _initialized;
}

} // namespace bsp
