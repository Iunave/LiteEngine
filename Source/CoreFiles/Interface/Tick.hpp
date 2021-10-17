#pragma once

#include "Definitions.hpp"
#include "Array.hpp"
#include "Object/Object.hpp"

class OTickable;

namespace ArrayConstants
{
    template<>
    inline consteval int64 ReserveExtraValue<OTickable*>()
    {
        return 256LL;
    }
}

class FTickManager final : public FSingleton<FTickManager, 1024>
{
private:

    friend class FSingleton<FTickManager, 1024>;

    FTickManager(const int64 ObjectsToPreallocate);

    ~FTickManager();

public:

    void Tick(float64 DeltaTime);

    NODISCARD int64 AddTickable(OTickable* ObjectToAdd NONNULL);

    void RemoveTickable(const int64 ObjectPosition);

    void RemoveAll();

private:

    TDynamicArray<OTickable*> TickingObjects;
};


OBJECT_CLASS(OTickable)
class OTickable
{
    OBJECT_BASES()
    friend class FTickManager;
public:

    explicit OTickable(const bool bRegisterTickOnConstruction = true);

    virtual ~OTickable();

    INLINE bool IsTickRegistered() const
    {
        return TickPosition >= 0;
    }

    INLINE int64 GetTickPosition() const
    {
        return TickPosition;
    }

    void RegisterTick();

    void UnRegisterTick();

private:

    virtual void Tick(float64 DeltaTime) = 0;

    int64 TickPosition;
};
