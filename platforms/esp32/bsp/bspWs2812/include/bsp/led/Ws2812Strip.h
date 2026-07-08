// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspWs2812
 *
 * WS2812 / NeoPixel addressable-LED strip driver for ESP32-S3 using the RMT
 * (Remote Control) TX peripheral.
 *
 * One instance drives one strip on one GPIO through one dedicated RMT TX
 * channel. The pixel data is stored in a caller-owned buffer in GRB byte order
 * (the wire order WS2812 LEDs expect), 3 bytes per LED. Colours are emitted at
 * 800 kHz using a 10 MHz RMT resolution (1 tick = 0.1 us).
 *
 * The public interface deliberately exposes no ESP-IDF types so that consumers
 * do not need the esp-idf driver headers on their include path; the RMT channel
 * and encoder handles are held as opaque pointers.
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
     * Set up the RMT TX channel and WS2812 bit encoders for this strip.
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

    uint8_t* _grbBuffer   = nullptr;
    void* _channel        = nullptr; // rmt_channel_handle_t
    void* _bytesEncoder   = nullptr; // rmt_encoder_handle_t (GRB bit stream)
    void* _copyEncoder    = nullptr; // rmt_encoder_handle_t (reset pulse)
    uint16_t _numLeds     = 0U;
    uint8_t _brightness   = 255U;
    bool _initialized     = false;
};

} // namespace bsp
