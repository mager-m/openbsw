add_library(bspLedc src/LedcPwm.cpp)

target_include_directories(bspLedc PUBLIC include)

target_link_libraries(bspLedc PRIVATE bsp espIdfHeaders)
