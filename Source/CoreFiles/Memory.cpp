#include "Memory.hpp"
#include <unistd.h>

namespace Memory::Cache
{
    const int64 L1LineSize{::sysconf(_SC_LEVEL1_DCACHE_LINESIZE)};
    const int64 L2LineSize{::sysconf(_SC_LEVEL2_CACHE_LINESIZE)};
    const int64 L3LineSize{::sysconf(_SC_LEVEL3_CACHE_LINESIZE)};
    const int64 L4LineSize{::sysconf(_SC_LEVEL4_CACHE_LINESIZE)};

    const int64 L1Size{::sysconf(_SC_LEVEL1_DCACHE_SIZE)};
    const int64 L2Size{::sysconf(_SC_LEVEL2_CACHE_SIZE)};
    const int64 L3Size{::sysconf(_SC_LEVEL3_CACHE_SIZE)};
    const int64 L4Size{::sysconf(_SC_LEVEL4_CACHE_SIZE)};
}
