# ESP-IDF top-level project file for OpenBSW on Arduino Nano ESP32 (ESP32-S3).
#
# This project links the pre-built OpenBSW static libraries with ESP-IDF's
# runtime (FreeRTOS, drivers, bootloader, partition table).
#
# Build workflow:
#   1. cd <openbsw-root>
#      cmake --preset esp32-freertos
#      cmake --build build/esp32-freertos --config RelWithDebInfo  (link failure OK)
#   2. cd executables/referenceApp/platforms/esp32/idf_project
#      idf.py set-target esp32s3
#      idf.py build
#   3. idf.py -p COMx flash monitor

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(openbsw_esp32)
