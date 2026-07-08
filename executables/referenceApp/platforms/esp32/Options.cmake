set(OPENBSW_PLATFORM esp32)

set(PLATFORM_SUPPORT_IO
    OFF
    CACHE BOOL "Turn IO support on or off (needs inputConfiguration/outputConfiguration)" FORCE)
set(PLATFORM_SUPPORT_CAN
    ON
    CACHE BOOL "Turn CAN support on or off (requires external transceiver)" FORCE)
set(PLATFORM_SUPPORT_ETHERNET
    OFF
    CACHE BOOL "Turn ethernet support on or off" FORCE)
set(PLATFORM_SUPPORT_TRANSPORT
    ON
    CACHE BOOL "Turn TRANSPORT support on or off" FORCE)
set(PLATFORM_SUPPORT_UDS
    ON
    CACHE BOOL "Turn UDS support on or off" FORCE)
set(PLATFORM_SUPPORT_WATCHDOG
    ON
    CACHE BOOL "Turn ON Watchdog support" FORCE)
set(PLATFORM_SUPPORT_MPU
    OFF
    CACHE BOOL "Turn OFF MPU support" FORCE)
set(PLATFORM_SUPPORT_STORAGE
    ON
    CACHE BOOL "Turn persistent storage on or off" FORCE)
set(PLATFORM_SUPPORT_ROM_CHECK
    OFF
    CACHE BOOL "Turn ON ROM check support" FORCE)
