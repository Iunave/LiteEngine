#pragma once

#include "Definitions.hpp"
#include "Array.hpp"
#include "SmartPointer.hpp"
#include "Actor.hpp"

class ABuildComponent;
class AMovementComponent;
class AWeaponComponent;

OBJECT_CLASS(ARobot) : public AActor
{
    OBJECT_BASES(AActor)
public:

    void AddComponent(TSharedPtr<ABuildComponent> NewComponent);

protected:

    TDynamicArray<TSharedPtr<ABuildComponent>> AllComponents;
    TDynamicArray<TSharedPtr<AWeaponComponent>> WeaponComponents;
    TDynamicArray<TSharedPtr<AMovementComponent>> MovementComponents;
};
