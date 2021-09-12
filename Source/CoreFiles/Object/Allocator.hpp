#pragma once

#include "Definitions.hpp"
/*
class FObjectAllocationManager final : public FSingleton<FObjectAllocationManager>
{
public:

    static constexpr uint32 ExtraAllocateSize{1024};

    class PACKED FMemoryBlock
    {
    public:

        FMemoryBlock(uint32 InUsedSize, uint32 OffsetPrev, uint32 OffsetNext);

        FMemoryBlock* GetPrevBlock();

        FMemoryBlock* GetNextBlock();

        uint32 AvailableSize() const;

        uint32 UsedSize;

        uint32 OffsetToPrev;
        uint32 OffsetToNext;
    };

    FObjectAllocationManager(const uint32 BytesToAllocate = 500);

    ~FObjectAllocationManager();

    template<typename ObjectClass, typename... ConstructorArgs>
    ObjectClass* PlaceObject(ConstructorArgs&&... Arguments)
    {
        return new(FindFreeMemory(sizeof(ObjectClass))) ObjectClass{MoveIfPossible(Arguments)...};
    }

    //this function does not call the destructor of the object
    void FreeObject(void* ObjectToFree NONNULL);

    INLINE uint64 GetPoolSize() const
    {
        return (EndBlock - StartBlock) * sizeof(FMemoryBlock);
    }

    template<typename Functor>
    void ForEachObject(Functor Function)
    {
        for(FMemoryBlock* Block{StartBlock->GetNextBlock()}; Block != EndBlock; Block = Block->GetNextBlock())
        {
            Function(static_cast<void*>(Block + 1));
        }
    }

private:

    //returns a pointer to the end of a free memory block
    uint8* FindFreeMemory(const uint32 ObjectSize);

    FMemoryBlock* StartBlock;
    FMemoryBlock* EndBlock;
};
*/


