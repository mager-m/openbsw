// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspSpiWs2812
 *
 * WS2812 / NeoPixel addressable-LED strip driver for POSIX (Raspberry Pi)
 * targets using the Linux spidev interface.
 *
 * One instance drives one strip on one spidev bus. The pixel data is stored in
 * a caller-owned buffer in GRB byte order (the wire order WS2812 LEDs expect),
 * 3 bytes per LED. Each WS2812 data bit is emitted as three SPI bits at
 * 2.4 MHz, so that the ~1.25 us WS2812 bit period is reconstructed from the SPI
 * clock: a WS '1' becomes 0b110 and a WS '0' becomes 0b100.
 *
 * The public interface is identical to the ESP32 RMT-based driver so that
 * application code stays platform-neutral. Only the private members differ per
 * platform: this POSIX variant holds a spidev file descriptor and an internal
 * SPI byte buffer instead of RMT channel and encoder handles.
 */
#pragma once

#include "bsp/Bsp.h"

#include <cstdint>
#include <vector>

namespace bsp
{

class Ws2812Strip
{
public:
    Ws2812Strip() = default;

    Ws2812Strip(Ws2812Strip const&)            = delete;
    Ws2812Strip& operator=(Ws2812Strip const&) = delete;

    /**
     * Open the spidev bus and configure it for this strip.
     *
     * \param busNum     Selects the spidev bus: /dev/spidev{busNum}.0. On this
     *                   POSIX target the first parameter is the spidev bus
     *                   number, not a GPIO number as on the ESP32 variant.
     * \param numLeds    Number of LEDs on the strip.
     * \param grbBuffer  Caller-owned buffer, at least 3 * numLeds bytes, kept
     *                   alive for the lifetime of this instance. Holds the
     *                   pixel data in GRB order that is streamed on show().
     * \return BSP_OK on success, BSP_ERROR otherwise.
     */
    ::bsp::BspReturnCode init(uint8_t busNum, uint16_t numLeds, uint8_t* grbBuffer);

    /**
     * Write one pixel. r/g/b are the desired colour; the current brightness is
     * applied before the value is stored (in GRB order) into the buffer.
     */
    void setPixel(uint16_t i, uint8_t r, uint8_t g, uint8_t b);

    /** Set the global brightness scale (0 = off, 255 = full) applied by
     * subsequent setPixel() calls. */
    void setBrightness(uint8_t b);

    /** Zero all pixels in the buffer (does not transmit). */
    void clear();

    /**
     * Encode and transmit the current buffer to the strip, followed by a low
     * reset pulse (>= 50 us) that latches the pixel values, and block until the
     * transmission has completed.
     *
     * \return BSP_OK on success, BSP_ERROR otherwise.
     */
    ::bsp::BspReturnCode show();

private:
    uint8_t scale(uint8_t value) const;

    int _fd                          = -1;
    uint8_t* _grbBuffer              = nullptr;
    std::vector<uint8_t> _spiBuffer  = {};
    uint16_t _numLeds                = 0U;
    uint8_t _brightness              = 255U;
    bool _initialized                = false;
};

} // namespace bsp
