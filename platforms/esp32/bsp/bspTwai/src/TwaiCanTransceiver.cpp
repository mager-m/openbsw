// Copyright 2024 Accenture.

#include "can/TwaiCanTransceiver.h"

#include "can/canframes/ICANFrameSentListener.h"
#include "driver/twai.h"

namespace can
{

TwaiCanTransceiver::TwaiCanTransceiver(uint8_t busId, TwaiConfig const& config)
: AbstractCANTransceiver(busId), _config(config), _initialized(false)
{}

ICanTransceiver::ErrorCode TwaiCanTransceiver::init()
{
    if (_initialized)
    {
        return ErrorCode::CAN_ERR_OK;
    }

    twai_general_config_t generalCfg = TWAI_GENERAL_CONFIG_DEFAULT(
        static_cast<gpio_num_t>(_config.txPin),
        static_cast<gpio_num_t>(_config.rxPin),
        TWAI_MODE_NORMAL);

    twai_timing_config_t timingCfg;
    switch (_config.baudRate)
    {
        case 125000U: timingCfg = TWAI_TIMING_CONFIG_125KBITS(); break;
        case 250000U: timingCfg = TWAI_TIMING_CONFIG_250KBITS(); break;
        case 500000U: timingCfg = TWAI_TIMING_CONFIG_500KBITS(); break;
        case 1000000U: timingCfg = TWAI_TIMING_CONFIG_1MBITS(); break;
        default: timingCfg = TWAI_TIMING_CONFIG_500KBITS(); break;
    }

    twai_filter_config_t filterCfg = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&generalCfg, &timingCfg, &filterCfg) != ESP_OK)
    {
        return ErrorCode::CAN_ERR_INIT_FAILED;
    }

    if (twai_start() != ESP_OK)
    {
        twai_driver_uninstall();
        return ErrorCode::CAN_ERR_INIT_FAILED;
    }

    _initialized = true;
    setState(State::INITIALIZED);
    return ErrorCode::CAN_ERR_OK;
}

void TwaiCanTransceiver::shutdown()
{
    if (_initialized)
    {
        twai_stop();
        twai_driver_uninstall();
        _initialized = false;
        setState(State::CLOSED);
    }
}

ICanTransceiver::ErrorCode TwaiCanTransceiver::open(CANFrame const& frame)
{
    if (!isInState(State::INITIALIZED))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::OPEN);
    (void)write(frame);
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode TwaiCanTransceiver::open()
{
    if (!isInState(State::INITIALIZED))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::OPEN);
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode TwaiCanTransceiver::close()
{
    shutdown();
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode TwaiCanTransceiver::mute()
{
    if (!isInState(State::OPEN))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::MUTED);
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode TwaiCanTransceiver::unmute()
{
    if (!isInState(State::MUTED))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::OPEN);
    return ErrorCode::CAN_ERR_OK;
}

uint32_t TwaiCanTransceiver::getBaudrate() const { return _config.baudRate; }

uint16_t TwaiCanTransceiver::getHwQueueTimeout() const
{
    // Approximate: one CAN frame at 500kbps takes ~230us, queue depth ~5
    return static_cast<uint16_t>((_config.baudRate > 0U) ? (5U * 8U * 130U * 1000U / _config.baudRate) : 10U);
}

ICanTransceiver::ErrorCode TwaiCanTransceiver::write(CANFrame const& frame)
{
    if (!_initialized)
    {
        return ErrorCode::CAN_ERR_TX_OFFLINE;
    }

    twai_message_t msg = {};
    msg.identifier     = frame.getId();
    msg.data_length_code
        = (frame.getPayloadLength() > 8U) ? 8U : frame.getPayloadLength();
    msg.extd = 0;
    msg.rtr  = 0;

    for (uint8_t i = 0U; i < msg.data_length_code; ++i)
    {
        msg.data[i] = frame.getPayload()[i];
    }

    if (twai_transmit(&msg, 0 /* no wait */) != ESP_OK)
    {
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }

    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode TwaiCanTransceiver::write(
    CANFrame const& frame, ICANFrameSentListener& listener)
{
    ErrorCode const result = write(frame);
    if (result == ErrorCode::CAN_ERR_OK)
    {
        // Notify the per-write listener (drives DoCAN tx-confirmation) as well as
        // globally-registered sent-listeners, matching SocketCanTransceiver semantics.
        listener.canFrameSent(frame);
        notifySentListeners(frame);
    }
    return result;
}

void TwaiCanTransceiver::run()
{
    if (!_initialized)
    {
        return;
    }

    twai_message_t msg;
    while (twai_receive(&msg, 0 /* no wait */) == ESP_OK)
    {
        CANFrame frame;
        frame.setId(msg.identifier);
        frame.setPayloadLength(msg.data_length_code);
        for (uint8_t i = 0U; i < msg.data_length_code; ++i)
        {
            frame.getPayload()[i] = msg.data[i];
        }
        notifyListeners(frame);
    }
}

} // namespace can
