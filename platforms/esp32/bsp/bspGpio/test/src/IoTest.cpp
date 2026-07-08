// Copyright 2024 Accenture.

#include "bsp/Io.h"

#include <gtest/gtest.h>

namespace
{

TEST(BspIoTest, DirectionEnumValues)
{
    EXPECT_EQ(bios::Io::_DISABLED, 0);
    EXPECT_EQ(bios::Io::_IN, 1);
    EXPECT_EQ(bios::Io::_OUT, 2);
    EXPECT_EQ(bios::Io::_IN_OUT, 3);
}

TEST(BspIoTest, PinConfigurationStructSize)
{
    bios::Io::PinConfiguration cfg = {0, 0, 0, 0};
    EXPECT_EQ(cfg.gpioNum, 0U);
    EXPECT_EQ(cfg.dir, 0U);
    EXPECT_EQ(cfg.pullUp, 0U);
    EXPECT_EQ(cfg.pullDown, 0U);
}

} // namespace
