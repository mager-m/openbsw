// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspTwai
 *
 * CAN transceiver for ESP32-S3 using TWAI (Two-Wire Automotive Interface).
 */
#pragma once

#include <can/canframes/CANFrame.h>
#include <can/transceiver/AbstractCANTransceiver.h>

#include <cstdint>

namespace can
{

class TwaiCanTransceiver : public AbstractCANTransceiver
{
public:
    struct TwaiConfig
    {
        int txPin;
        int rxPin;
        uint32_t baudRate;
    };

    explicit TwaiCanTransceiver(uint8_t busId, TwaiConfig const& config);

    TwaiCanTransceiver(TwaiCanTransceiver const&)            = delete;
    TwaiCanTransceiver& operator=(TwaiCanTransceiver const&) = delete;

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
    TwaiConfig const _config;
    bool _initialized;
};

} // namespace can
