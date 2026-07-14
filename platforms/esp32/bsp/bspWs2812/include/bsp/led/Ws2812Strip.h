// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspWs2812
 *
 * WS2812 / NeoPixel addressable-LED strip driver for ESP32-S3 using the RMT
 * (Remote Control) TX peripheral.
 *
 * One instance drives one strip on one GPIO. All instances SHARE a single RMT TX
 * channel that show() re-points to the caller's GPIO on demand, because the
 * ESP32-S3 has only 4 TX-capable RMT channels but an application may drive more
 * strips than that. This assumes the strips are shown from a single task:
 * show() is not re-entrant.
 *
 * The pixel data is stored in a caller-owned buffer in GRB byte order (the wire
 * order WS2812 LEDs expect), 3 bytes per LED, emitted at 800 kHz using a 10 MHz
 * RMT resolution (1 tick = 0.1 us). The public interface exposes no ESP-IDF types.
 */
#pragma once

#include "bsp/Bsp.h"

#include <cstdint>

namespace bsp
{

class Ws2812Strip
{
public:
    Ws2812Strip() = default;

    Ws2812Strip(Ws2812Strip const&)            = delete;
    Ws2812Strip& operator=(Ws2812Strip const&) = delete;

    /**
     * Record this strip's GPIO/buffer and create the shared WS2812 bit encoders
     * on first use. The shared RMT TX channel is (re)bound to this GPIO lazily by
     * show(), so init() allocates no per-strip channel.
     *
     * \param gpioNum    GPIO the WS2812 data line is wired to.
     * \param numLeds    Number of LEDs on the strip.
     * \param grbBuffer  Caller-owned buffer, at least 3 * numLeds bytes, kept
     *                   alive for the lifetime of this instance. Holds the
     *                   pixel data in GRB order that is streamed on show().
     * \return BSP_OK on success, BSP_ERROR otherwise.
     */
    ::bsp::BspReturnCode init(uint8_t gpioNum, uint16_t numLeds, uint8_t* grbBuffer);

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

    uint8_t* _grbBuffer = nullptr;
    uint16_t _numLeds   = 0U;
    uint8_t _gpioNum    = 0U;
    uint8_t _brightness = 255U;
    bool _initialized   = false;
};

} // namespace bsp
