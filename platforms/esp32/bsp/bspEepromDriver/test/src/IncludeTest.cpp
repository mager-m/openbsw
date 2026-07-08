// Copyright 2024 Accenture.

#include "eeprom/EepromDriver.h"

#include <gtest/gtest.h>

namespace
{

TEST(BspEepromDriverIncludeTest, IncludeCompiles) { SUCCEED(); }

TEST(BspEepromDriverIncludeTest, EepromSizeIs4K)
{
    EXPECT_EQ(::eeprom::EepromDriver::EEPROM_SIZE, 4096U);
}

} // namespace
