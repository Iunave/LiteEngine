#pragma once

#include "Definitions.hpp"
#include "Thread/Thread.hpp"
#include "TypeTraits.hpp"
#include "Array.hpp"

class OObject;

class FObjectAllocationManager final : public FSingleton<FObjectAllocationManager>
{
    template<typename ObjectClass> requires(TypeTrait::IsObjectClass<ObjectClass>)
    friend class TObjectIterator;
public:

    static constexpr int64 ArenaSize{100000000}; //1 gigabyte

    class FMemoryBlock
    {
    public:

        FMemoryBlock(uint64 InUsedSize, uint64 InAvailableSize, FMemoryBlock* InPrevBlock, FMemoryBlock* InNextBlock)
            : UsedSize{InUsedSize}
            , AvailableSize{InAvailableSize}
            , PrevBlock{InPrevBlock}
            , NextBlock{InNextBlock}
        {
        }

        OObject* GetAllocatedObject()
        {
            return reinterpret_cast<OObject*>(reinterpret_cast<int64>(this + 1) * (UsedSize != 0));
        }

        uint64 UsedSize;
        uint64 AvailableSize;

        FMemoryBlock* PrevBlock;
        FMemoryBlock* NextBlock;
    };

    struct FMemoryArena
    {
        FMemoryBlock* StartBlock;
        FMemoryBlock* EndBlock;
    };

    FObjectAllocationManager();

    ~FObjectAllocationManager();

    template<typename ObjectClass, typename... ConstructorArgs> requires(TypeTrait::IsObjectClass<ObjectClass>)
    ObjectClass* PlaceObject(ConstructorArgs&&... Arguments)
    {
        return new(FindFreeMemory(sizeof(ObjectClass))) ObjectClass{MoveIfPossible(Arguments)...};
    }

    //this function does not call the destructor of the object
    void FreeObject(OObject* ObjectToFree NONNULL);

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
    uint8* FindFreeMemory(const uint64 ObjectSize);

    void MakeNewArena();

    void RemoveEmptyArenas();

    uint8* const ProgramBreakStart;
    uint8* ProgramBreakEnd;

    TCountedArray<FMemoryArena, 64> MemoryArenas;

    Thread::FMutex ChangeMemoryMutex;
};



