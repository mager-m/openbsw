// Copyright 2024 Accenture.

#include "systems/CanSystem.h"

#include <net/if.h> // if_nametoindex

#include <cstdlib>

namespace systems
{
static uint32_t const TIMEOUT_CAN_SYSTEM_IN_MS = 1U;
static int const MAX_SENT_PER_RUN              = 3;
static int const MAX_RECEIVED_PER_RUN          = 3;

namespace
{

// Per-bus SocketCAN interface: an override env var and a default name. Bus 0
// defaults to vcan0 and honours CAN_IFACE so it can point at a real adapter (e.g. a
// CANable); buses 1..3 default to vcan1..vcan3 and honour CAN_IFACE1..3. Only the
// buses whose interface actually exists are opened (see run()), so unused buses
// cost nothing.
struct IfaceConfig
{
    char const* env;   // override the interface name
    char const* name;  // default interface name
    char const* fdEnv; // opt CAN FD on for this bus
};
constexpr IfaceConfig IFACES[CanSystem::NUM_BUSES] = {
    {"CAN_IFACE", "vcan0", "CAN_FD"},
    {"CAN_IFACE1", "vcan1", "CAN_FD1"},
    {"CAN_IFACE2", "vcan2", "CAN_FD2"},
    {"CAN_IFACE3", "vcan3", "CAN_FD3"},
};

char const* ifaceFor(uint8_t const bus)
{
    char const* const env = ::std::getenv(IFACES[bus].env);
    return (env != nullptr) ? env : IFACES[bus].name;
}

// Per-bus CAN FD opt-in. Classic MCP2515 HATs reject the CAN_RAW_FD_FRAMES
// setsockopt, so FD is off by default and only enabled when the matching env
// var is truthy (first char 1/t/T/y/Y). vcan supports FD, so both settings
// round-trip on the virtual bus.
bool canFdFor(uint8_t const bus)
{
    char const* const v = ::std::getenv(IFACES[bus].fdEnv);
    return (v != nullptr)
           && ((v[0] == '1') || (v[0] == 't') || (v[0] == 'T') || (v[0] == 'y') || (v[0] == 'Y'));
}

// File-scope so each config outlives the transceiver that references it.
::can::SocketCanTransceiver::DeviceConfig canConfig[CanSystem::NUM_BUSES] = {
    {ifaceFor(0U), ::busid::CAN_0, canFdFor(0U)},
    {ifaceFor(1U), ::busid::CAN_1, canFdFor(1U)},
    {ifaceFor(2U), ::busid::CAN_2, canFdFor(2U)},
    {ifaceFor(3U), ::busid::CAN_3, canFdFor(3U)},
};

} // namespace

CanSystem::CanSystem(::async::ContextType context)
: _timeout()
, _context(context)
, _can0(canConfig[0])
, _can1(canConfig[1])
, _can2(canConfig[2])
, _can3(canConfig[3])
, _bus{&_can0, &_can1, &_can2, &_can3}
, _active{false, false, false, false}
{
    setTransitionContext(context);
}

void CanSystem::init() { transitionDone(); }

void CanSystem::run()
{
    for (uint8_t i = 0U; i < NUM_BUSES; ++i)
    {
        // Skip buses whose interface is absent so unused buses do not log a
        // failed open on every boot.
        if (::if_nametoindex(canConfig[i].name) == 0U)
        {
            continue;
        }
        _bus[i]->init();
        _bus[i]->open();
        _active[i] = true;
    }
    ::async::scheduleAtFixedRate(
        _context, *this, _timeout, TIMEOUT_CAN_SYSTEM_IN_MS, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void CanSystem::shutdown()
{
    _timeout.cancel();
    for (uint8_t i = 0U; i < NUM_BUSES; ++i)
    {
        if (_active[i])
        {
            _bus[i]->close();
            _bus[i]->shutdown();
        }
    }
    transitionDone();
}

::can::ICanTransceiver* CanSystem::getCanTransceiver(uint8_t busId)
{
    switch (busId)
    {
        case ::busid::CAN_0: return &_can0;
        case ::busid::CAN_1: return &_can1;
        case ::busid::CAN_2: return &_can2;
        case ::busid::CAN_3: return &_can3;
        default:             return nullptr;
    }
}

void CanSystem::execute()
{
    for (uint8_t i = 0U; i < NUM_BUSES; ++i)
    {
        if (_active[i])
        {
            _bus[i]->run(MAX_SENT_PER_RUN, MAX_RECEIVED_PER_RUN);
        }
    }
}

} // namespace systems
