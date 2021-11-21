#include "Allocator.hpp"
#include "Memory.hpp"
#include "Assert.hpp"
#include "Log.hpp"
#include <new>

FObjectAllocationManager::FMemoryBlock::FMemoryBlock(uint64 InUsedSize, uint64 InAvailableSize, FObjectAllocationManager::FMemoryBlock* InPrevBlock, FObjectAllocationManager::FMemoryBlock* InNextBlock)
    : UsedSize{InUsedSize}
    , AvailableSize{InAvailableSize}
    , PrevBlock{InPrevBlock}
    , NextBlock{InNextBlock}
{
}

OObject* FObjectAllocationManager::FMemoryBlock::GetAllocatedObject() const
{
    int64 ObjectOrNullAddress{reinterpret_cast<int64>(this + 1) * (UsedSize != 0)};
    return reinterpret_cast<OObject*>(ObjectOrNullAddress);
}

FObjectAllocationManager::FObjectAllocationManager()
    : ProgramBreakStart{static_cast<uint8*>(Memory::MoveProgramBreak(ArenaSize))}
    , ProgramBreakEnd{static_cast<uint8*>(Memory::GetProgramBreak())}
{
    FMemoryArena StartArena{};
    StartArena.StartBlock = Memory::NextAlignedAddress<FMemoryBlock>(ProgramBreakStart);
    StartArena.EndBlock = Memory::PrevAlignedAddress<FMemoryBlock>(ProgramBreakEnd - 1);

    new(StartArena.StartBlock) FMemoryBlock{0, static_cast<uint64>(StartArena.EndBlock - StartArena.StartBlock), nullptr, StartArena.EndBlock};
    new(StartArena.EndBlock) FMemoryBlock{0, 0, StartArena.StartBlock, nullptr};

    MemoryArenas.Append(StartArena);

    LOG(LogMemory, "created new arena at {} with size {}", reinterpret_cast<void*>(StartArena.StartBlock), ArenaSize);
}

FObjectAllocationManager::~FObjectAllocationManager()
{
    ASSERT(StartBlock()->NextBlock == EndBlock(), "not all objects have been freed from the pool");
}

uint8* FObjectAllocationManager::FindFreeMemory(uint64 ObjectSize)
{
    Thread::FSharedScopedLock SharedSearchLock{SharedMutex};

    const uint64 RequiredSize{static_cast<uint64>(sizeof(FMemoryBlock) + ObjectSize)};

    for(FMemoryBlock* MemoryBlock{StartBlock()}; true; MemoryBlock = MemoryBlock->NextBlock)
    {
        if EXPECT(MemoryBlock == EndBlock(), false)
        {
            MakeNewArena();
        }

        if(MemoryBlock->AvailableSize >= RequiredSize)
        {
            static Thread::FMutex CreateBlockMutex{};
            Thread::FScopedLock CreateBlockLock{CreateBlockMutex};

            if EXPECT(MemoryBlock->AvailableSize >= RequiredSize, true)
            {
                uint8* NewBlockLocation{reinterpret_cast<uint8*>(MemoryBlock + 1) + MemoryBlock->UsedSize};
                NewBlockLocation = Memory::NextAlignedAddress<uint8>(NewBlockLocation, alignof(FMemoryBlock));

                const uint8* NewObjectAlignedEnd{Memory::NextAlignedAddress<uint8>(NewBlockLocation + ObjectSize, alignof(FMemoryBlock))};
                const uint64 NewAvailableSize{reinterpret_cast<uint64>(MemoryBlock->NextBlock) - reinterpret_cast<uint64>(NewObjectAlignedEnd)};

                new(NewBlockLocation) FMemoryBlock{ObjectSize, NewAvailableSize, MemoryBlock, MemoryBlock->NextBlock};

                MemoryBlock->NextBlock = reinterpret_cast<FMemoryBlock*>(NewBlockLocation);
                MemoryBlock->AvailableSize = 0;

                return NewBlockLocation + sizeof(FMemoryBlock);
            }
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

    uint8* PrevBlockObjectAlignedEnd{reinterpret_cast<uint8*>(PrevBlock + 1) + PrevBlock->UsedSize};
    PrevBlockObjectAlignedEnd = Memory::NextAlignedAddress<uint8>(PrevBlockObjectAlignedEnd, alignof(FMemoryBlock));

    PrevBlock->AvailableSize = reinterpret_cast<uint64>(PrevBlock->NextBlock) - reinterpret_cast<uint64>(PrevBlockObjectAlignedEnd);

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
    NewArena.StartBlock = reinterpret_cast<FMemoryBlock*>(Memory::NextAlignedAddress<FMemoryBlock>(PreviousBreak));
    NewArena.EndBlock = reinterpret_cast<FMemoryBlock*>(Memory::PrevAlignedAddress<FMemoryBlock>(ProgramBreakEnd - sizeof(FMemoryBlock)));

    new(NewArena.StartBlock) FMemoryBlock{0, static_cast<uint64>(NewArena.EndBlock - NewArena.StartBlock), LastArena->EndBlock, NewArena.EndBlock};
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

const FObjectAllocationManager::FMemoryBlock* GetOwningObjectBlock(const OObject* const Object)
{
    uint64 BlockAddress{reinterpret_cast<uint64>(Object) - sizeof(FObjectAllocationManager::FMemoryBlock)};
    return reinterpret_cast<const FObjectAllocationManager::FMemoryBlock*>(BlockAddress);
}
