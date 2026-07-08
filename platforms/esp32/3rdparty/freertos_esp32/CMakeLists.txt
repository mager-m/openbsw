# FreeRTOS ESP32 port configuration
#
# ESP-IDF provides its own modified FreeRTOS with different struct sizes
# (dual-core support, TLS, etc.). OpenBSW MUST be compiled against ESP-IDF's
# FreeRTOS headers to ensure ABI compatibility at link time.
#
# This file creates:
#   - freeRtos          INTERFACE target (replacement for vanilla libs/3rdparty/freeRtos)
#   - freeRtosEsp32Port INTERFACE target (port-specific include dirs)
#   - freeRtosEsp32     INTERFACE target (ESP32 FreeRTOS port impl)
#
# The generated sdkconfig.h (from idf.py configure) lives in include/ and
# provides all CONFIG_* defines that ESP-IDF headers need.

set(_IDF "${IDF_PATH}/components")
set(_FREERTOS_INC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# --- freeRtosEsp32Port ---
add_library(freeRtosEsp32Port INTERFACE)
target_include_directories(freeRtosEsp32Port INTERFACE "${_FREERTOS_INC}")

# --- freeRtosEsp32 ---
add_library(freeRtosEsp32 INTERFACE)
target_link_libraries(freeRtosEsp32 INTERFACE bspInterrupts)

# --- freeRtos (INTERFACE replacement — no source compilation) ---
# Uses ESP-IDF's FreeRTOS headers. The actual kernel is linked via the
# ESP-IDF wrapper project at link time.
add_library(freeRtos INTERFACE)

target_include_directories(
    freeRtos
    INTERFACE
        # Our sdkconfig.h + FreeRTOSConfig.h overrides + portmacro.h
        "${_FREERTOS_INC}"
        # ESP-IDF FreeRTOS kernel headers (FreeRTOS.h, task.h, etc.)
        "${_IDF}/freertos/FreeRTOS-Kernel/include"
        # For #include <FreeRTOS.h> (without freertos/ prefix)
        "${_IDF}/freertos/FreeRTOS-Kernel/include/freertos"
        # ESP-IDF FreeRTOS config (FreeRTOSConfig.h base)
        "${_IDF}/freertos/config/include"
        "${_IDF}/freertos/config/include/freertos"
        "${_IDF}/freertos/config/xtensa/include"
        "${_IDF}/freertos/config/xtensa/include/freertos"
        # Xtensa port (portmacro.h from ESP-IDF)
        "${_IDF}/freertos/FreeRTOS-Kernel/portable/xtensa/include"
        "${_IDF}/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos"
        # ESP-IDF additions (idf_additions.h, etc.)
        "${_IDF}/freertos/esp_additions/include"
        "${_IDF}/freertos/esp_additions/include/freertos"
        # Xtensa architecture headers (xtensa_config.h, etc.)
        "${_IDF}/xtensa/include"
        "${_IDF}/xtensa/esp32s3/include"
        "${_IDF}/xtensa/deprecated_include"
        # ESP-IDF common (esp_attr.h, esp_compiler.h, etc.)
        "${_IDF}/esp_common/include"
        # ESP-IDF system (esp_system.h needed by some FreeRTOS additions)
        "${_IDF}/esp_system/include"
        # ESP-IDF HW support (spinlocks, soc caps)
        "${_IDF}/esp_hw_support/include"
        "${_IDF}/esp_hw_support/include/soc"
        "${_IDF}/esp_hw_support/include/soc/esp32s3"
        # SOC headers (soc_caps.h, etc.)
        "${_IDF}/soc/include"
        "${_IDF}/soc/esp32s3/include"
        "${_IDF}/soc/esp32s3/register"
        # HAL headers
        "${_IDF}/hal/include"
        "${_IDF}/hal/esp32s3/include"
        "${_IDF}/hal/platform_port/include"
        # Heap (needed by some ESP-IDF additions)
        "${_IDF}/heap/include"
        # Log (needed by esp_check.h)
        "${_IDF}/log/include"
        # ROM
        "${_IDF}/esp_rom/include"
        # Newlib platform
        "${_IDF}/newlib/platform_include"
)

target_link_libraries(
    freeRtos
    INTERFACE platform freeRtosConfiguration freeRtosPort)
