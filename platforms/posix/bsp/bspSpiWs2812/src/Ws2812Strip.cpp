// Copyright 2024 Accenture.

#include "bsp/led/Ws2812Strip.h"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>

namespace bsp
{
namespace
{
// SPI clock: three SPI bits encode one WS2812 data bit. At 2.4 MHz each SPI bit
// lasts ~0.417 us, so three of them reconstruct the ~1.25 us WS2812 bit period.
constexpr uint32_t WS2812_SPI_SPEED_HZ = 2400000U;

// spidev word size and mode. Mode 0 keeps the clock idle low.
constexpr uint8_t WS2812_SPI_MODE          = 0U;
constexpr uint8_t WS2812_SPI_BITS_PER_WORD = 8U;

// Each WS2812 data bit becomes three SPI bits. A whole WS byte (8 data bits)
// therefore becomes 24 SPI bits, i.e. exactly three SPI bytes, so the encoding
// stays byte-aligned. A GRB triplet (3 bytes) expands to 9 SPI bytes per LED.
constexpr size_t WS2812_SPI_BYTES_PER_DATA_BYTE = 3U;
constexpr size_t WS2812_SPI_BYTES_PER_LED       = 9U;

// Trailing reset / latch: a run of low (zero) SPI bytes. At 2.4 MHz one SPI
// byte lasts ~3.33 us, so 20 zero bytes give ~66.7 us, comfortably above the
// WS2812 latch requirement of >= 50 us.
constexpr size_t WS2812_RESET_BYTES = 20U;

// Three-SPI-bit symbols, most significant bit first: a WS '1' is 0b110 (long
// high, short low), a WS '0' is 0b100 (short high, long low).
constexpr uint32_t WS2812_SPI_SYMBOL_ONE  = 0x6U; // 0b110
constexpr uint32_t WS2812_SPI_SYMBOL_ZERO = 0x4U; // 0b100

/**
 * Encode one GRB byte into three SPI bytes, most significant WS bit first.
 *
 * The eight WS data bits are laid out into a 24-bit big-endian word (the first
 * WS bit occupies the top three bits, which spidev transmits first) and then
 * split into three bytes.
 */
void encodeByte(uint8_t const value, uint8_t* const out)
{
    uint32_t bits = 0U;
    for (int8_t bit = 7; bit >= 0; --bit)
    {
        uint32_t const symbol
            = (((static_cast<uint32_t>(value) >> static_cast<uint8_t>(bit)) & 1U) != 0U)
                  ? WS2812_SPI_SYMBOL_ONE
                  : WS2812_SPI_SYMBOL_ZERO;
        bits = (bits << 3U) | symbol;
    }
    out[0] = static_cast<uint8_t>((bits >> 16U) & 0xFFU);
    out[1] = static_cast<uint8_t>((bits >> 8U) & 0xFFU);
    out[2] = static_cast<uint8_t>(bits & 0xFFU);
}
} // namespace

::bsp::BspReturnCode
Ws2812Strip::init(uint8_t const busNum, uint16_t const numLeds, uint8_t* const grbBuffer)
{
    if (_initialized)
    {
        return ::bsp::BSP_OK;
    }
    if ((grbBuffer == nullptr) || (numLeds == 0U))
    {
        return ::bsp::BSP_ERROR;
    }

    char path[32];
    int const pathLen
        = ::snprintf(path, sizeof(path), "/dev/spidev%u.0", static_cast<unsigned>(busNum));
    if ((pathLen <= 0) || (static_cast<size_t>(pathLen) >= sizeof(path)))
    {
        return ::bsp::BSP_ERROR;
    }

    int const fd = ::open(path, O_RDWR);
    if (fd < 0)
    {
        return ::bsp::BSP_ERROR;
    }

    uint8_t const mode         = WS2812_SPI_MODE;
    uint8_t const bitsPerWord  = WS2812_SPI_BITS_PER_WORD;
    uint32_t const speedHz     = WS2812_SPI_SPEED_HZ;
    if ((::ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0)
        || (::ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) < 0)
        || (::ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speedHz) < 0))
    {
        (void)::close(fd);
        return ::bsp::BSP_ERROR;
    }

    _spiBuffer.assign(
        (static_cast<size_t>(numLeds) * WS2812_SPI_BYTES_PER_LED) + WS2812_RESET_BYTES, 0U);

    _fd          = fd;
    _grbBuffer   = grbBuffer;
    _numLeds     = numLeds;
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
    size_t const base     = static_cast<size_t>(i) * 3U;
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

    // Re-encode the GRB pixel bytes into the SPI buffer. The trailing reset
    // bytes stay zero (they were zeroed on init) and are never overwritten.
    size_t const payloadBytes = static_cast<size_t>(_numLeds) * 3U;
    for (size_t idx = 0U; idx < payloadBytes; ++idx)
    {
        encodeByte(_grbBuffer[idx], &_spiBuffer[idx * WS2812_SPI_BYTES_PER_DATA_BYTE]);
    }

    struct spi_ioc_transfer transfer = {};
    transfer.tx_buf       = static_cast<__u64>(reinterpret_cast<uintptr_t>(_spiBuffer.data()));
    transfer.rx_buf       = 0U;
    transfer.len          = static_cast<__u32>(_spiBuffer.size());
    transfer.speed_hz     = WS2812_SPI_SPEED_HZ;
    transfer.bits_per_word = WS2812_SPI_BITS_PER_WORD;
    transfer.delay_usecs  = 0U;

    if (::ioctl(_fd, SPI_IOC_MESSAGE(1), &transfer) < 0)
    {
        return ::bsp::BSP_ERROR;
    }
    return ::bsp::BSP_OK;
}

} // namespace bsp
