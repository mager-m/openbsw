// Copyright 2024 Accenture.

#include "bsp/gpio/SysfsGpio.h"

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
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
 * Read an unsigned decimal integer from a sysfs attribute file.
 *
 * \return true if the file was read and parsed.
 */
bool readUintFile(char const* const path, unsigned long& value)
{
    int const fd = ::open(path, O_RDONLY);
    if (fd < 0)
    {
        return false;
    }
    char buf[32];
    ssize_t const got = ::read(fd, buf, sizeof(buf) - 1U);
    (void)::close(fd);
    if (got <= 0)
    {
        return false;
    }
    buf[got] = '\0';
    char* end                  = nullptr;
    unsigned long const parsed = ::strtoul(buf, &end, 10);
    if (end == buf)
    {
        return false;
    }
    value = parsed;
    return true;
}

/**
 * Resolve the base of the SoC GPIO controller in the legacy sysfs numberspace.
 *
 * That numberspace is global: a line's number is its controller's base plus the
 * line offset within that controller. The SoC controller's base is not
 * guaranteed to be zero (6.x kernels commonly place it at 512), so a raw offset
 * addresses the wrong line. This returns the base of the controller exposing
 * the most lines (the SoC bank), or 0 if none can be read (for example on a
 * host without /sys/class/gpio), which leaves offsets unchanged as on a
 * base-zero system.
 */
unsigned long resolveSocBase()
{
    DIR* const dir = ::opendir("/sys/class/gpio");
    if (dir == nullptr)
    {
        return 0UL;
    }
    unsigned long bestBase  = 0UL;
    unsigned long bestNgpio = 0UL;
    for (dirent const* entry = ::readdir(dir); entry != nullptr; entry = ::readdir(dir))
    {
        unsigned int base = 0U;
        if (::sscanf(entry->d_name, "gpiochip%u", &base) != 1)
        {
            continue;
        }
        char path[64];
        int const len = ::snprintf(path, sizeof(path), "/sys/class/gpio/%s/ngpio", entry->d_name);
        if ((len <= 0) || (static_cast<size_t>(len) >= sizeof(path)))
        {
            continue;
        }
        unsigned long ngpio = 0UL;
        if (readUintFile(path, ngpio) && (ngpio > bestNgpio))
        {
            bestBase  = base;
            bestNgpio = ngpio;
        }
    }
    (void)::closedir(dir);
    return bestBase;
}

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
    // index-safe even if the sysfs access below fails on a non-Pi host. The
    // caller-supplied offsets are resolved to kernel-global line numbers (base
    // resolved once, shared by every pin), so every later access uses them.
    unsigned long const socBase = resolveSocBase();
    for (size_t i = 0U; i < n; ++i)
    {
        sGpioNum[i]  = static_cast<uint32_t>(socBase + pins[i].gpioNum);
        sIsOutput[i] = pins[i].output;
    }
    sNumPins     = n;
    sInitialized = true;

    ::bsp::BspReturnCode result = ::bsp::BSP_OK;
    for (size_t i = 0U; i < n; ++i)
    {
        if (!exportPin(sGpioNum[i]))
        {
            result = ::bsp::BSP_ERROR;
            continue;
        }

        char path[64];
        if (!buildAttrPath(path, sizeof(path), sGpioNum[i], "direction"))
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
            if (!buildAttrPath(path, sizeof(path), sGpioNum[i], "value"))
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
