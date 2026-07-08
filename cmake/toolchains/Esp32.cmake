include_guard(GLOBAL)

# ESP32-S3 Xtensa LX7 cross-compilation toolchain
# Requires ESP-IDF to be installed with $IDF_PATH set.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR Xtensa)

set(ESP32_TARGET_TRIPLE xtensa-esp32s3-elf)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

add_compile_definitions(REALTIME_OS=1)

set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS "s;S")

# Determine IDF_PATH
if (NOT DEFINED IDF_PATH AND DEFINED ENV{IDF_PATH})
    set(IDF_PATH $ENV{IDF_PATH})
endif ()

if (NOT DEFINED IDF_PATH)
    message(
        FATAL_ERROR
            "IDF_PATH must be set. Install ESP-IDF and source export.sh")
endif ()

# ESP-IDF component include paths — needed for driver headers (gpio, uart,
# timer, etc.) and their transitive dependencies (esp_err.h, soc types, …).
set(_IDF_COMP "${IDF_PATH}/components")
set(ESP_IDF_INCLUDE_DIRS
    "${_IDF_COMP}/esp_common/include"
    "${_IDF_COMP}/esp_system/include"
    "${_IDF_COMP}/esp_hw_support/include"
    "${_IDF_COMP}/esp_hw_support/include/soc"
    "${_IDF_COMP}/esp_hw_support/include/soc/esp32s3"
    "${_IDF_COMP}/esp_rom/include"
    "${_IDF_COMP}/esp_rom/esp32s3"
    "${_IDF_COMP}/hal/include"
    "${_IDF_COMP}/hal/esp32s3/include"
    "${_IDF_COMP}/hal/platform_port/include"
    "${_IDF_COMP}/soc/include"
    "${_IDF_COMP}/soc/esp32s3/include"
    "${_IDF_COMP}/soc/esp32s3/register"
    "${_IDF_COMP}/heap/include"
    "${_IDF_COMP}/log/include"
    "${_IDF_COMP}/newlib/platform_include"
    "${_IDF_COMP}/esp_timer/include"
    "${_IDF_COMP}/esp_driver_gpio/include"
    "${_IDF_COMP}/esp_driver_uart/include"
    "${_IDF_COMP}/esp_driver_ledc/include"
    "${_IDF_COMP}/esp_adc/include"
    "${_IDF_COMP}/nvs_flash/include"
    "${_IDF_COMP}/driver/twai/include"
    "${_IDF_COMP}/esp_partition/include"
    "${_IDF_COMP}/spi_flash/include"
)

# Find the Xtensa toolchain from ESP-IDF tools
if (NOT DEFINED CMAKE_C_COMPILER AND NOT DEFINED ENV{CC})
    find_program(
        CMAKE_C_COMPILER
        NAMES ${ESP32_TARGET_TRIPLE}-gcc
        PATHS "${IDF_PATH}/tools" "$ENV{HOME}/.espressif/tools"
        PATH_SUFFIXES "bin"
        DOC "ESP32 C compiler")
    if (NOT CMAKE_C_COMPILER)
        message(FATAL_ERROR "Cannot find ${ESP32_TARGET_TRIPLE}-gcc")
    endif ()
endif ()

if (NOT DEFINED CMAKE_CXX_COMPILER AND NOT DEFINED ENV{CXX})
    find_program(
        CMAKE_CXX_COMPILER
        NAMES ${ESP32_TARGET_TRIPLE}-g++
        PATHS "${IDF_PATH}/tools" "$ENV{HOME}/.espressif/tools"
        PATH_SUFFIXES "bin"
        DOC "ESP32 C++ compiler")
endif ()

# Architecture flags for ESP32-S3 (Xtensa LX7)
set(_ARCH_FLAGS "-mlongcalls -fmessage-length=0")

set(_CC_CXX_COMMON
    "${_ARCH_FLAGS} \
    -fdata-sections \
    -ffunction-sections \
    -fno-builtin \
    -fno-common \
    -fstack-usage")

set(_C_FLAGS "${_CC_CXX_COMMON} -ffreestanding")

set(_CXX_FLAGS
    "${_CC_CXX_COMMON} \
    -fno-exceptions \
    -fno-rtti \
    -fno-threadsafe-statics \
    -fno-use-cxa-atexit")

set(_EXE_LINKER_FLAGS
    "${_ARCH_FLAGS} \
    -static \
    -Wl,--gc-sections \
    -Wl,-Map,application.map,--cref \
    -nostartfiles")

set(_ASM_FLAGS "-g -mlongcalls")

if (DEFINED CMAKE_CXX_FLAGS)
    string(FIND "${CMAKE_CXX_FLAGS}" "-mlongcalls" _TOOLCHAIN_FLAGS_FOUND)
    if (_TOOLCHAIN_FLAGS_FOUND EQUAL -1)
        set(CMAKE_CXX_FLAGS
            "${_CXX_FLAGS} ${CMAKE_CXX_FLAGS}"
            CACHE STRING "C++ flags" FORCE)
    endif ()
else ()
    set(CMAKE_CXX_FLAGS_INIT "${_CXX_FLAGS}")
endif ()

if (DEFINED CMAKE_C_FLAGS)
    string(FIND "${CMAKE_C_FLAGS}" "-mlongcalls" _TOOLCHAIN_FLAGS_FOUND)
    if (_TOOLCHAIN_FLAGS_FOUND EQUAL -1)
        set(CMAKE_C_FLAGS
            "${_C_FLAGS} ${CMAKE_C_FLAGS}"
            CACHE STRING "C flags" FORCE)
    endif ()
else ()
    set(CMAKE_C_FLAGS_INIT "${_C_FLAGS}")
endif ()

if (DEFINED CMAKE_ASM_FLAGS)
    if (NOT DEFINED _ASM_MERGED)
        set(_ASM_MERGED
            ON
            CACHE INTERNAL "")
        set(CMAKE_ASM_FLAGS
            "${_ASM_FLAGS} ${CMAKE_ASM_FLAGS}"
            CACHE STRING "ASM flags" FORCE)
    endif ()
else ()
    set(CMAKE_ASM_FLAGS_INIT "${_ASM_FLAGS}")
endif ()

if (DEFINED CMAKE_EXE_LINKER_FLAGS)
    string(FIND "${CMAKE_EXE_LINKER_FLAGS}" "-mlongcalls"
                _LINKER_FLAGS_FOUND)
    if (_LINKER_FLAGS_FOUND EQUAL -1)
        set(CMAKE_EXE_LINKER_FLAGS
            "${_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS}"
            CACHE STRING "linker flags" FORCE)
    endif ()
else ()
    set(CMAKE_EXE_LINKER_FLAGS_INIT "${_EXE_LINKER_FLAGS}")
endif ()
