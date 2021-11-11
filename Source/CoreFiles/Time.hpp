#pragma once

#include "Definitions.hpp"

namespace Time
{
    inline uint64 ReadCycleCounter()
    {
        return __builtin_readcyclecounter();
    }

    float64 Now();

    float64 Local();
}
