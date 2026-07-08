// Copyright 2024 Accenture.

#pragma once

#include <can/Mcp2515CanTransceiver.h>
#include <can/TwaiCanTransceiver.h>
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
    explicit CanSystem(::async::ContextType context);
    CanSystem(CanSystem const&)            = delete;
    CanSystem& operator=(CanSystem const&) = delete;

    ::can::ICanTransceiver* getCanTransceiver(uint8_t busId) override;

    void init() final;
    void run() final;
    void shutdown() final;

private:
    void execute() final;

    ::async::TimeoutType _timeout;
    ::async::ContextType _context;
    ::can::TwaiCanTransceiver _canTransceiver;
    ::can::Mcp2515CanTransceiver _mcpTransceiver;
};

} // namespace systems
