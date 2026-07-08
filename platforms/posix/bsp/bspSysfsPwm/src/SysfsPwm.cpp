// Copyright 2024 Accenture.

#include "bsp/pwm/SysfsPwm.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>

namespace bsp
{
namespace
{
/** Number of nanoseconds in one second. */
constexpr uint64_t NS_PER_SECOND = 1000000000ULL;
/** Number of nanoseconds in one microsecond. */
constexpr uint64_t NS_PER_MICROSECOND = 1000ULL;

/**
 * Write an unsigned decimal value to a sysfs attribute file.
 *
 * \return true on success, false if the file cannot be opened or fully written.
 */
bool writeUintFile(char const* const path, uint64_t const value)
{
    int const fd = ::open(path, O_WRONLY);
    if (fd < 0)
    {
        return false;
    }

    char buf[32];
    int const len
        = ::snprintf(buf, sizeof(buf), "%llu", static_cast<unsigned long long>(value));

    bool ok = false;
    if ((len > 0) && (static_cast<size_t>(len) < sizeof(buf)))
    {
        ssize_t const written = ::write(fd, buf, static_cast<size_t>(len));
        ok                    = (written == static_cast<ssize_t>(len));
    }

    (void)::close(fd);
    return ok;
}

/** Build a per-channel attribute path and write an unsigned decimal value. */
bool writeChannelAttr(
    uint8_t const chip, uint8_t const channel, char const* const attr, uint64_t const value)
{
    char path[96];
    int const len = ::snprintf(
        path,
        sizeof(path),
        "/sys/class/pwm/pwmchip%u/pwm%u/%s",
        static_cast<unsigned>(chip),
        static_cast<unsigned>(channel),
        attr);
    if ((len <= 0) || (static_cast<size_t>(len) >= sizeof(path)))
    {
        return false;
    }
    return writeUintFile(path, value);
}

/**
 * Export a channel on its chip.
 *
 * Writing a channel that is already exported yields EBUSY, which is treated as
 * success (the channel is available for configuration either way).
 */
bool exportChannel(uint8_t const chip, uint8_t const channel)
{
    char path[64];
    int const pathLen = ::snprintf(
        path, sizeof(path), "/sys/class/pwm/pwmchip%u/export", static_cast<unsigned>(chip));
    if ((pathLen <= 0) || (static_cast<size_t>(pathLen) >= sizeof(path)))
    {
        return false;
    }

    int const fd = ::open(path, O_WRONLY);
    if (fd < 0)
    {
        return false;
    }

    char buf[16];
    int const len = ::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(channel));

    bool ok = false;
    if ((len > 0) && (static_cast<size_t>(len) < sizeof(buf)))
    {
        ssize_t const written = ::write(fd, buf, static_cast<size_t>(len));
        int const err         = errno;
        // A fully written value succeeds; an already-exported channel reports
        // EBUSY, which we accept as an equivalent success.
        ok = (written == static_cast<ssize_t>(len)) || ((written < 0) && (err == EBUSY));
    }

    (void)::close(fd);
    return ok;
}
} // namespace

SysfsPwm::ChannelState SysfsPwm::sChannels[SysfsPwm::MAX_CHANNELS] = {};
size_t SysfsPwm::sNumChannels                                      = 0U;
bool SysfsPwm::sInitialized                                        = false;

::bsp::BspReturnCode SysfsPwm::init(ChannelConfig const* const channels, size_t const n)
{
    if (sInitialized)
    {
        return ::bsp::BSP_OK;
    }

    if ((channels == nullptr) || (n == 0U) || (n > MAX_CHANNELS))
    {
        return ::bsp::BSP_ERROR;
    }

    for (size_t i = 0U; i < n; ++i)
    {
        uint32_t const freqHz = channels[i].frequencyHz;
        if (freqHz == 0U)
        {
            return ::bsp::BSP_ERROR;
        }

        uint8_t const chip      = channels[i].chip;
        uint8_t const channel   = channels[i].channel;
        uint64_t const periodNs = NS_PER_SECOND / static_cast<uint64_t>(freqHz);

        if (!exportChannel(chip, channel))
        {
            return ::bsp::BSP_ERROR;
        }
        if (!writeChannelAttr(chip, channel, "period", periodNs))
        {
            return ::bsp::BSP_ERROR;
        }
        if (!writeChannelAttr(chip, channel, "duty_cycle", 0ULL))
        {
            return ::bsp::BSP_ERROR;
        }
        if (!writeChannelAttr(chip, channel, "enable", 1ULL))
        {
            return ::bsp::BSP_ERROR;
        }

        sChannels[i].chip     = chip;
        sChannels[i].channel  = channel;
        sChannels[i].periodNs = static_cast<uint32_t>(periodNs);
    }

    sNumChannels = n;
    sInitialized = true;
    return ::bsp::BSP_OK;
}

::bsp::BspReturnCode SysfsPwm::setDutyRaw(size_t const chanIdx, uint32_t const dutyNs)
{
    if ((!sInitialized) || (chanIdx >= sNumChannels))
    {
        return ::bsp::BSP_ERROR;
    }

    ChannelState const& ch = sChannels[chanIdx];
    if (!writeChannelAttr(ch.chip, ch.channel, "duty_cycle", static_cast<uint64_t>(dutyNs)))
    {
        return ::bsp::BSP_ERROR;
    }
    return ::bsp::BSP_OK;
}

::bsp::BspReturnCode SysfsPwm::setDutyPercent(size_t const chanIdx, uint16_t perMille)
{
    if ((!sInitialized) || (chanIdx >= sNumChannels))
    {
        return ::bsp::BSP_ERROR;
    }

    if (perMille > 1000U)
    {
        perMille = 1000U;
    }

    uint64_t const periodNs = static_cast<uint64_t>(sChannels[chanIdx].periodNs);
    uint32_t const dutyNs
        = static_cast<uint32_t>((periodNs * static_cast<uint64_t>(perMille)) / 1000ULL);

    return setDutyRaw(chanIdx, dutyNs);
}

::bsp::BspReturnCode SysfsPwm::setPulseMicroseconds(size_t const chanIdx, uint16_t const us)
{
    if ((!sInitialized) || (chanIdx >= sNumChannels))
    {
        return ::bsp::BSP_ERROR;
    }

    uint64_t const periodNs = static_cast<uint64_t>(sChannels[chanIdx].periodNs);
    uint64_t dutyNs         = static_cast<uint64_t>(us) * NS_PER_MICROSECOND;
    if (dutyNs > periodNs)
    {
        dutyNs = periodNs;
    }

    return setDutyRaw(chanIdx, static_cast<uint32_t>(dutyNs));
}

::bsp::BspReturnCode SysfsPwm::setServoAngle(
    size_t const chanIdx, uint8_t angle0to180, uint16_t const minUs, uint16_t const maxUs)
{
    if (maxUs < minUs)
    {
        return ::bsp::BSP_ERROR;
    }

    if (angle0to180 > 180U)
    {
        angle0to180 = 180U;
    }

    uint16_t const span = static_cast<uint16_t>(maxUs - minUs);
    uint16_t const us   = static_cast<uint16_t>(
        static_cast<uint32_t>(minUs)
        + ((static_cast<uint32_t>(span) * static_cast<uint32_t>(angle0to180)) / 180U));

    return setPulseMicroseconds(chanIdx, us);
}

} // namespace bsp
