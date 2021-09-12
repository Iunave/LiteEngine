#pragma once

#include "Definitions.hpp"
#include "Object/Object.hpp"
#include "Color.hpp"

class FColor;

OBJECT_CLASS(IDamageable)
{
    OBJECT_BASES()
public:

    IDamageable(float64 InMaxArmour = 0.0)
        : MaxArmour(InMaxArmour)
        , CurrentArmour(InMaxArmour)
    {
    }

    float64 GetMaxArmour() const
    {
        return MaxArmour;
    }

    float64 GetCurrentArmour() const
    {
        return CurrentArmour;
    }

    bool IsAlive() const
    {
        return MaxArmour > 0.0;
    }

    bool IsDestroyed() const
    {
        return MaxArmour <= 0.0;
    }

    virtual void TakeDamage(float64 DamageAmount) = 0;

    virtual void TakeRepair(float64 RepairAmount) = 0;

    NODISCARD FColor ConvertArmourToColor() const;

protected:

    float64 MaxArmour;
    float64 CurrentArmour;

    FColor DefaultColor;
};

