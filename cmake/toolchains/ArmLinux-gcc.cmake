# Hosted-Linux (glibc) cross toolchain for the Raspberry Pi Zero.
#
# Target: BCM2835 / ARM1176JZF-S, armv6 hard-float (armhf).
# This is the POSIX platform's FreeRTOS-simulator build (pthreads/signals over
# glibc), NOT a bare-metal image, so it does NOT reuse the freestanding
# ArmNoneEabi flags: no -ffreestanding, no -fno-exceptions/-fno-rtti, no
# -static, no nano.specs/nosys.specs, and no REALTIME_OS define.
#
# Arch flags go through *_FLAGS_INIT so a preset's CMAKE_<LANG>_FLAGS
# (-Wall -Wextra ...) still applies on top instead of being overwritten.
#
# Optional PI_SYSROOT env var points CMAKE_SYSROOT at an rsync'd Pi OS root so
# link-time glibc matches the device.
include_guard(GLOBAL)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(_TRIPLE arm-linux-gnueabihf)
set(CMAKE_C_COMPILER   ${_TRIPLE}-gcc)
set(CMAKE_CXX_COMPILER ${_TRIPLE}-g++)
set(CMAKE_ASM_COMPILER ${_TRIPLE}-gcc)

# BCM2835 ARMv6 core + VFPv2, hard-float ABI (matches Raspberry Pi OS armhf).
set(_ARCH_FLAGS "-mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard")
set(CMAKE_C_FLAGS_INIT   "${_ARCH_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${_ARCH_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${_ARCH_FLAGS}")

# Optional sysroot rsync'd from the target Pi to pin glibc/headers.
if (DEFINED ENV{PI_SYSROOT})
    set(CMAKE_SYSROOT $ENV{PI_SYSROOT})
endif ()

# Find host programs on the host, but libraries/includes/packages only under
# the cross root/sysroot.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
