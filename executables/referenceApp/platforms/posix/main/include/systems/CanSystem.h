// Copyright 2024 Accenture.

#pragma once

#include <can/SocketCanTransceiver.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <systems/ICanSystem.h>

namespace systems
{

class CanSystem final
: public ::can::ICanSystem
, public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    // vcan0..vcan3 map to CAN_0..CAN_3. A single-node build only brings up CAN_0
    // (vcan0); a multi-bus build uses all four. Buses whose SocketCAN interface
    // is absent stay inactive.
    static constexpr uint8_t NUM_BUSES = 4U;

    // [PUBLIC_API_START]
    explicit CanSystem(::async::ContextType context);
    CanSystem(CanSystem const&)            = delete;
    CanSystem& operator=(CanSystem const&) = delete;

    ::can::ICanTransceiver* getCanTransceiver(uint8_t busId) override;

    void init() final;
    void run() final;
    void shutdown() final;
    // [PUBLIC_API_END]
private:
    void execute() final;

    ::async::TimeoutType _timeout;
    ::async::ContextType _context;

    ::can::SocketCanTransceiver _can0;
    ::can::SocketCanTransceiver _can1;
    ::can::SocketCanTransceiver _can2;
    ::can::SocketCanTransceiver _can3;
    ::can::SocketCanTransceiver* const _bus[NUM_BUSES];
    bool _active[NUM_BUSES];
};

} // namespace systems
