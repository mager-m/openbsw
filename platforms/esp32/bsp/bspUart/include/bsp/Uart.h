// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspUart
 *
 * UART driver for ESP32-S3 using ESP-IDF UART driver.
 */
#pragma once

#include "bsp/Bsp.h"

#include <etl/span.h>

#include <cstddef>
#include <cstdint>

namespace bsp
{

class Uart
{
public:
    enum class Id : size_t;

    explicit Uart(Uart::Id id);

    Uart(Uart const&)            = delete;
    Uart& operator=(Uart const&) = delete;

    static Uart& getInstance(Id id);

    size_t write(::etl::span<uint8_t const> const data);
    size_t read(::etl::span<uint8_t> data);
    void init();
    bool isInitialized() const;
    bool waitForTxReady();

private:
    struct UartConfig
    {
        int uartNum;
        int txPin;
        int rxPin;
        uint32_t baudRate;
    };

    static UartConfig const _uartConfigs[];
    UartConfig const& _uartConfig;
    bool _initialized;
};

} // namespace bsp
