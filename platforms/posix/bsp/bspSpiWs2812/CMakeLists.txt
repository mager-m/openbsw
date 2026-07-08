add_library(bspSpiWs2812 src/Ws2812Strip.cpp)

target_include_directories(bspSpiWs2812 PUBLIC include)

target_link_libraries(bspSpiWs2812 PUBLIC bsp)
