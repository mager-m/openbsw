// Copyright 2024 Accenture.

/**
 * \file
 * \ingroup bspSysfsGpio
 *
 * Digital GPIO driver for Linux hosts via the sysfs interface
 * (/sys/class/gpio). POSIX counterpart to the ESP32 GPIO driver.
 *
 * This is the Raspberry Pi digital-output path for the H-bridge enable and
 * direction lines driven by the Engine node. Each configured pin is exported
 * through /sys/class/gpio/export, its direction is set to "out" or "in", and
 * output pins are latched to a caller-supplied initial level.
 *
 * The driver uses only plain POSIX file I/O (open/read/write/close); it does
 * not depend on libgpiod or any other external library. No dynamic
 * allocation, no exceptions, no RTTI.
 *
 * It is safe to build and run on a host without /sys/class/gpio (for example a
 * developer workstation). In that case every operation degrades gracefully:
 * any path that cannot be opened or written yields BSP_ERROR rather than a
 * crash, an assert or an exception.
 *
 * Example (caller-owned) configuration array:
 * \code
 * static bsp::SysfsGpio::PinConfig const ENGINE_PINS[] = {
 *     {17U, true,  false}, // R_EN, output, start low
 *     {27U, true,  false}, // L_EN, output, start low
 *     {22U, false, false}, // fault input
 * };
 * \endcode
 */
#pragma once

#include "bsp/Bsp.h"

#include <cstddef>
#include <cstdint>

namespace bsp
{

class SysfsGpio
{
public:
    /**
     * Description of a single GPIO line.
     */
    struct PinConfig
    {
        /** Linux GPIO number (the value written to /sys/class/gpio/export). */
        uint32_t gpioNum;
        /** true configures the line as an output, false as an input. */
        bool output;
        /** Initial level for an output line (ignored for inputs). */
        bool initialHigh;
    };

    SysfsGpio()                            = delete;
    SysfsGpio(SysfsGpio const&)            = delete;
    SysfsGpio& operator=(SysfsGpio const&) = delete;

    /** Upper bound on the number of pins the driver can track. */
    static constexpr size_t MAX_PINS = 16U;

    /**
     * Export and configure the GPIO lines described by \p pins.
     *
     * For every pin this exports the line, sets its direction, and (for
     * outputs) writes the initial level. A line that is already exported
     * (export reports EBUSY) is treated as success.
     *
     * The pin-to-index mapping is recorded before any hardware access, so
     * set() and get() remain index-safe even if the underlying sysfs access
     * fails (for example on a non-Pi host).
     *
     * \param pins array of pin descriptions (read during this call only).
     * \param n    number of entries in \p pins (0 < n <= MAX_PINS).
     * \return BSP_OK if every pin was configured, BSP_ERROR on invalid
     *         arguments or if any sysfs access failed.
     */
    static ::bsp::BspReturnCode init(PinConfig const* pins, size_t n);

    /**
     * Drive an output line high or low.
     *
     * \param idx  index into the array passed to init().
     * \param high true drives the line high, false drives it low.
     * \return BSP_OK on success, BSP_ERROR on an invalid index, an input
     *         line, or a sysfs write failure.
     */
    static ::bsp::BspReturnCode set(size_t idx, bool high);

    /**
     * Read the current level of a line (input or output).
     *
     * \param idx  index into the array passed to init().
     * \param high set to the read level on success (true = high).
     * \return BSP_OK on success, BSP_ERROR on an invalid index or a sysfs
     *         read failure.
     */
    static ::bsp::BspReturnCode get(size_t idx, bool& high);

private:
    static uint32_t sGpioNum[MAX_PINS];
    static bool sIsOutput[MAX_PINS];
    static size_t sNumPins;
    static bool sInitialized;
};

} // namespace bsp
