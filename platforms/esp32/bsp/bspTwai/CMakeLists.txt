add_library(bspTwai src/TwaiCanTransceiver.cpp)

target_include_directories(bspTwai PUBLIC include)

target_link_libraries(bspTwai PUBLIC cpp2can PRIVATE bsp espIdfHeaders)
