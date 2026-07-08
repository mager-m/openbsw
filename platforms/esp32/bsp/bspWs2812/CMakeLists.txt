add_library(bspWs2812 src/Ws2812Strip.cpp)

target_include_directories(bspWs2812 PUBLIC include)

target_link_libraries(bspWs2812 PRIVATE bsp espIdfHeaders)
