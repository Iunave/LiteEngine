#include "Robot.hpp"
#include "BuildComponent/BuildComponent.hpp"
#include "BuildComponent/MovementComponent.hpp"
#include "BuildComponent/WeaponComponent.hpp"

void ARobot::AddComponent(TSharedPtr<ABuildComponent> NewComponent)
{
    TSharedPtr<AWeaponComponent> NewWeapon{ObjectCast<AWeaponComponent*>(NewComponent)};
    TSharedPtr<AMovementComponent> NewMovement{ObjectCast<AMovementComponent*>(NewComponent)};

    if(NewWeapon.IsValid())
    {
        WeaponComponents.Append(Move(NewWeapon));
    }

    if(NewMovement.IsValid())
    {
        MovementComponents.Append(Move(NewMovement));
    }

    AllComponents.Append(Move(NewComponent));
}
