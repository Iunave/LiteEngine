#include "Tick.hpp"
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
    TickingObjects.RemoveAtSwap(ObjectPosition);

    if EXPECT(TickingObjects.IsIndexValid(ObjectPosition), true)
    {
        TickingObjects[ObjectPosition]->TickPosition = ObjectPosition;
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

