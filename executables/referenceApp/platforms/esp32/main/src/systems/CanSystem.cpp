// Copyright 2024 Accenture.

#include "systems/CanSystem.h"

#include "bsp/can/CanConfig.h"

#include <busid/BusId.h>

namespace systems
{

CanSystem::CanSystem(::async::ContextType context)
: _timeout()
, _context(context)
, _canTransceiver(::busid::CAN_0, ::can::defaultTwaiConfig)
, _mcpTransceiver(::busid::CAN_1, ::can::defaultMcp2515Config)
{}

::can::ICanTransceiver* CanSystem::getCanTransceiver(uint8_t busId)
{
    if (busId == ::busid::CAN_0)
    {
        return &_canTransceiver;
    }
    if (busId == ::busid::CAN_1)
    {
        return &_mcpTransceiver;
    }
    return nullptr;
}

void CanSystem::init()
{
    _canTransceiver.init();
    _mcpTransceiver.init();
    transitionDone();
}

void CanSystem::run()
{
    ::async::scheduleAtFixedRate(_context, *this, _timeout, 10U, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void CanSystem::shutdown()
{
    _timeout.cancel();
    _canTransceiver.shutdown();
    _mcpTransceiver.shutdown();
    transitionDone();
}

void CanSystem::execute()
{
    _canTransceiver.run();
    _mcpTransceiver.run();
}

} // namespace systems
