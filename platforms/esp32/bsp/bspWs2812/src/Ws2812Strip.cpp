// Copyright 2024 Accenture.

#include "bsp/led/Ws2812Strip.h"

#include "driver/rmt_encoder.h"
#include "driver/rmt_tx.h"

namespace bsp
{

namespace
{
// RMT channel clock resolution: 10 MHz => 1 tick = 0.1 us.
constexpr uint32_t WS2812_RESOLUTION_HZ = 10000000U;

// WS2812 bit timing at 800 kHz expressed in 0.1 us ticks.
constexpr uint16_t WS2812_T0H_TICKS = 3U;  // 0.3 us high for a '0' bit
constexpr uint16_t WS2812_T0L_TICKS = 9U;  // 0.9 us low  for a '0' bit
constexpr uint16_t WS2812_T1H_TICKS = 9U;  // 0.9 us high for a '1' bit
constexpr uint16_t WS2812_T1L_TICKS = 3U;  // 0.3 us low  for a '1' bit

// Reset / latch pulse: total low time >= 50 us. Split across the two halves of
// one RMT symbol; 1400 + 1400 ticks = 280 us, safe for WS2812 and WS2812B.
constexpr uint16_t WS2812_RESET_HALF_TICKS = 1400U;

// RMT channel memory (in rmt_symbol_word_t units) and pending-transfer queue.
constexpr size_t WS2812_MEM_BLOCK_SYMBOLS = 64U;
constexpr size_t WS2812_TRANS_QUEUE_DEPTH = 4U;

// Blocking wait bound for a single frame transmission.
constexpr int WS2812_TX_TIMEOUT_MS = 1000;

rmt_symbol_word_t makeSymbol(
    uint32_t const level0, uint32_t const duration0, uint32_t const level1, uint32_t const duration1)
{
    rmt_symbol_word_t symbol = {};
    symbol.level0            = level0;
    symbol.duration0         = duration0;
    symbol.level1            = level1;
    symbol.duration1         = duration1;
    return symbol;
}

// One RMT TX channel and the two encoders are shared by every strip: the S3 has
// only 4 TX-capable RMT channels, and a node may drive more strips than that.
// show() rebinds the channel to the calling strip's GPIO. The encoders are GPIO
// independent, so they are created once. Not re-entrant: strips must be shown
// from a single task (the node component's async task guarantees this).
rmt_channel_handle_t sChannel      = nullptr;
int sChannelGpio                   = -1; // GPIO the channel is currently bound to
rmt_encoder_handle_t sBytesEncoder = nullptr;
rmt_encoder_handle_t sCopyEncoder  = nullptr;

// Create the shared byte/copy encoders once. Returns true if both are ready.
bool ensureEncoders()
{
    if (sBytesEncoder == nullptr)
    {
        rmt_bytes_encoder_config_t bytesCfg = {};
        bytesCfg.bit0            = makeSymbol(1U, WS2812_T0H_TICKS, 0U, WS2812_T0L_TICKS);
        bytesCfg.bit1            = makeSymbol(1U, WS2812_T1H_TICKS, 0U, WS2812_T1L_TICKS);
        bytesCfg.flags.msb_first = 1U;
        if (rmt_new_bytes_encoder(&bytesCfg, &sBytesEncoder) != ESP_OK)
        {
            sBytesEncoder = nullptr;
            return false;
        }
    }
    if (sCopyEncoder == nullptr)
    {
        rmt_copy_encoder_config_t copyCfg = {};
        if (rmt_new_copy_encoder(&copyCfg, &sCopyEncoder) != ESP_OK)
        {
            sCopyEncoder = nullptr;
            return false;
        }
    }
    return true;
}

// Bind the shared TX channel to gpioNum, (re)creating it if it is bound to a
// different GPIO. Returns true if the channel is enabled and on gpioNum.
bool ensureChannelFor(uint8_t const gpioNum)
{
    if ((sChannel != nullptr) && (sChannelGpio == static_cast<int>(gpioNum)))
    {
        return true;
    }
    if (sChannel != nullptr)
    {
        (void)rmt_disable(sChannel);
        (void)rmt_del_channel(sChannel);
        sChannel     = nullptr;
        sChannelGpio = -1;
    }

    rmt_channel_handle_t channel       = nullptr;
    rmt_tx_channel_config_t channelCfg = {};
    channelCfg.gpio_num                = static_cast<gpio_num_t>(gpioNum);
    channelCfg.clk_src                 = RMT_CLK_SRC_DEFAULT;
    channelCfg.resolution_hz           = WS2812_RESOLUTION_HZ;
    channelCfg.mem_block_symbols       = WS2812_MEM_BLOCK_SYMBOLS;
    channelCfg.trans_queue_depth       = WS2812_TRANS_QUEUE_DEPTH;
    if (rmt_new_tx_channel(&channelCfg, &channel) != ESP_OK)
    {
        return false;
    }
    if (rmt_enable(channel) != ESP_OK)
    {
        (void)rmt_del_channel(channel);
        return false;
    }
    sChannel     = channel;
    sChannelGpio = static_cast<int>(gpioNum);
    return true;
}
} // namespace

::bsp::BspReturnCode Ws2812Strip::init(uint8_t const gpioNum, uint16_t const numLeds, uint8_t* const grbBuffer)
{
    if (_initialized)
    {
        return ::bsp::BSP_OK;
    }
    if ((grbBuffer == nullptr) || (numLeds == 0U))
    {
        return ::bsp::BSP_ERROR;
    }
    if (!ensureEncoders())
    {
        return ::bsp::BSP_ERROR;
    }

    _grbBuffer   = grbBuffer;
    _numLeds     = numLeds;
    _gpioNum     = gpioNum;
    _initialized = true;
    return ::bsp::BSP_OK;
}

uint8_t Ws2812Strip::scale(uint8_t const value) const
{
    return static_cast<uint8_t>(
        (static_cast<uint16_t>(value) * (static_cast<uint16_t>(_brightness) + 1U)) >> 8U);
}

void Ws2812Strip::setPixel(uint16_t const i, uint8_t const r, uint8_t const g, uint8_t const b)
{
    if ((_grbBuffer == nullptr) || (i >= _numLeds))
    {
        return;
    }
    size_t const base    = static_cast<size_t>(i) * 3U;
    _grbBuffer[base + 0U] = scale(g);
    _grbBuffer[base + 1U] = scale(r);
    _grbBuffer[base + 2U] = scale(b);
}

void Ws2812Strip::setBrightness(uint8_t const b) { _brightness = b; }

void Ws2812Strip::clear()
{
    if (_grbBuffer == nullptr)
    {
        return;
    }
    size_t const bytes = static_cast<size_t>(_numLeds) * 3U;
    for (size_t idx = 0U; idx < bytes; ++idx)
    {
        _grbBuffer[idx] = 0U;
    }
}

::bsp::BspReturnCode Ws2812Strip::show()
{
    if (!_initialized)
    {
        return ::bsp::BSP_ERROR;
    }
    if (!ensureChannelFor(_gpioNum))
    {
        return ::bsp::BSP_ERROR;
    }

    rmt_transmit_config_t txConfig = {};
    txConfig.loop_count            = 0;      // transmit once
    txConfig.flags.eot_level       = 0U;     // idle low after the frame

    // Stream the GRB pixel bytes, MSB first, as WS2812 bit symbols.
    size_t const payloadBytes = static_cast<size_t>(_numLeds) * 3U;
    if (rmt_transmit(sChannel, sBytesEncoder, _grbBuffer, payloadBytes, &txConfig) != ESP_OK)
    {
        return ::bsp::BSP_ERROR;
    }

    // Append the reset / latch pulse. The symbol is kept on the stack and stays
    // valid until rmt_tx_wait_all_done() below returns.
    rmt_symbol_word_t const resetSymbol
        = makeSymbol(0U, WS2812_RESET_HALF_TICKS, 0U, WS2812_RESET_HALF_TICKS);
    if (rmt_transmit(sChannel, sCopyEncoder, &resetSymbol, sizeof(resetSymbol), &txConfig) != ESP_OK)
    {
        return ::bsp::BSP_ERROR;
    }

    if (rmt_tx_wait_all_done(sChannel, WS2812_TX_TIMEOUT_MS) != ESP_OK)
    {
        return ::bsp::BSP_ERROR;
    }
    return ::bsp::BSP_OK;
}

} // namespace bsp
