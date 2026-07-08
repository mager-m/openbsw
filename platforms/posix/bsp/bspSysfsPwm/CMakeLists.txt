add_library(bspSysfsPwm src/SysfsPwm.cpp)
target_include_directories(bspSysfsPwm PUBLIC include)
target_link_libraries(bspSysfsPwm PUBLIC bsp)
