// Copyright 2024 Accenture.

#include <etl/print.h>
#include <util/stream/BspStubs.h>

extern "C" void etl_putchar(int c) { putByteToStdout(static_cast<uint8_t>(c)); }
