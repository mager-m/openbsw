// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspMcp2515
 *
 * CAN transceiver for ESP32-S3 driving an external MCP2515 SPI CAN controller.
 */
#pragma once

#include <can/canframes/CANFrame.h>
#include <can/transceiver/AbstractCANTransceiver.h>

#include <cstdint>

// Opaque ESP-IDF SPI device handle target; forward-declared so this public
// header does not pull in the ESP-IDF driver headers (kept PRIVATE in CMake).
struct spi_device_t;

namespace can
{

class Mcp2515CanTransceiver final : public AbstractCANTransceiver
{
public:
    struct Mcp2515Config
    {
        int spiHost;
        int sclkPin;
        int mosiPin;
        int misoPin;
        int csPin;
        uint32_t spiClockHz;
        uint32_t canBaud;
    };

    explicit Mcp2515CanTransceiver(uint8_t busId, Mcp2515Config const& config)
    : AbstractCANTransceiver(busId), _config(config), _initialized(false), _spi(nullptr)
    {}

    Mcp2515CanTransceiver(Mcp2515CanTransceiver const&)            = delete;
    Mcp2515CanTransceiver& operator=(Mcp2515CanTransceiver const&) = delete;

    ErrorCode init() override;
    void shutdown() override;
    ErrorCode open(CANFrame const& frame) override;
    ErrorCode open() override;
    ErrorCode close() override;
    ErrorCode mute() override;
    ErrorCode unmute() override;
    uint32_t getBaudrate() const override;
    uint16_t getHwQueueTimeout() const override;
    ErrorCode write(CANFrame const& frame) override;
    ErrorCode write(CANFrame const& frame, ICANFrameSentListener& listener) override;

    void run();

private:
    void readRxBuffer(uint8_t readCmd);

    Mcp2515Config const _config;
    bool _initialized;
    ::spi_device_t* _spi;
};

} // namespace can
