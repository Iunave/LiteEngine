#include "SmartPointer.hpp"
#include "Memory.hpp"
#include "Thread/Thread.hpp"
#include "Simd.hpp"
#include "Atomic.hpp"
#include "Math.hpp"
#include "String.hpp"

#include <new>

uint32 PtrPri::EndRefCountOffset{512};
PtrPri::FReferenceCounter* PtrPri::StartRefCountPtr{Memory::ZeroAllocate<PtrPri::FReferenceCounter>(PtrPri::EndRefCountOffset)};

class FFreeReferenceCounters
{
public:

    FFreeReferenceCounters()
        : ArrayCount{0}
        , ReverseCount{static_cast<uint32>(Array.Num())}
    {
    }

    void PlaceCounterOffset(uint32 NewOffset)
    {
        --ReverseCount;

        ReadArrayMutex.Lock();
        Array[ArrayCount] = NewOffset;
        ReadArrayMutex.Unlock();

        ++ArrayCount;
    }

    uint32 TakeCounterOffset()
    {
        --ArrayCount;

        ReadArrayMutex.Lock();
        uint32 Offset{Array[ArrayCount]};
        ReadArrayMutex.Unlock();

        ++ReverseCount;

        return Offset;
    }

private:

    TStaticArray<uint32, 64> Array;

    Thread::FSemaphore ArrayCount;
    Thread::FSemaphore ReverseCount;

    Thread::FMutex ReadArrayMutex;
};

FFreeReferenceCounters FreeRefCounters{};

namespace PtrPri
{
    void* PopulateFreeRefCounters(void*);
}

class FPopulateRefCountersManager
{
    friend PtrPri::FReferenceCounterWrapper;
    friend void* PtrPri::PopulateFreeRefCounters(void*);
public:

    FPopulateRefCountersManager()
        : bEndWork{false}
    {
        ASSERT(PtrPri::EndRefCountOffset % 16 == 0);

        SearchThread.Create(&PtrPri::PopulateFreeRefCounters, nullptr);
    }

    ~FPopulateRefCountersManager()
    {
        bEndWork = true;
        FreeRefCounters.TakeCounterOffset();

        SearchThread.Join();

        Memory::Free(PtrPri::StartRefCountPtr);
    }

private:

    Thread::FThread SearchThread;
    TAtomic<bool> bEndWork;
};

FPopulateRefCountersManager ThreadManager{};

namespace PtrPri
{
    void* PopulateFreeRefCounters(void*)
    {
#if defined(AVX512)
        using DataVectorType = int32_16;
#elif defined(AVX256)
        using DataVectorType = int32_8;
#endif

        constexpr auto AllocateMore = []() -> void
        {
            const uint64 OldSize{EndRefCountOffset * sizeof(PtrPri::FReferenceCounter)};
            const uint64 NewSize{OldSize + (512 * sizeof(PtrPri::FReferenceCounter))};
            ASSERT((NewSize % sizeof(PtrPri::FReferenceCounter)) == 0);

            StartRefCountPtr = Memory::ReAllocate(StartRefCountPtr, NewSize);

            FreeRefCounters.PlaceCounterOffset(EndRefCountOffset + 1);

            Memory::Set(StartRefCountPtr + EndRefCountOffset + 1, 0, NewSize - OldSize);

            EndRefCountOffset = (NewSize / sizeof(PtrPri::FReferenceCounter));
        };

        RestartSearch:

        if EXPECT(ThreadManager.bEndWork, false)
        {
            pthread_exit(nullptr);
        }

        for(uint32 Offset{0}; Offset != EndRefCountOffset; Offset += Simd::NumElements<DataVectorType>())
        {
            DataVectorType DataVector{Simd::LoadUnaligned<DataVectorType>(reinterpret_cast<const int32*>(StartRefCountPtr + Offset))};

            const Simd::MaskType<DataVectorType> ComparisonMask{Simd::MoveMask(DataVector == 0)};

            if EXPECT(ComparisonMask > 0, false)
            {
                const int32 MatchIndex{Math::FindFirstSet(ComparisonMask)};
                FreeRefCounters.PlaceCounterOffset(Offset + (MatchIndex - 1)); //ffs returns index + 1

                goto RestartSearch;
            }
        }

        AllocateMore();
        goto RestartSearch;
    }

    uint32 NewReferenceCounter()
    {
        const uint32 FreeOffset{FreeRefCounters.TakeCounterOffset()};
        new(StartRefCountPtr + FreeOffset) FReferenceCounter{};
        return FreeOffset;
    }
}
