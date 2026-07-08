// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspConfiguration
 *
 * CAN (TWAI) configuration for Arduino Nano ESP32.
 * Default pins: TX=GPIO5 (D2), RX=GPIO6 (D3).
 * Requires an external CAN transceiver (e.g., SN65HVD230).
 */
#pragma once

#include "can/Mcp2515CanTransceiver.h"
#include "can/TwaiCanTransceiver.h"

namespace can
{

inline TwaiCanTransceiver::TwaiConfig const defaultTwaiConfig = {
    5,       // TX pin (D2 / GPIO5)
    6,       // RX pin (D3 / GPIO6)
    500000U, // 500 kbit/s
};

// MCP2515 SPI CAN controller = second CAN bus (busid::CAN_1).
// Placeholder pins/host: set to the real Arduino Nano ESP32 wiring.
// NOTE: bit timing is hard-coded for an 8 MHz MCP2515 crystal at 500 kbit/s.
inline Mcp2515CanTransceiver::Mcp2515Config const defaultMcp2515Config = {
    2,         // spiHost: 0=SPI1(flash), 1=SPI2/FSPI, 2=SPI3 (value 2 = SPI3_HOST)
    12,        // SCLK
    11,        // MOSI
    13,        // MISO
    10,        // CS
    10000000U, // 10 MHz SPI clock
    500000U,   // 500 kbit/s CAN
};

} // namespace can
