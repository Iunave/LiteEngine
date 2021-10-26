#include "Allocator.hpp"
#include "Memory.hpp"
#include "Assert.hpp"
#include "Log.hpp"
#include <new>

FObjectAllocationManager::FObjectAllocationManager()
    : ProgramBreakStart{static_cast<uint8*>(Memory::MoveProgramBreak(ArenaSize))}
    , ProgramBreakEnd{static_cast<uint8*>(Memory::GetProgramBreak())}
{
    LOG(LogMemory, "creating new arena at {} with size {}", reinterpret_cast<void*>(ProgramBreakStart), ArenaSize);

    FMemoryArena StartArena{};
    StartArena.StartBlock = reinterpret_cast<FMemoryBlock*>(ProgramBreakStart);
    StartArena.EndBlock = reinterpret_cast<FMemoryBlock*>(ProgramBreakEnd) - 1;

    new(StartArena.StartBlock) FMemoryBlock{0, ArenaSize - sizeof(FMemoryBlock), nullptr, StartArena.EndBlock};
    new(StartArena.EndBlock) FMemoryBlock{0, 0, StartArena.StartBlock, nullptr};

    MemoryArenas.Append(StartArena);
}

FObjectAllocationManager::~FObjectAllocationManager()
{
    ASSERT(StartBlock()->NextBlock == EndBlock(), "not all objects have been freed from the pool");
}

uint8* FObjectAllocationManager::FindFreeMemory(const uint64 ObjectSize)
{
    const uint64 RequiredSize{static_cast<uint64>(sizeof(FMemoryBlock) + ObjectSize)};

    auto CreateNewAfter = [ObjectSize, RequiredSize](FMemoryBlock* CurrentBlock) -> uint8*
    {
        uint8* const NewBlockLocation{reinterpret_cast<uint8*>(CurrentBlock + 1) + CurrentBlock->UsedSize};

        new(NewBlockLocation) FMemoryBlock{ObjectSize, CurrentBlock->AvailableSize - RequiredSize, CurrentBlock, CurrentBlock->NextBlock};

        CurrentBlock->NextBlock = reinterpret_cast<FMemoryBlock*>(NewBlockLocation);
        CurrentBlock->AvailableSize = 0;

        return NewBlockLocation + sizeof(FMemoryBlock);
    };

    for(FMemoryBlock* MemoryBlock{StartBlock()}; true; MemoryBlock = MemoryBlock->NextBlock)
    {
        if(MemoryBlock->AvailableSize >= RequiredSize)
        {
            return CreateNewAfter(MemoryBlock);
        }

        if EXPECT(MemoryBlock == EndBlock(), false)
        {
            MakeNewArena();
        }
    }
    UNREACHABLE;
}

void FObjectAllocationManager::FreeObject(OObject* ObjectToFree NONNULL)
{
    FMemoryBlock* OwningBlock{reinterpret_cast<FMemoryBlock*>(ObjectToFree) - 1};

    FMemoryBlock* PrevBlock{OwningBlock->PrevBlock};
    FMemoryBlock* NextBlock{OwningBlock->NextBlock};

    PrevBlock->NextBlock = OwningBlock->NextBlock;
    PrevBlock->AvailableSize += OwningBlock->UsedSize + OwningBlock->AvailableSize + sizeof(FMemoryBlock);

    NextBlock->PrevBlock = OwningBlock->PrevBlock;
}

void FObjectAllocationManager::MakeNewArena()
{
    FMemoryArena* LastArena{MemoryArenas.End()};

    uint8* PreviousBreak{static_cast<uint8*>(Memory::MoveProgramBreak(ArenaSize))};
    ProgramBreakEnd = static_cast<uint8*>(Memory::GetProgramBreak());

    LOG(LogMemory, "creating new arena at {} with size {}", reinterpret_cast<void*>(PreviousBreak), ArenaSize);

    FMemoryArena NewArena{};
    NewArena.StartBlock = reinterpret_cast<FMemoryBlock*>(PreviousBreak);
    NewArena.EndBlock = reinterpret_cast<FMemoryBlock*>(ProgramBreakEnd - sizeof(FMemoryBlock));

    new(NewArena.StartBlock) FMemoryBlock{0, ArenaSize - sizeof(FMemoryBlock), LastArena->EndBlock, NewArena.EndBlock};
    new(NewArena.EndBlock) FMemoryBlock{0, 0, NewArena.StartBlock, nullptr};

    LastArena->EndBlock->NextBlock = NewArena.StartBlock;

    MemoryArenas.Append(NewArena);
}

void FObjectAllocationManager::RemoveEmptyArenas()
{
    while(MemoryArenas.Num() > 1)
    {
        FMemoryArena* ArenaToCheck{MemoryArenas.End()};

        if(ArenaToCheck->StartBlock->NextBlock == ArenaToCheck->EndBlock)
        {
            if(ProgramBreakEnd == reinterpret_cast<uint8*>(ArenaToCheck->EndBlock + 1))
            {
                FMemoryArena* PreviousArena{ArenaToCheck - 1};

                ProgramBreakEnd = reinterpret_cast<uint8*>(PreviousArena->EndBlock + 1);
                Memory::SetProgramBreak(ProgramBreakEnd);
            }

            LOG(LogMemory, "removing arena at {} with size {}", reinterpret_cast<void*>(MemoryArenas.End()), ArenaSize);

            MemoryArenas.RemoveAtSwap(MemoryArenas.Num() - 1);
        }
        else
        {
            break;
        }
    }
}

