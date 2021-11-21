#pragma once

#include "Definitions.hpp"
#include "Thread/Thread.hpp"
#include "TypeTraits.hpp"
#include "Array.hpp"

class OObject;

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



