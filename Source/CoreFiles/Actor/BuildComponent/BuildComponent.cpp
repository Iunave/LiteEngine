#include "BuildComponent.hpp"

template<int64 NumConnectionPoints>
void ABuildComponentImpl<NumConnectionPoints>::TakeDamage(float64 DamageAmount)
{
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
        for(TSharedPtr<ABuildComponent>& ConnectedComponent : Connections)
        {
            if(ConnectedComponent.IsValid() && ConnectedComponent->IsAlive())
            {
                ConnectedComponent->TakeDamage(DamageOverflow);
            }
        }
    }
}

template<int64 NumConnectionPoints>
uint32 ABuildComponentImpl<NumConnectionPoints>::NumAliveConnections() const
{
    return NumAliveConnectionsImpl();
}

template<int64 NumConnectionPoints>
uint32 ABuildComponentImpl<NumConnectionPoints>::NumAliveConnectionsImpl() const
{
    uint32 Count{0};

    #pragma unroll
    for(const TSharedPtr<ABuildComponent>& ConnectedComponent : Connections)
    {
        if(ConnectedComponent.IsValid())
        {
            Count += ConnectedComponent->IsAlive();
        }
    }

    return Count;
}

template class ABuildComponentImpl<1>;
template class ABuildComponentImpl<2>;
template class ABuildComponentImpl<3>;
template class ABuildComponentImpl<4>;
template class ABuildComponentImpl<5>;
template class ABuildComponentImpl<6>;
