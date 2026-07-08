// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspLedcPwm
 *
 * LEDC-based PWM driver for ESP32-S3 (Arduino Nano ESP32).
 *
 * Supersedes the earlier, half-built bspLedc module: supports multiple
 * independent channels, each carrying its own GPIO, LEDC channel, LEDC timer,
 * frequency and duty resolution. Channels that share a timer are configured
 * once. Adds duty-percent, servo pulse-width and servo-angle helpers on top of
 * raw duty control.
 *
 * All operation is on LEDC_LOW_SPEED_MODE (the only speed mode available on the
 * ESP32-S3). No dynamic allocation, no exceptions, no RTTI.
 *
 * Two intended presets (documented here, supported but not mandated - the
 * caller supplies the ChannelConfig array):
 *
 *   SERVO  = 50 Hz, LEDC_TIMER_16_BIT resolution.
 *            RC servo, one channel. Pulse width 544..2400 us (1000..2000 us
 *            nominal), 90 deg centre. Use setPulseMicroseconds() or
 *            setServoAngle().
 *
 *   MOTOR  = 1000 Hz, LEDC_TIMER_8_BIT resolution.
 *            H-bridge motor, two channels, 8-bit duty. Use setDutyRaw() or
 *            setDutyPercent().
 *
 * Example (caller-owned) configuration arrays:
 * \code
 * static bsp::LedcPwm::ChannelConfig const SERVO_CHANNELS[] = {
 *     {D2, LEDC_CHANNEL_0, LEDC_TIMER_0, 50U,   LEDC_TIMER_16_BIT},
 * };
 * static bsp::LedcPwm::ChannelConfig const MOTOR_CHANNELS[] = {
 *     {D5, LEDC_CHANNEL_1, LEDC_TIMER_1, 1000U, LEDC_TIMER_8_BIT}, // channel A
 *     {D6, LEDC_CHANNEL_2, LEDC_TIMER_1, 1000U, LEDC_TIMER_8_BIT}, // channel B
 * };
 * \endcode
 */
#pragma once

#include "bsp/Bsp.h"

#include "driver/ledc.h"

#include <cstddef>
#include <cstdint>

namespace bsp
{

class LedcPwm
{
public:
    /**
     * Description of a single PWM output channel.
     *
     * \note Channels that reference the same \c timer must also request the
     *       same \c frequencyHz and \c resolution; the timer is configured from
     *       the first channel that uses it.
     */
    struct ChannelConfig
    {
        /** GPIO the PWM signal is routed to. */
        uint8_t gpioNum;
        /** LEDC channel (LEDC_CHANNEL_0 .. LEDC_CHANNEL_7). */
        ledc_channel_t channel;
        /** LEDC timer that drives this channel (LEDC_TIMER_0 .. LEDC_TIMER_3). */
        ledc_timer_t timer;
        /** Output frequency in Hz (e.g. 50 for a servo, 1000 for a motor). */
        uint32_t frequencyHz;
        /** Duty resolution in bits (e.g. LEDC_TIMER_16_BIT, LEDC_TIMER_8_BIT). */
        ledc_timer_bit_t resolution;
    };

    LedcPwm()                          = delete;
    LedcPwm(LedcPwm const&)            = delete;
    LedcPwm& operator=(LedcPwm const&) = delete;

    /**
     * Configure the LEDC timers and channels described by \p channels.
     *
     * Each distinct timer is configured once; every channel is then bound to
     * its GPIO with a starting duty of 0.
     *
     * \param channels array of channel descriptions (must remain valid only for
     *                 the duration of this call; the contents are copied).
     * \param n        number of entries in \p channels (0 < n <= MAX_CHANNELS).
     * \return BSP_OK on success, BSP_ERROR on invalid arguments or driver error.
     */
    static ::bsp::BspReturnCode init(ChannelConfig const* channels, size_t n);

    /**
     * Set the raw duty (in timer counts) of a channel and latch it.
     *
     * \param chanIdx index into the array passed to init().
     * \param duty    raw duty in counts (0 .. 2^resolution - 1).
     */
    static ::bsp::BspReturnCode setDutyRaw(size_t chanIdx, uint32_t duty);

    /**
     * Set the duty as a fraction of full scale, in per-mille.
     *
     * \param chanIdx index into the array passed to init().
     * \param perMille duty in tenths of a percent, 0..1000 (values above 1000
     *                 are clamped to 1000).
     */
    static ::bsp::BspReturnCode setDutyPercent(size_t chanIdx, uint16_t perMille);

    /**
     * Set the output high-time as an absolute pulse width in microseconds.
     *
     * Converts to raw counts using the channel's frequency and resolution:
     *   duty = us * 2^resolution * frequencyHz / 1e6
     * The result is clamped to the channel's maximum duty.
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
    /** LEDC low-speed channels available on the ESP32-S3. */
    static constexpr size_t MAX_CHANNELS = 8U;

    static ChannelConfig sChannels[MAX_CHANNELS];
    static size_t sNumChannels;
    static bool sInitialized;
};

} // namespace bsp
