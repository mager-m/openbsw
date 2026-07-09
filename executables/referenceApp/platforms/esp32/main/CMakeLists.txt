add_library(main src/main.cpp src/lifecycle/StaticBsp.cpp)

target_include_directories(main PUBLIC include)

target_link_libraries(
    main
    PRIVATE bspUart asyncBinding lifecycle safeSupervisor
            bspConfiguration etl freeRtos
    PUBLIC bspEepromDriver)

# main.cpp contains app_main() which calls app::run() — needs application headers
target_include_directories(
    main
    PRIVATE "${CMAKE_SOURCE_DIR}/executables/referenceApp/application/include")

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_library(osHooks src/osHooks/freertos/osHooks.cpp)
    target_link_libraries(osHooks PRIVATE freeRtos)
endif ()

if (PLATFORM_SUPPORT_CAN)
    target_sources(main PRIVATE src/systems/CanSystem.cpp)
    target_link_libraries(main PRIVATE configuration bspTwai bspMcp2515)
    # TWAI pins are build-configurable (see platforms/esp32/Options.cmake).
    target_compile_definitions(
        main PRIVATE TWAI_TX_PIN=${TWAI_TX_PIN} TWAI_RX_PIN=${TWAI_RX_PIN})
endif ()
