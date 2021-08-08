#include "Quaternion.hpp"
#include "Vector.hpp"
#include "Math.hpp"
/*
bool FQuaternion::operator==(const FQuaternion& Other) const
{
    return (Register == Other.Register) == ComparisonMask();
}

bool FQuaternion::operator!=(const FQuaternion& Other) const
{
    return (Register != Other.Register) == ComparisonMask();
}

float64 FQuaternion::Length() const
{
    const Simd::float64_4 VectorSquared{Register * Register};

    return Math::SquareRoot(VectorSquared[0] + VectorSquared[1] + VectorSquared[2]);
}

FVector FQuaternion::RotateVector(FVector Vector) const
{
    const FVector QuatVector{Register};

    const FVector CrossProduct{(QuatVector ^ Vector) * 2.0};

    Vector += (CrossProduct * W()) + (QuatVector ^ CrossProduct);

    return Vector;
}
*/
