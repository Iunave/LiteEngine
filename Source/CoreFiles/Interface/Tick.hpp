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

class FTickManager final
{
private:

    FTickManager(const int64 ObjectsToPreallocate);

    ~FTickManager();

public:

    static inline FTickManager& Instance()
    {
        static FTickManager Instance{1024};
        return Instance;
    }

    HOT void Tick(float64 DeltaTime);

    int64 AddTickable(OTickable* ObjectToAdd);

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

    HOT virtual void Tick(float64 DeltaTime) = 0;

    int64 TickPosition;
};

class FTimerManager : public OTickable
{
private:

    FTimerManager();

    struct FTimerData
    {
        void(*Function)(void*);
        void* FunctionData;

        float64 TargetTime;
        float64 PassedTime;
    };

public:

    CONSTRUCTOR(50) static inline FTimerManager& Instance()
    {
        static FTimerManager Instance{};
        return Instance;
    }

    void AddTimer(void(*Function)(void*), float64 TargetTime, void* FunctionData = nullptr);

private:

    virtual void Tick(float64 DeltaTime) override;

    TDynamicArray<FTimerData> Timers;
};
