#include "Allocator.hpp"
#include "Memory.hpp"
#include "Assert.hpp"
#include "Log.hpp"
#include <new>

FAllocationManager::FMemoryBlock::FMemoryBlock(uint32 InUsedSize, uint32 OffsetPrev, uint32 OffsetNext)
    : UsedSize{InUsedSize}
    , OffsetToPrev{OffsetPrev}
    , OffsetToNext{OffsetNext}
{
}

FAllocationManager::FMemoryBlock* FAllocationManager::FMemoryBlock::GetPrevBlock()
{
    if EXPECT(OffsetToPrev > 0, true)
    {
        return reinterpret_cast<FMemoryBlock*>(reinterpret_cast<uint8*>(this) - OffsetToPrev);
    }
    return nullptr;
}

FAllocationManager::FMemoryBlock* FAllocationManager::FMemoryBlock::GetNextBlock()
{
    if EXPECT(OffsetToNext > 0, true)
    {
        return reinterpret_cast<FMemoryBlock*>(reinterpret_cast<uint8*>(this) + OffsetToNext);
    }
    return nullptr;
}

OObject* FAllocationManager::FMemoryBlock::GetAllocatedObject()
{
    return reinterpret_cast<OObject*>(this + 1);
}

uint32 FAllocationManager::FMemoryBlock::AvailableSize() const
{
    return (OffsetToNext - UsedSize) - sizeof(FMemoryBlock);
}

FAllocationManager::FAllocationManager(const int64 BytesToAllocate)
    : ProgramBreakStart{Memory::MoveProgramBreak(BytesToAllocate)}
    , ProgramBreakEnd{Memory::GetProgramBreak()}
    , StartBlock{reinterpret_cast<FMemoryBlock*>(ProgramBreakStart)}
    , EndBlock{StartBlock + 1}
{
    new(StartBlock) FMemoryBlock{0, 0, sizeof(FMemoryBlock)};
    new(EndBlock) FMemoryBlock{0, sizeof(FMemoryBlock), 0};
}

FAllocationManager::~FAllocationManager()
{
    ASSERT(StartBlock->GetNextBlock() == EndBlock, "not all objects have been freed from the pool");
}

uint8* FAllocationManager::FindFreeMemory(const uint32 ObjectSize)
{
    const uint32 RequiredSize{static_cast<uint32>(sizeof(FMemoryBlock) + ObjectSize)};

    auto CreateNewAfter = [ObjectSize](FMemoryBlock* CurrentBlock) -> uint8*
    {
        uint8* const NewBlockLocation{reinterpret_cast<uint8*>(CurrentBlock + 1) + CurrentBlock->UsedSize};

        const uint32 OffsetToNew{static_cast<uint32>(NewBlockLocation - reinterpret_cast<uint8*>(CurrentBlock))};
        const uint32 OffsetFromNewToNext{CurrentBlock->OffsetToNext - OffsetToNew};

        new(NewBlockLocation) FMemoryBlock{ObjectSize, OffsetToNew, OffsetFromNewToNext};

        CurrentBlock->OffsetToNext = OffsetToNew;

        return NewBlockLocation + sizeof(FMemoryBlock);
    };

    auto MoveEndBlock = [this, &CreateNewAfter, RequiredSize]() -> uint8*
    {
        FMemoryBlock* BlockBeforeEnd{EndBlock->GetPrevBlock()};
        uint32 ExtraSizeNeeded{RequiredSize - BlockBeforeEnd->AvailableSize()};
        uint8* NewEndBlockLocation{reinterpret_cast<uint8*>(EndBlock) + ExtraSizeNeeded};

        if EXPECT(NewEndBlockLocation > (ProgramBreakEnd - sizeof(FMemoryBlock)), false)
        {
            try
            {
                Memory::MoveProgramBreak(Math::Max(ExtraSizeNeeded, ExtraAllocateSize));
            }
            catch(Error::Memory& OutOfMemoryError)
            {
                Defragmentate();

                BlockBeforeEnd = EndBlock->GetPrevBlock();
                ExtraSizeNeeded = RequiredSize - BlockBeforeEnd->AvailableSize();
                NewEndBlockLocation = reinterpret_cast<uint8*>(EndBlock) + ExtraSizeNeeded;

                Memory::SetProgramBreak(NewEndBlockLocation + sizeof(FMemoryBlock));
            }

            ProgramBreakEnd = Memory::GetProgramBreak();
        }

        BlockBeforeEnd->OffsetToNext = BlockBeforeEnd->UsedSize + sizeof(FMemoryBlock);

        EndBlock = reinterpret_cast<FMemoryBlock*>(NewEndBlockLocation);
        new(EndBlock) FMemoryBlock{0, sizeof(FMemoryBlock), 0};

        return CreateNewAfter(BlockBeforeEnd);
    };

    for(int64 ArenaIndex{0}; ArenaIndex < MemoryArenas.Num(); ++ArenaIndex)
    {
        FMemoryArena& Arena{MemoryArenas[ArenaIndex]};

        for(FMemoryBlock* MemoryBlock{Arena.EndBlock->GetPrevBlock()}; true; MemoryBlock = MemoryBlock->GetPrevBlock())
        {
            if EXPECT(MemoryBlock->AvailableSize() >= RequiredSize, false)
            {
                return CreateNewAfter(MemoryBlock);
            }

            if EXPECT(MemoryBlock == Arena.StartBlock && ArenaIndex == MemoryArenas.Num() - 1, false)
            {
                MakeNewArena();
                break;
            }
        }
    }
}

