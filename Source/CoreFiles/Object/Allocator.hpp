#pragma once

#include "Definitions.hpp"
#include "TypeTraits.hpp"
#include "SmartPointer.hpp"

class OObject;

struct FMemoryBlock
{
    uint8* Data;
    uint64 BlockSize;

    FMemoryBlock* Next;
};

class FObjectAllocationManager final : public FSingleton<FObjectAllocationManager>
{
public:

    ~FObjectAllocationManager();

    template<typename ObjectClass, typename... ConstructorArgs>
    ObjectClass* PlaceObject(ConstructorArgs&&... Arguments)
    {
        return new(0) ObjectClass{MoveIfPossible(Arguments)...}; //todo
    }

private:

    FMemoryBlock FirstBlock;
    FMemoryBlock LastBlock;
    FMemoryBlock CursorBlock;
};

template<typename ObjectClass, typename... VarArgs> requires(TypeTrait::IsObjectClass<TypeTrait::RemovePointer<ObjectClass>>)
inline TSharedPtr<ObjectClass, ESPMode::Default> MakeShared(VarArgs&&... Args)
{
    return TSharedPtr<ObjectClass, ESPMode::Default>{FObjectAllocationManager::Instance().template PlaceObject<TypeTrait::RemovePointer<ObjectClass>>(MoveIfPossible(Args)...)};
}



