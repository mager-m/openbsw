add_executable(
    app.referenceApp
    src/app/app.cpp
    src/console/console.cpp
    src/logger/logger.cpp
    src/systems/DemoSystem.cpp
    src/systems/RuntimeSystem.cpp
    src/systems/SysAdminSystem.cpp
    src/systems/SafetySystem.cpp)

# ESP-IDF uses app_main() as entry point, which the ESP32 platform provides
# directly (with BSP init). Other platforms use the generic app_main() here.
if (NOT OPENBSW_PLATFORM STREQUAL "esp32")
    target_sources(app.referenceApp PRIVATE src/main.cpp)
endif ()

if (PLATFORM_SUPPORT_TRANSPORT)
    target_sources(app.referenceApp PRIVATE src/systems/TransportSystem.cpp)
    if (PLATFORM_SUPPORT_UDS)
        target_sources(app.referenceApp PRIVATE src/systems/UdsSystem.cpp
                                                src/uds/ReadIdentifierPot.cpp)
    endif ()
endif ()

if (PLATFORM_SUPPORT_ETHERNET)
    target_sources(app.referenceApp PRIVATE src/systems/EthernetSystem.cpp)
    if (PLATFORM_SUPPORT_TRANSPORT)
        target_sources(app.referenceApp
                       PRIVATE src/systems/DoIpServerSystem.cpp)
    endif ()
endif ()

if (PLATFORM_SUPPORT_CAN)
    target_sources(app.referenceApp PRIVATE src/app/CanDemoListener.cpp)
    if (PLATFORM_SUPPORT_TRANSPORT)
        target_sources(app.referenceApp PRIVATE src/systems/DoCanSystem.cpp)
    endif ()
endif ()

if (PLATFORM_SUPPORT_STORAGE)
    target_sources(app.referenceApp PRIVATE src/systems/StorageSystem.cpp)
endif ()

# BEGIN app node glue hook.
# Optional external application-node build wiring. If an app-glue root providing
# cmake/AppNodeGlue.cmake is available, it is included here so no
# application-specific paths are hardcoded in OpenBSW. It is discovered whether
# this repo sits beside the app repo (sibling checkout) or inside it (a submodule
# at <app>/external/openbsw), or it can be given explicitly with -DAPP_GLUE_ROOT.
if (PLATFORM_SUPPORT_CAN)
    if (NOT DEFINED APP_GLUE_ROOT)
        file(GLOB _app_glue_candidates
             "${CMAKE_SOURCE_DIR}/../*/cmake/AppNodeGlue.cmake"   # sibling checkout
             "${CMAKE_SOURCE_DIR}/../../cmake/AppNodeGlue.cmake") # submodule under <app>/external
        if (_app_glue_candidates)
            list(GET _app_glue_candidates 0 _app_glue_file)
            get_filename_component(_app_glue_dir "${_app_glue_file}" DIRECTORY)
            get_filename_component(APP_GLUE_ROOT "${_app_glue_dir}" DIRECTORY)
        endif ()
    endif ()
    if (APP_GLUE_ROOT AND EXISTS "${APP_GLUE_ROOT}/cmake/AppNodeGlue.cmake")
        set(APP_TARGET app.referenceApp)
        include("${APP_GLUE_ROOT}/cmake/AppNodeGlue.cmake")
    else ()
        # app.cpp references the application-node seam under PLATFORM_SUPPORT_CAN,
        # so the glue is required when CAN support is enabled.
        message(
            FATAL_ERROR
            "app node glue not found; "
            "pass -DAPP_GLUE_ROOT=<path to the app node checkout>")
    endif ()
endif ()
# END app node glue hook.

set_target_properties(app.referenceApp PROPERTIES SUFFIX ".elf")

if (PLATFORM_SUPPORT_ROM_CHECK)
    add_custom_command(
        TARGET app.referenceApp
        POST_BUILD
        COMMAND python3 ${OPENBSW_DIR}/tools/safety/crcCheck.py
        COMMENT
            "Running post-build script to run the CRC and inject the result into the ELF file"
    )
endif ()

if (TARGET startUp)
    get_target_property(LINKER_SCRIPT startUp PROP_LINKER_SCRIPT)
    if (DEFINED LINKER_SCRIPT)
        target_link_options(app.referenceApp PRIVATE -T ${LINKER_SCRIPT})
    endif ()
endif ()

target_include_directories(app.referenceApp PRIVATE include)

target_link_libraries(
    app.referenceApp
    PRIVATE asyncBinding
            asyncConsole
            bspMcu
            configuration
            etl
            etlImpl
            lifecycle
            consoleCommands
            logger
            printf
            loggerIntegration
            main
            stdioConsoleInput
            osRtos
            asyncRtosImpl
            util
            runtime
            safeLifecycle
            safeUtils
            safeWatchdog
            bspUart
            socBsp)

if (PLATFORM_SUPPORT_TRANSPORT)
    target_link_libraries(app.referenceApp PRIVATE transport
                                                   transportRouterSimple)
    if (PLATFORM_SUPPORT_UDS)
        target_link_libraries(app.referenceApp PRIVATE uds udsConfigurationImpl)
    endif ()
endif ()

if (PLATFORM_SUPPORT_CAN)
    target_link_libraries(app.referenceApp PRIVATE cpp2can)
    if (PLATFORM_SUPPORT_TRANSPORT)
        target_link_libraries(app.referenceApp PRIVATE docan)
    endif ()
endif ()

if (PLATFORM_SUPPORT_ETHERNET)
    target_link_libraries(app.referenceApp PRIVATE cpp2ethernet lwipSocket
                                                   lwipcore ethConfiguration)
    if (PLATFORM_SUPPORT_TRANSPORT)
        target_link_libraries(app.referenceApp PRIVATE doip)
    endif ()
endif ()

if (PLATFORM_SUPPORT_STORAGE)
    target_link_libraries(app.referenceApp PRIVATE storage)
endif ()

if (PLATFORM_SUPPORT_ROM_CHECK)
    target_link_libraries(app.referenceApp PRIVATE safeMemory)
endif ()

if (BUILD_RUST)
    target_compile_definitions(app.referenceApp PRIVATE BUILD_RUST)
    # Link against Rust static library(should be only one static library)
    target_link_libraries(app.referenceApp PRIVATE rust_hello_world)
    # Add include directory for Rust C/C++ headers
    target_include_directories(
        app.referenceApp
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../rustHelloWorld/include)
endif ()
