add_library(bspConfiguration src/bsp/uart/UartConfig.cpp)

target_include_directories(bspConfiguration PUBLIC include)

target_link_libraries(bspConfiguration PRIVATE bsp)

# Include bspUart headers for UartConfig.cpp (avoid circular dependency)
target_include_directories(
    bspConfiguration
    PRIVATE "${CMAKE_SOURCE_DIR}/platforms/esp32/bsp/bspUart/include")
