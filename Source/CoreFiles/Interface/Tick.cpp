#include "Interface/Tick.hpp"
#include "Math.hpp"

FTickManager::FTickManager(const int64 ObjectsToPreallocate)
{
    TickingObjects.ReserveUndefined(ObjectsToPreallocate);
}

FTickManager::~FTickManager()
{
    RemoveAll();
}

void FTickManager::Tick(float64 DeltaTime)
{
    for(OTickable* Tickable : TickingObjects)
    {
        Tickable->Tick(DeltaTime);
    }
}

int64 FTickManager::AddTickable(OTickable* ObjectToAdd NONNULL)
{
    return TickingObjects.Append(ObjectToAdd);
}

void FTickManager::RemoveTickable(const int64 ObjectPosition)
{
    const int64 MovedObjectIndex{TickingObjects.RemoveAtSwap(ObjectPosition, 1)};

    if EXPECT(TickingObjects.IsIndexValid(MovedObjectIndex), true)
    {
        TickingObjects[MovedObjectIndex]->TickPosition = MovedObjectIndex;
    }
}

void FTickManager::RemoveAll()
{
    for(OTickable* Tickable : TickingObjects)
    {
        Tickable->TickPosition = INDEX_NONE;
    }

    TickingObjects.Empty();
}

OTickable::OTickable(const bool bRegisterTickOnConstruction)
{
    if(bRegisterTickOnConstruction)
    {
        TickPosition = FTickManager::Instance().AddTickable(this);
    }
    else
    {
        TickPosition = INDEX_NONE;
    }
}

OTickable::~OTickable()
{
    UnRegisterTick();
}

void OTickable::RegisterTick()
{
    if(!IsTickRegistered())
    {
        TickPosition = FTickManager::Instance().AddTickable(this);
    }
}

void OTickable::UnRegisterTick()
{
    if(IsTickRegistered())
    {
        FTickManager::Instance().RemoveTickable(TickPosition);
    }
}

