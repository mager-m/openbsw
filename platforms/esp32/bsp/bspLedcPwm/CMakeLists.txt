add_library(bspLedcPwm src/LedcPwm.cpp)

target_include_directories(bspLedcPwm PUBLIC include)

target_link_libraries(bspLedcPwm PRIVATE bsp espIdfHeaders)
