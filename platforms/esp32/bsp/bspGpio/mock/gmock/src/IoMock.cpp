// Copyright 2024 Accenture.

#include "bsp/IoMock.h"

namespace bios
{

static IoMock* sInstance = nullptr;

IoMock& IoMock::instance()
{
    if (sInstance == nullptr)
    {
        static IoMock mock;
        sInstance = &mock;
    }
    return *sInstance;
}

} // namespace bios
