#include "Allocator.hpp"
#include "Memory.hpp"

FObjectAllocationManager::~FObjectAllocationManager()
{
    /*
    if(AllocationBegin != nullptr)
    {
        Memory::UnMap(AllocationBegin, AllocationEnd - AllocationBegin);
    }
     */
}
