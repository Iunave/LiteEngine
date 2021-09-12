#pragma once

#include "Definitions.hpp"
#include "Array.hpp"
#include "SmartPointer.hpp"
#include "Interface/Damageable.hpp"
#include "Actor/Actor.hpp"

OBJECT_CLASS(ABuildComponent) : public AActor, public IDamageable
{
    OBJECT_BASES(AActor, IDamageable)
public:

    virtual uint32 NumAliveConnections() const = 0;
};

OBJECT_CLASS(ABuildComponentImpl, int64, NumConnectionPoints) : public ABuildComponent
{
    OBJECT_BASES(ABuildComponent)
public:

    virtual void TakeDamage(float64 DamageAmount) override;

    virtual void TakeRepair(float64 RepairAmount) override;

    virtual uint32 NumAliveConnections() const final;

    uint32 NumAliveConnectionsImpl() const;

protected:

    TStaticArray<TSharedPtr<ABuildComponent>, NumConnectionPoints> Connections;
};

