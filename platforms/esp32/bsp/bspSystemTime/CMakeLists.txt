add_library(bspSystemTime src/bsp/time/systemTimer.cpp)

target_include_directories(bspSystemTime PUBLIC include)

target_link_libraries(bspSystemTime PRIVATE bsp platform espIdfHeaders)
