add_library(bspAdc src/Adc.cpp)

target_include_directories(bspAdc PUBLIC include)

target_link_libraries(bspAdc PRIVATE bsp espIdfHeaders)
