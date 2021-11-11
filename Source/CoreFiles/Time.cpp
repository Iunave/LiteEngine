#include "Time.hpp"
#include <time.h>

namespace Time
{
    float64 Now()
    {
        timespec Time;
        clock_gettime(CLOCK_MONOTONIC, &Time);

        float64 Seconds{static_cast<float64>(Time.tv_sec)};
        float64 NanoSeconds{static_cast<float64>(Time.tv_nsec) * 1.0_nano};

        return Seconds + NanoSeconds;
    }

    float64 Local()
    {
        timespec Time;
        clock_gettime(CLOCK_REALTIME, &Time);

        float64 Seconds{static_cast<float64>(Time.tv_sec)};
        float64 NanoSeconds{static_cast<float64>(Time.tv_nsec) * 1.0_nano};

        return Seconds + NanoSeconds;
    }
}
