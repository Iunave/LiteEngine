#include "Math.hpp"
#include "Vector.hpp"

namespace Math
{
    bool IsInSphere(float64 SphereRadius, FVector SpherePosition, FVector TargetPosition)
    {
        TargetPosition -= SpherePosition;
        TargetPosition *= TargetPosition;
        SphereRadius *= SphereRadius;

        const float64 Sum{TargetPosition.X() + TargetPosition.Y() + TargetPosition.Z()};
        return Sum <= SphereRadius;
    }
}