void FAllocationManager::FreeObject(OObject* ObjectToFree NONNULL)
{
    FMemoryBlock* OwningBlock{reinterpret_cast<FMemoryBlock*>(ObjectToFree) - 1};

    FMemoryBlock* PrevBlock{OwningBlock->GetPrevBlock()};
    FMemoryBlock* NextBlock{OwningBlock->GetNextBlock()};

    PrevBlock->OffsetToNext += OwningBlock->OffsetToNext;
    NextBlock->OffsetToPrev += OwningBlock->OffsetToPrev;
}

void FAllocationManager::Defragmentate()
{

}

FAllocationManager::FMemoryArena* FAllocationManager::MakeNewArena()
{
    FMemoryArena* PreviousArena{MemoryArenas.End()};

    uint8* PreviousBreak{Memory::MoveProgramBreak(ArenaSize)};
    ProgramBreakEnd = Memory::GetProgramBreak();

    const uint32 OffsetOldEndToNewStart{static_cast<uint32>(PreviousBreak - reinterpret_cast<uint8*>(EndBlock))};

    if(OffsetOldEndToNewStart == sizeof(FMemoryBlock)) //we can merge the arenas (dont need a new startblock)
    {
        EndBlock->OffsetToNext = ArenaSize - sizeof(FMemoryBlock);

        uint8* NewEndBlockLocation{ProgramBreakEnd - sizeof(FMemoryBlock)};
        new(NewEndBlockLocation) FMemoryBlock{0, EndBlock->OffsetToNext, 0};

        PreviousArena->EndBlock = reinterpret_cast<FMemoryBlock*>(NewEndBlockLocation);;
        EndBlock = PreviousArena->EndBlock;
    }
    else
    {
        EndBlock->OffsetToNext = OffsetOldEndToNewStart;

        FMemoryArena NewArena{};
        NewArena.StartBlock = reinterpret_cast<FMemoryBlock*>(PreviousBreak);
        NewArena.EndBlock = reinterpret_cast<FMemoryBlock*>(ProgramBreakEnd - sizeof(FMemoryBlock));

        new(NewArena.StartBlock) FMemoryBlock{0, OffsetOldEndToNewStart, ArenaSize - sizeof(FMemoryBlock)};
        new(NewArena.EndBlock) FMemoryBlock{0, ArenaSize - sizeof(FMemoryBlock), 0};

        EndBlock = NewArena.EndBlock;

        MemoryArenas.Append(NewArena);
    }

    return MemoryArenas.End();
}

