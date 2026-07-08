// Copyright 2024 Accenture.

#include "lifecycle/StaticBsp.h"

#include "bsp/Uart.h"
#include "bsp/uart/UartConfig.h"

#include <app/app.h>
#include <async/AsyncBinding.h>
#include <etl/alignment.h>
#include <etl/error_handler.h>
#include <lifecycle/LifecycleManager.h>
#include <safeSupervisor/SafeSupervisor.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#ifdef PLATFORM_SUPPORT_CAN
#include "systems/CanSystem.h"
#endif // PLATFORM_SUPPORT_CAN

extern void main_thread_setup(void);

void etl_assert_function(etl::exception const&) { std::abort(); }

namespace platform
{
StaticBsp staticBsp;

StaticBsp& getStaticBsp() { return staticBsp; }

#ifdef PLATFORM_SUPPORT_CAN
::etl::typed_storage<::systems::CanSystem> canSystem;
#endif // PLATFORM_SUPPORT_CAN

void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t const level)
{
    (void)lifecycleManager;
    if (level == 2)
    {
#ifdef PLATFORM_SUPPORT_CAN
        lifecycleManager.addComponent("can", canSystem.create(TASK_CAN), level);
#endif // PLATFORM_SUPPORT_CAN
    }
}

} // namespace platform

#ifdef PLATFORM_SUPPORT_CAN
namespace systems
{
::can::ICanSystem& getCanSystem() { return *::platform::canSystem; }
} // namespace systems
#endif // PLATFORM_SUPPORT_CAN

extern "C"
{

void putByteToStdout(uint8_t b)
{
    // Use ESP-IDF's stdout (routed to USB Serial/JTAG console)
    fputc(b, stdout);
}

int32_t getByteFromStdin()
{
    // Use ESP-IDF's stdin (routed from USB Serial/JTAG console).
    // stdin is non-blocking (configured in app_main), returns EOF if no data.
    int c = fgetc(stdin);
    if (c == EOF)
    {
        clearerr(stdin);
        return -1;
    }
    return static_cast<int32_t>(c);
}

void putchar_(char character) { putByteToStdout(static_cast<uint8_t>(character)); }

} // extern "C"

/**
 * ESP-IDF entry point. Called by the ESP-IDF FreeRTOS scheduler after
 * system initialization is complete.
 *
 * On ESP-IDF the entry point is app_main() (not main()). The generic
 * OpenBSW app_main() in application/src/main.cpp must be excluded from
 * the ESP32 build (see application/CMakeLists.txt) because this
 * platform-specific version replaces it.
 */
extern "C" void app_main()
{
    // Make stdin non-blocking so getByteFromStdin() doesn't block the idle loop
    setvbuf(stdin, NULL, _IONBF, 0);

    main_thread_setup();
    ::safety::safeSupervisorConstructor.construct();
    ::platform::staticBsp.init();

    // Inline what the generic app_main() does (etl error handler + app::run)
    etl::set_assert_function(etl_assert_function);
    ::app::run();
}
