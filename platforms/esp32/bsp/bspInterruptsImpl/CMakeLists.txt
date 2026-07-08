if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    set(bspInterruptsImplName "bspInterruptsImpl")
else ()
    set(bspInterruptsImplName "bspInterruptsImplUt")
endif ()

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_library(
        ${bspInterruptsImplName}
        freertos/src/interrupts/suspendResumeAllInterrupts.cpp)

    target_include_directories(${bspInterruptsImplName}
                               PUBLIC freertos/include)

    target_link_libraries(${bspInterruptsImplName} PUBLIC bspInterrupts
                                                          freeRtos)
endif ()
