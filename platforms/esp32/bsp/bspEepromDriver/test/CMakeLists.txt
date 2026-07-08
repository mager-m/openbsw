add_executable(bspEepromDriverTestEsp32 src/IncludeTest.cpp)

target_include_directories(bspEepromDriverTestEsp32 PRIVATE ../include)

target_link_libraries(bspEepromDriverTestEsp32 PRIVATE platform gtest_main
                                                       bsp)

gtest_discover_tests(bspEepromDriverTestEsp32 PROPERTIES LABELS
                                                         "bspEepromDriverTest")
