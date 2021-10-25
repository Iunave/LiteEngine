#pragma once

#include "Definitions.hpp"
#include "Object.hpp"

class FAllocationManager final : public FSingleton<FAllocationManager>
{
    template<typename ObjectClass> requires(TypeTrait::IsObjectClass<ObjectClass>)
    friend class TObjectIterator;
public:

    static constexpr uint32 ExtraAllocateSize{1024};
    static constexpr int64 ArenaSize{100000000}; //1 gigabyte

    class PACKED FMemoryBlock
    {
    public:

        FMemoryBlock(uint32 InUsedSize, uint32 OffsetPrev, uint32 OffsetNext);

        FMemoryBlock* GetPrevBlock();

        FMemoryBlock* GetNextBlock();

        OObject* GetAllocatedObject();

        uint32 AvailableSize() const;

        uint32 UsedSize;

        uint32 OffsetToPrev;
        uint32 OffsetToNext;
    };

    struct FMemoryArena
    {
        FMemoryBlock* StartBlock;
        FMemoryBlock* EndBlock;
    };

    FAllocationManager(const int64 BytesToAllocate = 10000);

    ~FAllocationManager();

    template<typename ObjectClass, typename... ConstructorArgs> requires(TypeTrait::IsObjectClass<ObjectClass>)
    ObjectClass* PlaceData(ConstructorArgs&&... Arguments)
    {
        return new(FindFreeMemory(sizeof(ObjectClass))) ObjectClass{MoveIfPossible(Arguments)...};
    }

    //this function does not call the destructor of the object
    void FreeObject(OObject* ObjectToFree NONNULL);

    INLINE uint64 GetPoolSize() const
    {
        return (EndBlock - StartBlock) * sizeof(FMemoryBlock);
    }

    void Defragmentate();

private:

    //returns a pointer to the end of a free memory block
    uint8* FindFreeMemory(const uint32 ObjectSize);

    FMemoryArena* MakeNewArena();

    uint8* const ProgramBreakStart;
    uint8* ProgramBreakEnd;

    TCountedArray<FMemoryArena, 64> MemoryArenas;

    FMemoryBlock* const StartBlock; //the very first block, same as ObjectArenas[0].StartBlock
    FMemoryBlock* EndBlock;  //the very last block, same as ObjectArenas[ObjectArenas.Num() - 1].EndBlock
};

template<typename ObjectClass> requires(TypeTrait::IsObjectClass<ObjectClass>)
class TObjectIterator final
{
public:

    TObjectIterator()
        : BlockPtr{FAllocationManager::Instance().StartBlock}
        , ObjectPtr{ObjectCast<ObjectClass*>(BlockPtr->GetAllocatedObject())}
    {
        if(ObjectPtr == nullptr)
        {
            operator++();
        }
    }

    TObjectIterator& operator++()
    {
        BlockPtr = BlockPtr->GetNextBlock();

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

    NONNULL ObjectClass* operator->()
    {
        ASSUME(ObjectPtr != nullptr);
        return ObjectPtr;
    }

    ObjectClass& operator*()
    {
        ASSUME(ObjectPtr != nullptr);
        return *ObjectPtr;
    }

    explicit operator bool() const
    {
        return BlockPtr != FAllocationManager::Instance().EndBlock;
    }

private:

    FAllocationManager::FMemoryBlock* BlockPtr;
    ObjectClass* ObjectPtr;
};



