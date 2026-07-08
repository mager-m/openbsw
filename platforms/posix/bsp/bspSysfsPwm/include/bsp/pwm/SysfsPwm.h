// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspSysfsPwm
 *
 * Sysfs-based hardware PWM driver for Linux hosts (Raspberry Pi).
 *
 * POSIX counterpart of the ESP32 bspLedcPwm module. Drives the kernel
 * hardware-PWM channels exposed under /sys/class/pwm/pwmchip{chip}/pwm{channel}
 * using nothing but plain file I/O (open/write): no external libraries, no
 * dynamic allocation, no exceptions, no RTTI. Supports multiple independent
 * channels, each carrying its own chip, channel and frequency. Adds
 * duty-percent, servo pulse-width and servo-angle helpers on top of raw duty
 * control.
 *
 * On a host without /sys/class/pwm (any non-Pi Linux box) every entry point
 * degrades gracefully: init() and the setters return BSP_ERROR instead of
 * asserting or throwing.
 *
 * Intended presets (documented here, supported but not mandated - the caller
 * supplies the ChannelConfig array):
 *
 *   SERVO  = 50 Hz.
 *            RC servo, one channel. Pulse width 544..2400 us (1000..2000 us
 *            nominal), 90 deg centre. Use setPulseMicroseconds() or
 *            setServoAngle().
 *
 *   MOTOR  = 1000 Hz.
 *            H-bridge motor, two channels. Use setDutyRaw() or setDutyPercent().
 *
 * Example (caller-owned) configuration arrays:
 * \code
 * static bsp::SysfsPwm::ChannelConfig const SERVO_CHANNELS[] = {
 *     {0U, 0U, 50U},   // /sys/class/pwm/pwmchip0/pwm0
 * };
 * static bsp::SysfsPwm::ChannelConfig const MOTOR_CHANNELS[] = {
 *     {0U, 0U, 1000U}, // channel A
 *     {0U, 1U, 1000U}, // channel B
 * };
 * \endcode
 */
#pragma once

#include "bsp/Bsp.h"

#include <cstddef>
#include <cstdint>

namespace bsp
{

class SysfsPwm
{
public:
    /**
     * Description of a single PWM output channel.
     *
     * Selects the sysfs node /sys/class/pwm/pwmchip{chip}/pwm{channel}.
     */
    struct ChannelConfig
    {
        /** PWM chip index (the N in pwmchipN). */
        uint8_t chip;
        /** Channel index within the chip (the N in pwmN). */
        uint8_t channel;
        /** Output frequency in Hz (e.g. 50 for a servo, 1000 for a motor). */
        uint32_t frequencyHz;
    };

    SysfsPwm()                           = delete;
    SysfsPwm(SysfsPwm const&)            = delete;
    SysfsPwm& operator=(SysfsPwm const&) = delete;

    /**
     * Export and configure the PWM channels described by \p channels.
     *
     * For each channel the channel index is written to the chip's \c export
     * attribute, then \c period is set to 1e9 / frequencyHz nanoseconds, the
     * initial \c duty_cycle is set to 0 and finally \c enable is set to 1. A
     * channel that is already exported reports EBUSY, which is treated as
     * success.
     *
     * \param channels array of channel descriptions (must remain valid only for
     *                 the duration of this call; the contents are copied).
     * \param n        number of entries in \p channels (0 < n <= MAX_CHANNELS).
     * \return BSP_OK on success, BSP_ERROR on invalid arguments or when a sysfs
     *         path cannot be opened or written.
     */
    static ::bsp::BspReturnCode init(ChannelConfig const* channels, size_t n);

    /**
     * Set the raw duty (high time) of a channel in nanoseconds.
     *
     * Writes \p dutyNs to the channel's \c duty_cycle attribute.
     *
     * \param chanIdx index into the array passed to init().
     * \param dutyNs  duty (high time) in nanoseconds (0 .. period).
     */
    static ::bsp::BspReturnCode setDutyRaw(size_t chanIdx, uint32_t dutyNs);

    /**
     * Set the duty as a fraction of the channel's period, in per-mille.
     *
     * \param chanIdx  index into the array passed to init().
     * \param perMille duty in tenths of a percent, 0..1000 (values above 1000
     *                 are clamped to 1000).
     */
    static ::bsp::BspReturnCode setDutyPercent(size_t chanIdx, uint16_t perMille);

    /**
     * Set the output high-time as an absolute pulse width in microseconds.
     *
     * Writes \c duty_cycle = us * 1000 ns, clamped to the channel's period.
     *
     * \param chanIdx index into the array passed to init().
     * \param us      desired pulse width in microseconds.
     */
    static ::bsp::BspReturnCode setPulseMicroseconds(size_t chanIdx, uint16_t us);

    /**
     * Convenience helper for RC-style servos: map an angle to a pulse width and
     * apply it via setPulseMicroseconds().
     *
     * \param chanIdx     index into the array passed to init().
     * \param angle0to180 servo angle in degrees, 0..180 (clamped to 180).
     * \param minUs       pulse width for 0 deg (default 544 us).
     * \param maxUs       pulse width for 180 deg (default 2400 us).
     */
    static ::bsp::BspReturnCode setServoAngle(
        size_t chanIdx,
        uint8_t angle0to180,
        uint16_t minUs = 544U,
        uint16_t maxUs = 2400U);

private:
    /** Maximum number of channels tracked by the driver. */
    static constexpr size_t MAX_CHANNELS = 8U;

    /** Per-channel state retained after init(). */
    struct ChannelState
    {
        uint8_t chip;
        uint8_t channel;
        uint32_t periodNs;
    };

    static ChannelState sChannels[MAX_CHANNELS];
    static size_t sNumChannels;
    static bool sInitialized;
};

} // namespace bsp
