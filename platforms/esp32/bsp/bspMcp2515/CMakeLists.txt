add_library(bspMcp2515 src/Mcp2515CanTransceiver.cpp)

target_include_directories(bspMcp2515 PUBLIC include)

target_link_libraries(bspMcp2515 PUBLIC cpp2can PRIVATE bsp espIdfHeaders)
