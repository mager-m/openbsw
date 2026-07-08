# Main ESP-IDF component — links pre-built OpenBSW libraries.
#
# A stub source file is required by idf_component_register.
# The actual app_main() is in the OpenBSW libmain.a library.

idf_component_register(
    SRCS "stub.c"
    REQUIRES freertos driver nvs_flash esp_timer esp_system esp_adc
             esp_driver_uart esp_driver_gpio esp_driver_ledc esp_driver_twai
             esp_driver_spi esp_driver_rmt
    PRIV_REQUIRES spi_flash esp_vfs_console
)

# Path to pre-built OpenBSW libraries (from cmake --preset esp32-freertos)
set(BSW_BUILD "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../build/esp32-freertos")

# Strip FreeRTOS hook symbols that conflict with ESP-IDF's implementations.
# ESP-IDF's versions have correct buffer sizes for its modified FreeRTOS TCB.
find_program(OBJCOPY xtensa-esp32s3-elf-objcopy)
set(_STRIP_LIBS
    "${BSW_BUILD}/libs/bsw/asyncFreeRtos/RelWithDebInfo/libasyncFreeRtosImpl.a"
    "${BSW_BUILD}/executables/referenceApp/platforms/esp32/main/RelWithDebInfo/libosHooks.a"
)
set(_STRIP_SYMBOLS
    vApplicationGetIdleTaskMemory
    vApplicationGetTimerTaskMemory
    # NOTE: vApplicationIdleHook is NOT localized — we call it from our idle loop
    vApplicationStackOverflowHook
    vApplicationMallocFailedHook
)
foreach(_lib IN LISTS _STRIP_LIBS)
    if(EXISTS "${_lib}")
        set(_weaken_args "")
        foreach(_sym IN LISTS _STRIP_SYMBOLS)
            list(APPEND _weaken_args "--localize-symbol=${_sym}")
        endforeach()
        execute_process(
            COMMAND ${OBJCOPY} ${_weaken_args} "${_lib}"
            RESULT_VARIABLE _rc)
    endif()
endforeach()

# Collect all OpenBSW static libraries
set(BSW_LIBS
    # Platform main (libmain.a and libappReferenceApp.a are in --whole-archive above)
    "${BSW_BUILD}/executables/referenceApp/platforms/esp32/main/RelWithDebInfo/libosHooks.a"
    "${BSW_BUILD}/executables/referenceApp/platforms/esp32/bspConfiguration/RelWithDebInfo/libbspConfiguration.a"
    "${BSW_BUILD}/executables/referenceApp/configuration/RelWithDebInfo/libcommonImpl.a"
    "${BSW_BUILD}/executables/referenceApp/consoleCommands/RelWithDebInfo/libconsoleCommands.a"
    "${BSW_BUILD}/executables/referenceApp/safety/safeSupervisor/RelWithDebInfo/libsafeSupervisor.a"
    "${BSW_BUILD}/executables/referenceApp/safety/safeWatchdog/RelWithDebInfo/libsafeWatchdog.a"
    "${BSW_BUILD}/executables/referenceApp/platforms/esp32/safety/safeLifecycle/RelWithDebInfo/libsafeLifecycle.a"
    "${BSW_BUILD}/executables/referenceApp/udsConfiguration/RelWithDebInfo/libudsConfigurationImpl.a"
    # BSW libraries
    "${BSW_BUILD}/libs/bsw/asyncFreeRtos/RelWithDebInfo/libasyncFreeRtosImpl.a"
    "${BSW_BUILD}/libs/bsw/asyncConsole/RelWithDebInfo/libasyncConsole.a"
    "${BSW_BUILD}/libs/bsw/lifecycle/RelWithDebInfo/liblifecycle.a"
    "${BSW_BUILD}/libs/bsw/logger/RelWithDebInfo/liblogger.a"
    "${BSW_BUILD}/libs/bsw/loggerIntegration/RelWithDebInfo/libloggerIntegration.a"
    "${BSW_BUILD}/libs/bsw/runtime/RelWithDebInfo/libruntime.a"
    "${BSW_BUILD}/libs/bsw/stdioConsoleInput/RelWithDebInfo/libstdioConsoleInput.a"
    "${BSW_BUILD}/libs/bsw/storage/RelWithDebInfo/libstorage.a"
    "${BSW_BUILD}/libs/bsw/transport/RelWithDebInfo/libtransport.a"
    "${BSW_BUILD}/libs/bsw/transportRouterSimple/RelWithDebInfo/libtransportRouterSimple.a"
    "${BSW_BUILD}/libs/bsw/docan/RelWithDebInfo/libdocan.a"
    "${BSW_BUILD}/libs/bsw/uds/RelWithDebInfo/libuds.a"
    "${BSW_BUILD}/libs/bsw/util/RelWithDebInfo/libutil.a"
    "${BSW_BUILD}/libs/bsw/cpp2can/RelWithDebInfo/libcpp2can.a"
    # 3rd party
    "${BSW_BUILD}/libs/3rdparty/printf/lib/RelWithDebInfo/libprintf.a"
    # Platform BSP
    "${BSW_BUILD}/platforms/esp32/bsp/bspTwai/RelWithDebInfo/libbspTwai.a"
    "${BSW_BUILD}/platforms/esp32/bsp/bspMcp2515/RelWithDebInfo/libbspMcp2515.a"
    "${BSW_BUILD}/platforms/esp32/bsp/bspLedcPwm/RelWithDebInfo/libbspLedcPwm.a"
    "${BSW_BUILD}/platforms/esp32/bsp/bspWs2812/RelWithDebInfo/libbspWs2812.a"
    "${BSW_BUILD}/platforms/esp32/bsp/bspAdc/RelWithDebInfo/libbspAdc.a"
    "${BSW_BUILD}/platforms/esp32/bsp/bspEepromDriver/RelWithDebInfo/libbspEepromDriver.a"
    "${BSW_BUILD}/platforms/esp32/bsp/bspGpio/RelWithDebInfo/libbspGpio.a"
    "${BSW_BUILD}/platforms/esp32/bsp/bspInterruptsImpl/RelWithDebInfo/libbspInterruptsImpl.a"
    "${BSW_BUILD}/platforms/esp32/bsp/bspMcu/RelWithDebInfo/libbspMcu.a"
    "${BSW_BUILD}/platforms/esp32/bsp/bspSystemTime/RelWithDebInfo/libbspSystemTime.a"
    "${BSW_BUILD}/platforms/esp32/bsp/bspUart/RelWithDebInfo/libbspUart.a"
    "${BSW_BUILD}/platforms/esp32/etlImpl/RelWithDebInfo/libetlImpl.a"
    "${BSW_BUILD}/platforms/esp32/safety/safeBspMcuWatchdog/RelWithDebInfo/libsafeBspMcuWatchdog.a"
)

