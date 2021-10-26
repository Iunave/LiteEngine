#include "Memory.hpp"
#include "Assert.hpp"
#include "Log.hpp"
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

void* Memory::GetProgramBreak()
{
    return ::sbrk(0);
}

void* Memory::SetProgramBreak(void* NewBreakLocation) THROWS
{
    void* PreviousBreak{GetProgramBreak()};

    if EXPECT(::brk(NewBreakLocation) == -1, false)
    {
        throw Error::Memory{"out of memory"};
    }

    LOG(LogMemory, "program break set to: {}, from previous break: {}, delta = {}", NewBreakLocation, PreviousBreak, reinterpret_cast<int64>(NewBreakLocation) - reinterpret_cast<int64>(PreviousBreak));

    return PreviousBreak;
}

void* Memory::MoveProgramBreak(int64 MoveAmount) THROWS
{
    void* PreviousBreak{::sbrk(MoveAmount)};

    if EXPECT(PreviousBreak == reinterpret_cast<void*>(-1), false)
    {
        throw Error::Memory{"out of memory"};
    }

    LOG(LogMemory, "program break set to: {}, from previous break: {}, delta = {}", reinterpret_cast<void*>(reinterpret_cast<int64>(PreviousBreak) + MoveAmount), PreviousBreak, MoveAmount);

    return PreviousBreak;
}
