#include "Allocator.hpp"
#include "Memory.hpp"
#include "Assert.hpp"
#include "Log.hpp"
#include <new>
/*
FObjectAllocationManager::FMemoryBlock::FMemoryBlock(uint32 InUsedSize, uint32 OffsetPrev, uint32 OffsetNext)
    : UsedSize{InUsedSize}
    , OffsetToPrev{OffsetPrev}
    , OffsetToNext{OffsetNext}
{
}

FObjectAllocationManager::FMemoryBlock* FObjectAllocationManager::FMemoryBlock::GetPrevBlock()
{
    if EXPECT(OffsetToPrev > 0, true)
    {
        return reinterpret_cast<FMemoryBlock*>(reinterpret_cast<uint8*>(this) - OffsetToPrev);
    }
    return nullptr;
}

FObjectAllocationManager::FMemoryBlock* FObjectAllocationManager::FMemoryBlock::GetNextBlock()
{
    if EXPECT(OffsetToNext > 0, true)
    {
        return reinterpret_cast<FMemoryBlock*>(reinterpret_cast<uint8*>(this) + OffsetToNext);
    }
    return nullptr;
}

uint32 FObjectAllocationManager::FMemoryBlock::AvailableSize() const
{
    return OffsetToNext - UsedSize;
}

FObjectAllocationManager::FObjectAllocationManager(const uint32 BytesToAllocate)
    : StartBlock{Memory::Allocate<FMemoryBlock>(BytesToAllocate)}
    , EndBlock{reinterpret_cast<FMemoryBlock*>(reinterpret_cast<uint8*>(StartBlock) + Memory::AllocatedSize(StartBlock))}
{
    const uint32 Offset{static_cast<uint32>(Memory::AllocatedSize(StartBlock) - sizeof(FMemoryBlock))};

    new(StartBlock) FMemoryBlock{0, 0, Offset};
    new(EndBlock) FMemoryBlock{0, Offset, 0};
}

FObjectAllocationManager::~FObjectAllocationManager()
{
    ASSERT(StartBlock->GetNextBlock() == EndBlock, "not all objects have been freed from the pool");
    Memory::Free(StartBlock);
}

uint8* FObjectAllocationManager::FindFreeMemory(const uint32 ObjectSize)
{
    const uint32 RequiredSize{static_cast<uint32>(sizeof(FMemoryBlock) + ObjectSize)};

    auto AllocateMoreFor = [this, RequiredSize, ObjectSize](FMemoryBlock* CurrentBlock) -> uint8*
    {
        const uint32 OldPoolSize{static_cast<uint32>(Memory::AllocatedSize(StartBlock))};
        const uint32 NewPoolSize{OldPoolSize + RequiredSize + ExtraAllocateSize};

        StartBlock = Memory::ReAllocate(StartBlock, NewPoolSize);

        uint8* OldEndBlockLocation{reinterpret_cast<uint8*>(StartBlock - 1) + OldPoolSize};
        uint8* NewEndBlockLocation{reinterpret_cast<uint8*>(StartBlock - 1) + NewPoolSize};
        const uint32 EndBlockDeltaSize{static_cast<uint32>(NewEndBlockLocation - OldEndBlockLocation)};

        EndBlock = new(NewEndBlockLocation) FMemoryBlock{0, EndBlockDeltaSize, 0};

        CurrentBlock->OffsetToNext = EndBlockDeltaSize;
        CurrentBlock->UsedSize = ObjectSize;

        return reinterpret_cast<uint8*>(CurrentBlock + 1);
    };

    auto CreateNewAfter = [ObjectSize](FMemoryBlock* CurrentBlock) -> uint8*
    {
        uint8* const NewBlockLocation{reinterpret_cast<uint8*>(CurrentBlock + 1) + CurrentBlock->UsedSize};

        const uint32 OffsetToNew{static_cast<uint32>(NewBlockLocation - reinterpret_cast<uint8*>(CurrentBlock))};
        const uint32 OffsetFromNewToNext{CurrentBlock->OffsetToNext - OffsetToNew};

        new(NewBlockLocation) FMemoryBlock{ObjectSize, OffsetToNew, OffsetFromNewToNext};

        CurrentBlock->OffsetToNext = OffsetToNew;

        return NewBlockLocation + sizeof(FMemoryBlock);
    };

    for(FMemoryBlock* MemoryBlock{StartBlock}; true; MemoryBlock = MemoryBlock->GetNextBlock())
    {
        if EXPECT(MemoryBlock->AvailableSize() >= RequiredSize, false)
        {
            return CreateNewAfter(MemoryBlock);
        }
        else if EXPECT(MemoryBlock->GetNextBlock() == nullptr, false)
        {
            return AllocateMoreFor(MemoryBlock);
        }
    }
}

void FObjectAllocationManager::FreeObject(void* ObjectToFree)
{
    FMemoryBlock* OwningBlock{static_cast<FMemoryBlock*>(ObjectToFree) - 1};

    FMemoryBlock* PrevBlock{OwningBlock->GetPrevBlock()};
    FMemoryBlock* NextBlock{OwningBlock->GetNextBlock()};

    PrevBlock->OffsetToNext += OwningBlock->OffsetToNext;
    NextBlock->OffsetToPrev += OwningBlock->OffsetToPrev;
}
*/
