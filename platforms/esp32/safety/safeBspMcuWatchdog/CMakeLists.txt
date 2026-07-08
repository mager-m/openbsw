add_library(
    safeBspMcuWatchdog
    src/SafeBspMcuWatchdog.cpp
    src/watchdog/Watchdog.cpp)

target_include_directories(safeBspMcuWatchdog PUBLIC include)

target_link_libraries(safeBspMcuWatchdog PRIVATE espIdfHeaders freeRtos)
