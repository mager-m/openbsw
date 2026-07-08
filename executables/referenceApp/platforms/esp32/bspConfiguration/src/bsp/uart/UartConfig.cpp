// Copyright 2024 Accenture.

#include <bsp/uart/UartConfig.h>

namespace bsp
{

/**
 * UART hardware configuration for Arduino Nano ESP32.
 * UART0: 115200 baud on header pins (GPIO43=TX, GPIO44=RX).
 */
Uart::UartConfig const Uart::_uartConfigs[] = {
    {0, 43, 44, 115200U}, // TERMINAL (UART0)
};

} // namespace bsp
