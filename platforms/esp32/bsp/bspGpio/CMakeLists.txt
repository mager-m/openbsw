add_library(bspGpio src/Io.cpp)

target_include_directories(bspGpio PUBLIC include)

target_link_libraries(bspGpio PRIVATE bsp espIdfHeaders)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    add_library(bspGpioMock mock/gmock/src/IoMock.cpp)
    target_include_directories(bspGpioMock PUBLIC mock/gmock/include)
    target_link_libraries(bspGpioMock PRIVATE gmock_main)
endif ()
