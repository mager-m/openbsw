# 3rdparty modules Configure — must come first (creates freeRtos target)
if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_subdirectory(3rdparty/freertos_esp32)
endif ()

# ESP-IDF component headers — INTERFACE library providing include paths
# for ESP-IDF driver and system headers used by the ESP32 BSP.
add_library(espIdfHeaders INTERFACE)

set(_IDF "${IDF_PATH}/components")

target_include_directories(
    espIdfHeaders
    SYSTEM INTERFACE
        # Our sdkconfig.h lives here
        "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/freertos_esp32/include"
        "${_IDF}/esp_common/include"
        "${_IDF}/esp_system/include"
        "${_IDF}/esp_hw_support/include"
        "${_IDF}/esp_hw_support/include/soc"
        "${_IDF}/esp_hw_support/include/soc/esp32s3"
        "${_IDF}/esp_rom/include"
        "${_IDF}/esp_rom/esp32s3"
        "${_IDF}/hal/include"
        "${_IDF}/hal/esp32s3/include"
        "${_IDF}/hal/platform_port/include"
        "${_IDF}/soc/include"
        "${_IDF}/soc/esp32s3/include"
        "${_IDF}/soc/esp32s3/register"
        "${_IDF}/heap/include"
        "${_IDF}/log/include"
        "${_IDF}/esp_timer/include"
        "${_IDF}/esp_driver_gpio/include"
        "${_IDF}/esp_driver_uart/include"
        "${_IDF}/esp_driver_ledc/include"
        "${_IDF}/esp_adc/include"
        "${_IDF}/esp_adc/esp32s3/include"
        "${_IDF}/nvs_flash/include"
        "${_IDF}/driver/twai/include"
        "${_IDF}/esp_driver_spi/include"
        "${_IDF}/esp_driver_rmt/include"
        "${_IDF}/esp_partition/include"
        "${_IDF}/spi_flash/include"
        # Xtensa architecture headers
        "${_IDF}/xtensa/include"
        "${_IDF}/xtensa/esp32s3/include"
        "${_IDF}/xtensa/deprecated_include"
)

# ESP-IDF driver headers include freertos/FreeRTOS.h
target_link_libraries(espIdfHeaders INTERFACE freeRtos)

# BSP modules
add_subdirectory(bsp)

# Safety modules
add_subdirectory(safety)

# ETL implementation
add_subdirectory(etlImpl)

# Other modules
add_subdirectory(lwipSysArch)
