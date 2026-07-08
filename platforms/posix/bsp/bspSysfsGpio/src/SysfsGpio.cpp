// Copyright 2024 Accenture.

#include "bsp/gpio/SysfsGpio.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

namespace bsp
{

uint32_t SysfsGpio::sGpioNum[SysfsGpio::MAX_PINS] = {0U};
bool SysfsGpio::sIsOutput[SysfsGpio::MAX_PINS]    = {false};
size_t SysfsGpio::sNumPins                        = 0U;
bool SysfsGpio::sInitialized                      = false;

namespace
{

char const SYSFS_GPIO_EXPORT[] = "/sys/class/gpio/export";

/**
 * Write \p text to \p path using plain POSIX I/O.
 *
 * \return true if the file was opened and the whole string was written.
 */
bool writeToFile(char const* const path, char const* const text)
{
    int const fd = ::open(path, O_WRONLY);
    if (fd < 0)
    {
        return false;
    }
    size_t const len      = ::strlen(text);
    ssize_t const written = ::write(fd, text, len);
    (void)::close(fd);
    return (written >= 0) && (static_cast<size_t>(written) == len);
}

/**
 * Export a GPIO line. A line that is already exported reports EBUSY, which is
 * treated as success.
 *
 * \return true on success (or already exported), false otherwise.
 */
bool exportPin(uint32_t const gpioNum)
{
    int const fd = ::open(SYSFS_GPIO_EXPORT, O_WRONLY);
    if (fd < 0)
    {
        return false;
    }
    char buf[16];
    int const formatted = ::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(gpioNum));
    if ((formatted <= 0) || (static_cast<size_t>(formatted) >= sizeof(buf)))
    {
        (void)::close(fd);
        return false;
    }
    errno                 = 0;
    ssize_t const written = ::write(fd, buf, static_cast<size_t>(formatted));
    int const writeErrno  = errno;
    (void)::close(fd);
    if ((written >= 0) && (static_cast<size_t>(written) == static_cast<size_t>(formatted)))
    {
        return true;
    }
    // Already exported by a previous run or another process: not an error.
    return (written < 0) && (writeErrno == EBUSY);
}

/**
 * Build the sysfs path of a per-line attribute (for example "direction" or
 * "value") into \p out.
 *
 * \return true if the path fit into \p out.
 */
bool buildAttrPath(char* const out, size_t const outLen, uint32_t const gpioNum, char const* const attr)
{
    int const formatted = ::snprintf(
        out, outLen, "/sys/class/gpio/gpio%u/%s", static_cast<unsigned>(gpioNum), attr);
    return (formatted > 0) && (static_cast<size_t>(formatted) < outLen);
}

} // namespace

::bsp::BspReturnCode SysfsGpio::init(PinConfig const* const pins, size_t const n)
{
    if ((pins == nullptr) || (n == 0U) || (n > MAX_PINS))
    {
        return ::bsp::BSP_ERROR;
    }

    // Record the pin-to-index mapping first so that set()/get() stay
    // index-safe even if the sysfs access below fails on a non-Pi host.
    for (size_t i = 0U; i < n; ++i)
    {
        sGpioNum[i]  = pins[i].gpioNum;
        sIsOutput[i] = pins[i].output;
    }
    sNumPins     = n;
    sInitialized = true;

    ::bsp::BspReturnCode result = ::bsp::BSP_OK;
    for (size_t i = 0U; i < n; ++i)
    {
        if (!exportPin(pins[i].gpioNum))
        {
            result = ::bsp::BSP_ERROR;
            continue;
        }

        char path[64];
        if (!buildAttrPath(path, sizeof(path), pins[i].gpioNum, "direction"))
        {
            result = ::bsp::BSP_ERROR;
            continue;
        }
        char const* const direction = pins[i].output ? "out" : "in";
        if (!writeToFile(path, direction))
        {
            result = ::bsp::BSP_ERROR;
            continue;
        }

        if (pins[i].output)
        {
            if (!buildAttrPath(path, sizeof(path), pins[i].gpioNum, "value"))
            {
                result = ::bsp::BSP_ERROR;
                continue;
            }
            if (!writeToFile(path, pins[i].initialHigh ? "1" : "0"))
            {
                result = ::bsp::BSP_ERROR;
            }
        }
    }

    return result;
}

::bsp::BspReturnCode SysfsGpio::set(size_t const idx, bool const high)
{
    if ((!sInitialized) || (idx >= sNumPins) || (!sIsOutput[idx]))
    {
        return ::bsp::BSP_ERROR;
    }

    char path[64];
    if (!buildAttrPath(path, sizeof(path), sGpioNum[idx], "value"))
    {
        return ::bsp::BSP_ERROR;
    }
    return writeToFile(path, high ? "1" : "0") ? ::bsp::BSP_OK : ::bsp::BSP_ERROR;
}

::bsp::BspReturnCode SysfsGpio::get(size_t const idx, bool& high)
{
    if ((!sInitialized) || (idx >= sNumPins))
    {
        return ::bsp::BSP_ERROR;
    }

    char path[64];
    if (!buildAttrPath(path, sizeof(path), sGpioNum[idx], "value"))
    {
        return ::bsp::BSP_ERROR;
    }

    int const fd = ::open(path, O_RDONLY);
    if (fd < 0)
    {
        return ::bsp::BSP_ERROR;
    }
    char c            = '0';
    ssize_t const got = ::read(fd, &c, 1U);
    (void)::close(fd);
    if (got <= 0)
    {
        return ::bsp::BSP_ERROR;
    }
    high = (c == '1');
    return ::bsp::BSP_OK;
}

} // namespace bsp