# Redirect vanilla FreeRTOS port symbols to our ESP-IDF wrappers
target_link_libraries(${COMPONENT_LIB} INTERFACE
    # ESP-IDF already starts the FreeRTOS scheduler before app_main().
    # OpenBSW's FreeRtosAdapter::run() calls vTaskStartScheduler() which
    # must be a no-op on ESP-IDF to avoid double-start.
    "-Wl,--wrap=vTaskStartScheduler"
    "-Wl,--wrap=xTaskGetIdleTaskHandle"
    "-Wl,--wrap=xTaskCreateStaticPinnedToCore"
    # Force-pull app_main symbol, let linker resolve the rest
    "-Wl,--undefined=app_main"
    "-Wl,--start-group"
    "${BSW_BUILD}/libappReferenceApp.a"
    "${BSW_BUILD}/executables/referenceApp/platforms/esp32/main/RelWithDebInfo/libmain.a"
    ${BSW_LIBS}
    "-Wl,--end-group"
)

# Build FreeRTOS API wrappers as a separate object and link AFTER OpenBSW
# to resolve symbols that ESP-IDF renames/macros away.
add_library(freertos_wrappers STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/freertos_wrappers.c")
target_link_libraries(${COMPONENT_LIB} INTERFACE freertos_wrappers)

# Explicitly link ESP-IDF component libraries AFTER OpenBSW to resolve
# driver references (uart, gpio, nvs, etc.) from BSP modules.
set(IDF_BUILD "${CMAKE_BINARY_DIR}/esp-idf")
target_link_libraries(${COMPONENT_LIB} INTERFACE
    "${IDF_BUILD}/esp_driver_uart/libesp_driver_uart.a"
    "${IDF_BUILD}/esp_driver_gpio/libesp_driver_gpio.a"
    "${IDF_BUILD}/esp_driver_ledc/libesp_driver_ledc.a"
    "${IDF_BUILD}/nvs_flash/libnvs_flash.a"
    "${IDF_BUILD}/esp_timer/libesp_timer.a"
    "${IDF_BUILD}/esp_system/libesp_system.a"
    "${IDF_BUILD}/esp_adc/libesp_adc.a"
    "${IDF_BUILD}/freertos/libfreertos.a"
    "${IDF_BUILD}/driver/libdriver.a"
    "${IDF_BUILD}/esp_driver_twai/libesp_driver_twai.a"
    "${IDF_BUILD}/esp_driver_spi/libesp_driver_spi.a"
    "${IDF_BUILD}/esp_driver_rmt/libesp_driver_rmt.a"
    "${IDF_BUILD}/hal/libhal.a"
    "${IDF_BUILD}/soc/libsoc.a"
    "${IDF_BUILD}/esp_hw_support/libesp_hw_support.a"
)
