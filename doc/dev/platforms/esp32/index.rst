.. _esp32_overview:

Arduino Nano ESP32
==================

Overview
--------

The Arduino Nano ESP32 uses an ESP32-S3 SoC (dual-core Xtensa LX7, 240 MHz,
8 MB flash, 8 MB PSRAM). OpenBSW runs on it with the full reference application:
lifecycle manager, console, logger, storage, UDS, and FreeRTOS task scheduling.

Console I/O goes through the built-in USB connection (USB Serial/JTAG).
No external UART adapter is needed.

Prerequisites
-------------

1. Install `ESP-IDF v5.5.x <https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/get-started/index.html>`_.

2. Set ``IDF_PATH`` to the ESP-IDF install directory.

3. Add ESP-IDF tools to PATH: ``cmake``, ``ninja``, ``xtensa-esp32s3-elf-gcc``,
   ``idf.py``, and the ESP-IDF Python environment. Verify::

       cmake --version
       ninja --version
       xtensa-esp32s3-elf-gcc --version

Build
-----

The build has two phases. Phase 1 cross-compiles OpenBSW into static libraries.
Phase 2 links them with ESP-IDF's runtime (FreeRTOS, drivers, bootloader) to
produce a flashable binary.

.. code-block:: bash

   # Phase 1: Compile OpenBSW (link failure at the end is expected)
   cmake --preset esp32-freertos
   cmake --build build/esp32-freertos --config RelWithDebInfo

   # Archive the application objects into a static library
   xtensa-esp32s3-elf-ar rcs build/esp32-freertos/libappReferenceApp.a \
     build/esp32-freertos/executables/referenceApp/application/CMakeFiles/\
     app.referenceApp.dir/RelWithDebInfo/src/app/*.obj \
     build/esp32-freertos/executables/referenceApp/application/CMakeFiles/\
     app.referenceApp.dir/RelWithDebInfo/src/console/console.cpp.obj \
     build/esp32-freertos/executables/referenceApp/application/CMakeFiles/\
     app.referenceApp.dir/RelWithDebInfo/src/logger/logger.cpp.obj \
     build/esp32-freertos/executables/referenceApp/application/CMakeFiles/\
     app.referenceApp.dir/RelWithDebInfo/src/systems/*.obj \
     build/esp32-freertos/executables/referenceApp/application/CMakeFiles/\
     app.referenceApp.dir/RelWithDebInfo/src/uds/*.obj

   # Phase 2: Build the ESP-IDF wrapper project
   cd executables/referenceApp/platforms/esp32/idf_project
   cmake -G Ninja -S . -B build \
     -DIDF_TARGET=esp32s3 \
     -DSDKCONFIG_DEFAULTS=sdkconfig.defaults \
     -DPYTHON_DEPS_CHECKED=1 \
     -DCCACHE_ENABLE=0
   cmake --build build

Output: ``idf_project/build/openbsw_esp32.bin``

Phase 1 always fails at the link step because ESP-IDF runtime symbols
(FreeRTOS kernel, UART driver, etc.) are only available in Phase 2.
All compilation succeeds; the link failure is expected.

Flash
-----

Put the board in bootloader mode: hold **B1**, plug in USB, release B1. Then::

   cd executables/referenceApp/platforms/esp32/idf_project
   idf.py -p COMx flash

Replace ``COMx`` with the port that appears in bootloader mode (typically
``VID:PID=303A:1001`` in Device Manager).

Press RESET after flashing.

Monitor
-------

::

   cd executables/referenceApp/platforms/esp32/idf_project
   idf.py -p COMx monitor

Press Enter to get the ``>`` prompt. Type ``help`` for available commands::

   > help
    help       - Show all commands
    lc         - lifecycle (reboot, poweroff, level)
    logger     - logger settings (level)
    stats      - statistics (cpu, stack, all)
    storage    - NVS storage testing (write, read, fill)

``Ctrl+]`` exits the monitor.

VS Code Tasks
-------------

Copy ``executables/referenceApp/platforms/esp32/vscode-tasks.json`` to
``.vscode/tasks.json`` in the project root. The tasks require ESP-IDF tools
in PATH. Update the COM port in the Flash and Monitor tasks to match your board.

Available tasks:

- **ESP32: Configure OpenBSW** — run once after clone
- **ESP32: Configure IDF Project** — run once after clone
- **ESP32: Build Only** — full compile (Phase 1 + archive + Phase 2)
- **ESP32: 4 - Flash** — flash to board (bootloader mode required)
- **ESP32: Monitor USB Console** — serial monitor via ``idf.py monitor``
- **ESP32: Full Build & Flash** — build + flash in one step
- **ESP32: Clean All** — remove build directories

How It Works
------------

OpenBSW cannot use ESP-IDF's build system (``idf.py``) directly because it has
its own CMake structure. Instead:

- Phase 1 compiles OpenBSW against ESP-IDF's FreeRTOS **headers** (for correct
  struct sizes like ``StaticTask_t``), producing ``.a`` static libraries.
- Phase 2 uses a thin ESP-IDF project (``idf_project/``) that links those
  libraries with ESP-IDF's compiled runtime.
- Glue code in ``idf_project/main/stub.c`` handles differences:

  - ``vTaskStartScheduler`` is wrapped (pass-through on ESP-IDF boot, triggers
    ``asyncInitialized`` on OpenBSW's call).
  - ``xTaskCreateStaticPinnedToCore`` is wrapped to use dynamic allocation
    (ESP-IDF rejects static stack buffers in ``.bss``).
  - FreeRTOS hook symbols from OpenBSW are localized via ``objcopy`` so
    ESP-IDF's versions take precedence.

The ``sdkconfig.h`` in ``platforms/esp32/3rdparty/freertos_esp32/include/`` is
generated from ESP-IDF and committed. It must match the ESP-IDF version used.
To regenerate: delete ``idf_project/sdkconfig``, re-run Phase 2 configure, copy
``idf_project/build/config/sdkconfig.h`` to that location.

Pin Configuration
-----------------

.. csv-table::
   :header: "Function", "GPIO", "Notes"
   :widths: 20, 10, 30

   "UART TX", "43", "Also available via USB"
   "UART RX", "44", "Also available via USB"
   "PWM", "5", "LEDC output (D2 header pin)"
   "CAN TX", "5", "Shared with PWM; needs external transceiver"
   "CAN RX", "6", "Needs external transceiver"
   "RGB LED", "48", "WS2812 — not driven (needs RMT driver)"
   "ADC", "1-4", "4 channels configured"

Feature Support
---------------

.. csv-table::
   :header: "Feature", "Status", "Notes"
   :widths: 20, 10, 30

   "GPIO", "Yes", "22 digital pins"
   "ADC", "Yes", "4 channels configured"
   "PWM", "Yes", "Via LEDC peripheral"
   "UART", "Yes", "Console via USB Serial/JTAG"
   "CAN", "Build only", "Via TWAI; needs external transceiver"
   "Storage", "Yes", "NVS-based EEPROM emulation (4 KB)"
   "Watchdog", "Yes", "ESP-IDF task watchdog"
   "Ethernet", "No", "No PHY on board"
