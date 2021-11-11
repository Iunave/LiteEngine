#pragma once

#include "Definitions.hpp"
#include "Array.hpp"
#include "SmartPointer.hpp"
#include "Interface/Damageable.hpp"
#include "Actor/Actor.hpp"

enum class EComponentTier : uint8
{
    T1, T2, T3, T4, T5, T6, T7, T9, T10
};

OBJECT_CLASS(ABuildComponent)
class ABuildComponent : public AActor, public IDamageable
{
    OBJECT_BASES(AActor, IDamageable)
public:

    virtual void TakeDamage(float64 DamageAmount) override;

    uint32 NumAliveConnections() const;

protected:

    TStaticArray<uint16, 6> ConnectionOffsets;
};

