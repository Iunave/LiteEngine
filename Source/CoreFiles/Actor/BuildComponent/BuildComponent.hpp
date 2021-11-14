#pragma once

#include "Definitions.hpp"
#include "Array.hpp"
#include "SmartPointer.hpp"
#include "Interface/Damageable.hpp"
#include "Actor/Actor.hpp"

class ARobot;

OBJECT_CLASS(ABuildComponent)
class ABuildComponent : public AActor, public IDamageable
{
    OBJECT_BASES(AActor, IDamageable)
public:

    virtual void TakeDamage(float64 DamageAmount) override;

    uint32 NumAliveConnections() const;

protected:

    TWeakPtr<ARobot> Owner;

    TCountedArray<uint16, 6> ConnectionOffsets;
};

