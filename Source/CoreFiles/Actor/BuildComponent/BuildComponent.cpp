#include "BuildComponent.hpp"
#include "Actor/Robot.hpp"

void ABuildComponent::TakeDamage(float64 DamageAmount)
{
    /*
    float64 DamageOverflow{DamageAmount - CurrentArmour};
    CurrentArmour -= DamageAmount;

    if(CurrentArmour < 0.0)
    {
        CurrentArmour = 0.0;

        const uint32 AliveConnections{NumAliveConnectionsImpl()};

        if(AliveConnections == 0)
        {
            return;
        }

        DamageOverflow /= AliveConnections;

        #pragma unroll
        for(ABuildComponent* ConnectedComponent : Connections)
        {
            if(ConnectedComponent != nullptr && ConnectedComponent->IsAlive())
            {
                ConnectedComponent->TakeDamage(DamageOverflow);
            }
        }
    }
    */
}

uint32 ABuildComponent::NumAliveConnections() const
{
    uint32 Count{0};

    for(const uint16 Offset : ConnectionOffsets)
    {
        Count += Owner->Components[Offset].IsAlive();
    }

    return Count;
}

