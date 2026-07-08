add_subdirectory(bspAdc)
add_subdirectory(bspEepromDriver)
add_subdirectory(bspGpio)
add_subdirectory(bspInterruptsImpl)
add_subdirectory(bspLedc)
add_subdirectory(bspMcu)
add_subdirectory(bspSystemTime)
add_subdirectory(bspTwai)
add_subdirectory(bspUart)
add_subdirectory(bspLedcPwm)
add_subdirectory(bspMcp2515)
add_subdirectory(bspWs2812)

add_library(socBsp INTERFACE)
target_link_libraries(
    socBsp
    INTERFACE bspAdc
              bspEepromDriver
              bspGpio
              bspInterruptsImpl
              bspMcu
              bspSystemTime
              bspUart
              safeBspMcuWatchdog)
