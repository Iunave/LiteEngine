#pragma once

#include "Definitions.hpp"
#include "Thread/Thread.hpp"
#include "TypeTraits.hpp"
#include "Array.hpp"
#include "Simd.hpp"

#include <immintrin.h>
#include <x86intrin.h>

class OObject;

class MAY_ALIAS FMemoryBlock
{
public:

    inline constexpr FMemoryBlock(void* InMemoryStart, void* InMemoryEnd)
        : MemoryStart{InMemoryStart}
        , MemoryEnd{InMemoryEnd}
    {
    }

    inline uint64 UsedSize() const
    {
        return static_cast<uint8*>(MemoryEnd) - static_cast<uint8*>(MemoryStart);
    }

    inline uint64 FreeSize() const
    {
        return static_cast<uint8*>(this[1].MemoryStart) - static_cast<uint8*>(MemoryEnd);
    }

    void* MemoryStart;
    void* MemoryEnd;
};

class FAllocationArray final
{
public:

    FMemoryBlock* InsertAtPushBack(int64 AtIndex, void* InMemoryStart, void* InMemoryEnd)
    {
        const uint64 NumBytesToMove{sizeof(FMemoryBlock) * (TerminatorIndex - AtIndex)};
        Memory::Move(&ElementPtr[AtIndex + 1], &ElementPtr[AtIndex], NumBytesToMove);

        ++TerminatorIndex;

        return ElementPtr + AtIndex;
    }

    FMemoryBlock* operator+(int64 Index)
    {
        return ElementPtr + Index;
    }

    FMemoryBlock& operator[](int64 Index)
    {
        return ElementPtr[Index];
    }

    int64 Num() const
    {
        return TerminatorIndex + 1;
    }

    FMemoryBlock* ElementPtr;
    int64 TerminatorIndex;
};

class FAllocationManager final
{
public:

    #ifdef AVX256
    using AllocationVectorType = uint64_4;
    #elif defined(AVX512)
    using AllocationVectorType = uint64_8;
    #endif

    static inline constexpr uint64 AllocationSize{128UL};

    FAllocationManager()
    {
        uint8* ProgramBreak{static_cast<uint8*>(Memory::GetProgramBreak())};
        uint8* AlignedBreak{Memory::NextAlignedAddress<uint8>(ProgramBreak, alignof(AllocationVectorType))};

        uint8* NewBreak{AlignedBreak + (AllocationSize * sizeof(FMemoryBlock))};
        Memory::SetProgramBreak(NewBreak);

        Allocations.ElementPtr = reinterpret_cast<FMemoryBlock*>(AlignedBreak);
        Allocations.TerminatorIndex = 0;

        Allocations[0].MemoryStart = AlignedBreak;
        Allocations[0].MemoryEnd = NewBreak;

        Memory::Set(Allocations.ElementPtr + 1, 0, AllocationSize * sizeof(FMemoryBlock));
    }

    FMemoryBlock* FindAllocation(uint64 SourcePtr)
    {
        const uint64_4 SourceVector{Simd::SetAll<uint64_4>(SourcePtr)};

        for(int64 Index{0}; Index < Allocations.Num(); void())
        {
            int32 ComparisonMask{0};

            #pragma unroll AllocationSize
            for(uint64 Iteration{0}; Iteration < AllocationSize; ++Iteration, Index += Simd::NumElements<uint64_4>())
            {
                uint64_4 VectorLow{Simd::LoadAligned<uint64_4>(reinterpret_cast<uint64*>(Allocations + Index))};
                Index += Simd::NumElements<uint64_4>();
                uint64_4 VectorHigh{Simd::LoadUnaligned<uint64_4>(reinterpret_cast<uint64*>(Allocations + Index))};

                uint64_4 CombinedVector{Simd::SelectElements<0, 1, 0, 1>(VectorLow, VectorHigh)};

                ComparisonMask |= Simd::MoveMask(SourceVector == CombinedVector);
            }

            if(ComparisonMask != 0)
            {
                const int64 FoundIndex{(Math::FindFirstSet(ComparisonMask) - 1) + Index};
                return Allocations + FoundIndex;
            }
        }

        return nullptr;
    }

