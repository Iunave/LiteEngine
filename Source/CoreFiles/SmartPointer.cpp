#include "SmartPointer.hpp"
#include "Memory.hpp"
#include "Thread/Thread.hpp"
#include "Simd.hpp"
#include "Atomic.hpp"
#include "Math.hpp"

uint64 PtrPri::EndRefCountOffset{512};
PtrPri::FReferenceCounter* PtrPri::StartRefCountPtr{Memory::ZeroAllocate<PtrPri::FReferenceCounter>(PtrPri::EndRefCountOffset)};

uint64 PtrPri::FindNewRefCounterOffset()
{
    #if defined(AVX512)
    using DataVectorType = int32_16;
    #elif defined(AVX256)
    using DataVectorType = int32_8;
    #endif

    static Thread::FMutex SearchMutex{};

    auto AllocateMore = []() -> void
    {
        if(SearchMutex.TryLock())
        {
            const uint64 OldSize{EndRefCountOffset * sizeof(PtrPri::FReferenceCounter)};
            const uint64 NewSize{OldSize + (512 * sizeof(PtrPri::FReferenceCounter))};
            ASSERT((NewSize % sizeof(PtrPri::FReferenceCounter)) == 0);

            StartRefCountPtr = Memory::Reallocate(StartRefCountPtr, NewSize);

            Memory::Set(StartRefCountPtr + EndRefCountOffset + 1, 0, NewSize - OldSize);

            EndRefCountOffset = (NewSize / sizeof(PtrPri::FReferenceCounter));

            SearchMutex.Unlock();
        }
    };

    RestartSearch:
    for(uint64 Offset{0}; Offset != EndRefCountOffset; Offset += Simd::NumElements<DataVectorType>())
    {
        DataVectorType DataVector{Simd::LoadUnaligned<DataVectorType>(reinterpret_cast<const int32*>(StartRefCountPtr + Offset))};
        Simd::MaskType<DataVectorType> ComparisonMask{Simd::MoveMask(DataVector == 0)};

        if EXPECT(ComparisonMask > 0, false)
        {
            uint64 FoundOffset{(Math::FindFirstSet(ComparisonMask) - 1) + Offset};

            if EXPECT(SearchMutex.TryLock(), true)
            {
                FReferenceCounter* FoundCounter{StartRefCountPtr + FoundOffset};
                FoundCounter->StrongReferenceCount = 1;
                FoundCounter->WeakReferenceCount = 1;

                SearchMutex.Unlock();

                return FoundOffset;
            }
        }
    }

    AllocateMore();
    goto RestartSearch;
}
