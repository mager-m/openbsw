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
, _mcpTransceiver2(::busid::CAN_2, ::can::mcp2515ConfigCan2)
, _mcpTransceiver3(::busid::CAN_3, ::can::mcp2515ConfigCan3)
{}

::can::ICanTransceiver* CanSystem::getCanTransceiver(uint8_t busId)
{
    switch (busId)
    {
        case ::busid::CAN_0: return &_canTransceiver;
        case ::busid::CAN_1: return &_mcpTransceiver;
        case ::busid::CAN_2: return &_mcpTransceiver2;
        case ::busid::CAN_3: return &_mcpTransceiver3;
        default:             return nullptr;
    }
}

void CanSystem::init()
{
    _canTransceiver.init();
    _mcpTransceiver.init();
    _mcpTransceiver2.init();
    _mcpTransceiver3.init();
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
    _mcpTransceiver2.shutdown();
    _mcpTransceiver3.shutdown();
    transitionDone();
}

void CanSystem::execute()
{
    _canTransceiver.run();
    _mcpTransceiver.run();
    _mcpTransceiver2.run();
    _mcpTransceiver3.run();
}

} // namespace systems