    void* Allocate(uint64 Size, uint64 Align)
    {
        const int64_4 NeededSizeVector{Simd::SetAll<int64_4>(Size)};
        const int64_4 AlignVector{Simd::SetAll<int64_4>(Align)};

        for(int64 Index{0}; Index < Allocations.Num(); Index += Simd::NumElements<uint64_4>())
        {
            #if true
            Index += Simd::NumElements<uint64_4>();

            int64_4 MemoryStarts{Simd::Gather(Allocations + Index, int32_4{0, 2, 4, 6})};
            int64_4 MemoryEnds{Simd::Gather(Allocations + Index, int32_4{1, 3, 5, 7})};

            MemoryStarts = Simd::NextAlignedAddress(MemoryStarts, AlignVector);

            int64_4 MemorySize{MemoryEnds - MemoryStarts};

            int32 EnoughSizeMask{Simd::MoveMask(MemorySize >= NeededSizeVector)};

            if EXPECT(EnoughSizeMask != 0, false)
            {
                int32 FoundIndex{Math::FindFirstSet(EnoughSizeMask) - 1};
                FoundIndex <<= 1;
            }
            #else
            int64_4 VectorLow{Simd::LoadAligned<int64_4>(reinterpret_cast<int64*>(Allocations + Index))}; //0123

            Index += Simd::NumElements<uint64_4>();

            int64_4 VectorHigh{Simd::LoadAligned<int64_4>(reinterpret_cast<int64*>(Allocations + Index))}; //4567
            VectorHigh = Simd::ShuffleInLane<1, 0, 1, 0>(VectorHigh); //5476

            int64_4 MemoryStarts{Simd::SelectElements<0, 1, 0, 1>(VectorLow, VectorHigh)}; //0426
            MemoryStarts = Simd::NextAlignedAddress(MemoryStarts, AlignVector);

            int64_4 MemoryEnds{Simd::SelectElements<1, 0, 1, 0>(VectorLow, VectorHigh)}; //5173
            MemoryEnds = Simd::ShuffleInLane<1, 0, 1, 0>(MemoryEnds); //1537

            int64_4 MemorySize{MemoryEnds - MemoryStarts};

            int32 EnoughSizeMask{Simd::MoveMask(MemorySize >= NeededSizeVector)};

            if EXPECT(EnoughSizeMask != 0, false)
            {
                int32 FoundIndex{Math::FindFirstSet(EnoughSizeMask) - 1};
                FoundIndex += 3 * (FoundIndex % 2 != 0); // 0 = 0; 1 = 4; 2 = 2; 3 = 6;
            }
            #endif
        }
    }

    FAllocationArray Allocations;
};

class FObjectAllocationManager final
{
    template<typename ObjectClass> requires(TypeTrait::IsObjectClass<ObjectClass>)
    friend class TObjectIterator;
public:

    static constexpr uint64 ArenaSize{1_mega};

    static inline FObjectAllocationManager& Instance()
    {
        static FObjectAllocationManager Instance{};
        return Instance;
    }

    class FMemoryBlock
    {
    public:

        FMemoryBlock(uint64 InUsedSize, uint64 InAvailableSize, FMemoryBlock* InPrevBlock, FMemoryBlock* InNextBlock);

        OObject* GetAllocatedObject() const;

        uint64 UsedSize;
        uint64 AvailableSize;

        FMemoryBlock* PrevBlock;
        FMemoryBlock* NextBlock;
    };

    static_assert(alignof(FMemoryBlock) == 8);

    struct FMemoryArena
    {
        FMemoryBlock* StartBlock;
        FMemoryBlock* EndBlock;
    };

private:

    FObjectAllocationManager();
    ~FObjectAllocationManager();

public:

    template<typename ObjectClass, typename... ConstructorArgs> requires TypeTrait::IsObjectClass<ObjectClass>
    HOT ObjectClass* PlaceObject(ConstructorArgs&&... Arguments)
    {
        return new(FindFreeMemory(sizeof(ObjectClass))) ObjectClass{MoveIfPossible(Arguments)...};
    }

    //this function does not call the destructor of the object
    HOT void FreeObject(OObject* ObjectToFree);

    void RemoveEmptyArenas();

private:

    FMemoryBlock* StartBlock() const
    {
        return MemoryArenas.Start()->StartBlock;
    }

    FMemoryBlock* EndBlock() const
    {
        return MemoryArenas.End()->EndBlock;
    }

    //returns a pointer to the end of a free memory block
    HOT uint8* FindFreeMemory(uint64 ObjectSize);

    void MakeNewArena();

    uint8* const ProgramBreakStart;
    uint8* ProgramBreakEnd;

    TCountedArray<FMemoryArena, 64> MemoryArenas;

    Thread::FSharedMutex SharedMutex;
};

template<typename ObjectClass> requires(TypeTrait::IsObjectClass<ObjectClass>)
class TObjectIterator final
{
public:

    TObjectIterator()
        : BlockPtr{AllocationManager.StartBlock()}
        , ObjectPtr{nullptr}
    {
        operator++();
    }

    TObjectIterator& operator++()
    {
        do
        {
            BlockPtr = BlockPtr->NextBlock;
            ObjectPtr = ObjectCast<ObjectClass*>(BlockPtr->GetAllocatedObject());
        }
        while(ObjectPtr == nullptr && BlockPtr != AllocationManager.EndBlock());

        return *this;
    }

    ObjectClass* operator->()
    {
        return ObjectPtr;
    }

    ObjectClass& operator*()
    {
        return *ObjectPtr;
    }

    explicit operator bool() const
    {
        return BlockPtr != AllocationManager.EndBlock();
    }

private:

    static inline const FObjectAllocationManager& AllocationManager{FObjectAllocationManager::Instance()};

    FObjectAllocationManager::FMemoryBlock* BlockPtr;
    ObjectClass* ObjectPtr;
};

const FObjectAllocationManager::FMemoryBlock* GetOwningObjectBlock(const OObject* const Object);



