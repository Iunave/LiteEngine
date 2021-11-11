#include "Math.hpp"
#include "Vector.hpp"
#include "Time.hpp"

#include <random>

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

    uint64 GenerateRandom(uint64 Min, uint64 Max)
    {
        std::random_device Seed{};
        std::mt19937_64 Twister{Seed()};
        std::uniform_int_distribution<uint64> Distribution{Min, Max};
        return Distribution(Twister);
    }
}
