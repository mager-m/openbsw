// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspConfiguration
 *
 * CAN (TWAI) configuration for Arduino Nano ESP32.
 * Pins default to TX=GPIO5 (D2), RX=GPIO6 (D3) and are build-configurable via
 * -DTWAI_TX_PIN / -DTWAI_RX_PIN (see platforms/esp32/Options.cmake).
 * Requires an external CAN transceiver (e.g., SN65HVD230).
 */
#pragma once

#include "can/Mcp2515CanTransceiver.h"
#include "can/TwaiCanTransceiver.h"

#ifndef TWAI_TX_PIN
#define TWAI_TX_PIN 5
#endif
#ifndef TWAI_RX_PIN
#define TWAI_RX_PIN 6
#endif

namespace can
{

inline TwaiCanTransceiver::TwaiConfig const defaultTwaiConfig = {
    TWAI_TX_PIN, // TX pin (default D2 / GPIO5)
    TWAI_RX_PIN, // RX pin (default D3 / GPIO6)
    500000U,     // 500 kbit/s
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
