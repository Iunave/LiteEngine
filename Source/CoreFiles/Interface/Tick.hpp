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

    attr(hot) void Tick(float64 DeltaTime);

    attr(warn_unused_result, nonnull(2)) int64 AddTickable(OTickable* ObjectToAdd);

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

    bool IsTickRegistered() const
    {
        return TickPosition >= 0;
    }

    int64 GetTickPosition() const
    {
        return TickPosition;
    }

    void RegisterTick();

    void UnRegisterTick();

private:

    attr(hot) virtual void Tick(float64 DeltaTime) = 0;

    int64 TickPosition;
};

class FTimerManager : public OTickable, public FSingleton<FTimerManager>
{
private:

    friend class FSingleton<FTimerManager>;

    FTimerManager();

    struct FTimerData
    {
        void(*Function)(void*);
        void* FunctionData;

        float64 TargetTime;
        float64 PassedTime;
    };

public:

    attr(nonnull(2)) void AddTimer(void(*Function)(void*), float64 TargetTime, void* FunctionData = nullptr);

private:

    virtual void Tick(float64 DeltaTime) override;

    TDynamicArray<FTimerData> Timers;
};
