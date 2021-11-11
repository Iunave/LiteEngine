#include "Allocator.hpp"
#include "Memory.hpp"
#include "Assert.hpp"
#include "Log.hpp"
#include <new>

FObjectAllocationManager::FObjectAllocationManager()
    : ProgramBreakStart{static_cast<uint8*>(Memory::MoveProgramBreak(ArenaSize))}
    , ProgramBreakEnd{static_cast<uint8*>(Memory::GetProgramBreak())}
{
    FMemoryArena StartArena{};
    StartArena.StartBlock = Memory::NextAlignedAddress<FMemoryBlock>(ProgramBreakStart);
    StartArena.EndBlock = Memory::PrevAlignedAddress<FMemoryBlock>(ProgramBreakEnd - 1);

    new(StartArena.StartBlock) FMemoryBlock{0, ArenaSize - sizeof(FMemoryBlock), nullptr, StartArena.EndBlock};
    new(StartArena.EndBlock) FMemoryBlock{0, 0, StartArena.StartBlock, nullptr};

    MemoryArenas.Append(StartArena);

    LOG(LogMemory, "created new arena at {} with size {}", reinterpret_cast<void*>(StartArena.StartBlock), ArenaSize);
}

FObjectAllocationManager::~FObjectAllocationManager()
{
    ASSERT(StartBlock()->NextBlock == EndBlock(), "not all objects have been freed from the pool");
}

uint8* FObjectAllocationManager::FindFreeMemory(const uint64 ObjectSize, const uint64 ObjectAlignment)
{
    Thread::FSharedScopedLock SharedSearchLock{SharedMutex};

    const uint64 RequiredSize{static_cast<uint64>(sizeof(FMemoryBlock) + ObjectSize)};

    for(FMemoryBlock* MemoryBlock{StartBlock()}; true; MemoryBlock = MemoryBlock->NextBlock)
    {
        if EXPECT(MemoryBlock->AvailableSize >= RequiredSize, false)
        {
            static Thread::FMutex CreateBlockMutex{};
            Thread::FScopedLock CreateBlockLock{CreateBlockMutex};

            if EXPECT(MemoryBlock->AvailableSize >= RequiredSize, true)
            {
                uint8* const NewBlockLocation{reinterpret_cast<uint8*>(MemoryBlock + 1) + MemoryBlock->UsedSize};

                new(NewBlockLocation) FMemoryBlock{ObjectSize, MemoryBlock->AvailableSize - RequiredSize, MemoryBlock, MemoryBlock->NextBlock};

                MemoryBlock->NextBlock = reinterpret_cast<FMemoryBlock*>(NewBlockLocation);
                MemoryBlock->AvailableSize = 0;

                return NewBlockLocation + sizeof(FMemoryBlock);
            }
        }

        if EXPECT(MemoryBlock == EndBlock(), false)
        {
            MakeNewArena();
        }
    }
}

void FObjectAllocationManager::FreeObject(OObject* ObjectToFree)
{
    Thread::FScopedLock Lock{SharedMutex};

    FMemoryBlock* OwningBlock{reinterpret_cast<FMemoryBlock*>(ObjectToFree) - 1};

    FMemoryBlock* PrevBlock{OwningBlock->PrevBlock};
    FMemoryBlock* NextBlock{OwningBlock->NextBlock};

    PrevBlock->NextBlock = OwningBlock->NextBlock;
    PrevBlock->AvailableSize += OwningBlock->UsedSize + OwningBlock->AvailableSize + sizeof(FMemoryBlock);

    NextBlock->PrevBlock = OwningBlock->PrevBlock;
}

void FObjectAllocationManager::MakeNewArena()
{
    Thread::FScopedLock Lock{SharedMutex};

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
    Thread::FScopedLock Lock{SharedMutex};

    while(MemoryArenas.Num() > 1)
    {
        FMemoryArena* ArenaToCheck{MemoryArenas.End()};

        if(ArenaToCheck->StartBlock->NextBlock == ArenaToCheck->EndBlock)
        {
            ProgramBreakEnd = static_cast<uint8*>(Memory::GetProgramBreak());
            uint8* ArenaEndAddress{reinterpret_cast<uint8*>(ArenaToCheck->EndBlock + 1)};

            if(ProgramBreakEnd == ArenaEndAddress)
            {
                FMemoryArena* PreviousArena{ArenaToCheck - 1};
                PreviousArena->EndBlock->NextBlock = nullptr;

                ProgramBreakEnd = ArenaEndAddress;
                Memory::SetProgramBreak(ArenaEndAddress);
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

