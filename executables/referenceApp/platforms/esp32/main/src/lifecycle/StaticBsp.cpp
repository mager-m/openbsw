// Copyright 2024 Accenture.

#include "lifecycle/StaticBsp.h"

#include "bsp/Uart.h"
#include "bsp/uart/UartConfig.h"

void StaticBsp::init()
{
    _eepromDriver.init();
    ::bsp::Uart::getInstance(::bsp::Uart::Id::TERMINAL).init();
}
