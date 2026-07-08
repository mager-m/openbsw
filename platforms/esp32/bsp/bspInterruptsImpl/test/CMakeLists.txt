add_executable(bspInterruptsImplTestEsp32 src/IncludeTest.cpp)

target_include_directories(bspInterruptsImplTestEsp32
                           PRIVATE ../freertos/include)

target_link_libraries(bspInterruptsImplTestEsp32 PRIVATE platform gtest_main)

gtest_discover_tests(bspInterruptsImplTestEsp32 PROPERTIES LABELS
                                                           "bspInterruptsImplTest")
