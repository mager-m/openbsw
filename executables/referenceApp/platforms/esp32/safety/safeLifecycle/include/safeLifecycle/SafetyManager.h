// Copyright 2024 Accenture.

#pragma once

namespace safety
{

class SafetyManager
{
public:
    SafetyManager() = default;
    void init();
    void run();
    void shutdown();
    void cyclic();
};

} // namespace safety
