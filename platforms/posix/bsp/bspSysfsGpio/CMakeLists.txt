add_library(bspSysfsGpio src/SysfsGpio.cpp)

target_include_directories(bspSysfsGpio PUBLIC include)

target_link_libraries(bspSysfsGpio PUBLIC bsp)
