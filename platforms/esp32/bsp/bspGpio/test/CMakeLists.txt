add_executable(bspGpioTest src/GpioIncludeTest.cpp src/IoTest.cpp)

target_include_directories(bspGpioTest PRIVATE ../include)

target_link_libraries(bspGpioTest PRIVATE platform gtest_main)

gtest_discover_tests(bspGpioTest PROPERTIES LABELS "bspGpioTest")
