#pragma once

#include "Definitions.hpp"
#include "Thread/Thread.hpp"
#include "Atomic.hpp"
#include "TypeTraits.hpp"
#include "Math.hpp"
#include "Array.hpp"

class OObject;

class FObjectAllocationManager final : public FSingleton<FObjectAllocationManager>
{
    template<typename ObjectClass> requires TypeTrait::IsObjectClass<ObjectClass>
    friend class TObjectIterator;
public:

    static constexpr uint64 ArenaSize{1_mega};

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

        OObject* GetAllocatedObject() const
        {
            int64 ObjectOrNullAddress{reinterpret_cast<int64>(this + 1) * (UsedSize != 0)};
            return reinterpret_cast<OObject*>(ObjectOrNullAddress);
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

    template<typename ObjectClass, typename... ConstructorArgs> requires TypeTrait::IsObjectClass<ObjectClass>
    ObjectClass* PlaceObject(ConstructorArgs&&... Arguments)
    {
        return new(FindFreeMemory(sizeof(ObjectClass), alignof(ObjectClass))) ObjectClass{MoveIfPossible(Arguments)...};
    }

    //this function does not call the destructor of the object
    attr(nonnull(2)) void FreeObject(OObject* ObjectToFree);

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
    uint8* FindFreeMemory(const uint64 ObjectSize, const uint64 ObjectAlignment);

    void MakeNewArena();

    uint8* const ProgramBreakStart;
    uint8* ProgramBreakEnd;

    TCountedArray<FMemoryArena, 64> MemoryArenas;

    Thread::FSharedMutex SharedMutex;
};

template<typename ObjectClass> requires TypeTrait::IsObjectClass<ObjectClass>
class TObjectIterator final
{
public:

    TObjectIterator()
        : BlockPtr{FObjectAllocationManager::Instance().StartBlock()}
        , ObjectPtr{nullptr}
    {
        operator++();
    }

    TObjectIterator& operator++()
    {
        ASSUME(BlockPtr != nullptr);
        BlockPtr = BlockPtr->NextBlock;

        if EXPECT(operator bool(), true)
        {
            ObjectPtr = ObjectCast<ObjectClass*>(BlockPtr->GetAllocatedObject());

            if(ObjectPtr == nullptr)
            {
                return operator++();
            }
        }
        return *this;
    }

    attr(returns_nonnull) ObjectClass* operator->()
    {
        return ObjectPtr;
    }

    ObjectClass& operator*()
    {
        ASSUME(ObjectPtr != nullptr);
        return *ObjectPtr;
    }

    explicit operator bool() const
    {
        return BlockPtr != FObjectAllocationManager::Instance().EndBlock();
    }

private:

    FObjectAllocationManager::FMemoryBlock* BlockPtr;
    ObjectClass* ObjectPtr;
};

inline const FObjectAllocationManager::FMemoryBlock* GetOwningObjectBlock(const OObject* const Object)
{
    uint64 BlockAddress{reinterpret_cast<uint64>(Object) - sizeof(FObjectAllocationManager::FMemoryBlock)};
    return reinterpret_cast<const FObjectAllocationManager::FMemoryBlock*>(BlockAddress);
}



