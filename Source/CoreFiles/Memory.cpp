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

uint8* Memory::GetProgramBreak() noexcept
{
    return static_cast<uint8*>(::sbrk(0));
}

uint8* Memory::SetProgramBreak(uint8* NewBreakLocation) noexcept(false)
{
    uint8* PreviousBreak{GetProgramBreak()};

    if EXPECT(::brk(NewBreakLocation) == -1, false)
    {
        throw Error::Memory{"out of memory"};
    }

    LOG(LogMemory
        , "program break set to: {}, from previous break: {}, delta = {}"
        , reinterpret_cast<void*>(NewBreakLocation)
        , reinterpret_cast<void*>(PreviousBreak)
        , reinterpret_cast<int64>(NewBreakLocation) - reinterpret_cast<int64>(PreviousBreak));

    return PreviousBreak;
}

uint8* Memory::MoveProgramBreak(int64 MoveAmount) noexcept(false)
{
    uint8* PreviousBreak{static_cast<uint8*>(::sbrk(MoveAmount))};

    if EXPECT(PreviousBreak == reinterpret_cast<uint8*>(-1), false)
    {
        throw Error::Memory{"out of memory"};
    }

    LOG(LogMemory
        , "program break set to: {}, from previous break: {}, delta = {}"
        , reinterpret_cast<void*>(reinterpret_cast<int64>(PreviousBreak) + MoveAmount)
        , reinterpret_cast<void*>(PreviousBreak)
        , MoveAmount);

    return PreviousBreak;
}
