#include "Quaternion.hpp"
#include "Vector.hpp"
#include "Math.hpp"
#include "String.hpp"

#include <fmt/core.h>

const constinit FQuaternion FQuaternion::Identity{0, 0, 0, 1};
const constinit FQuaternion FQuaternion::Up{0, 0, 1, 0};
const constinit FQuaternion FQuaternion::Down{0, 0, -1, 0};
const constinit FQuaternion FQuaternion::Left{0, 1, 0, 0};
const constinit FQuaternion FQuaternion::Right{0, -1, 0, 0};
const constinit FQuaternion FQuaternion::Front{1, 0, 0, 0};
const constinit FQuaternion FQuaternion::Back{-1, 0, 0, 0};

bool FQuaternion::ExactEquals(FQuaternion Other) const
{
    const int32 ComparisonResult{Simd::MoveMask(Quaternion == Other.Quaternion)};
    return ComparisonResult == Mask;
}

bool FQuaternion::operator==(FQuaternion Other) const
{
    const float64_4 SubResult{static_cast<int64_4>(Quaternion - Other.Quaternion) & Math::SignMask<int64>()};
    const int32 ComparisonResult{Simd::MoveMask(SubResult <= 0.0001)};

    return ComparisonResult == Mask;
}

FQuaternion FQuaternion::operator*(FQuaternion Other) const
{
    float64_4 VectorX{Simd::SetAll<float64_4>(X())};
    float64_4 VectorY{Simd::SetAll<float64_4>(Y())};
    float64_4 VectorZ{Simd::SetAll<float64_4>(Z())};
    float64_4 VectorW{Simd::SetAll<float64_4>(W())};

    const float64_4 ShuffledOtherX{Simd::ShuffleCrossLane<3, 2, 1, 0>(Other.Quaternion)};
    const float64_4 ShuffledOtherY{Simd::ShuffleCrossLane<2, 3, 0, 1>(Other.Quaternion)};
    const float64_4 ShuffledOtherZ{Simd::ShuffleCrossLane<1, 0, 3, 2>(Other.Quaternion)};
    const float64_4 ShuffledOtherW{Simd::ShuffleCrossLane<0, 1, 2, 3>(Other.Quaternion)};

    VectorX *= ShuffledOtherX;
    VectorY *= ShuffledOtherY;
    VectorZ *= ShuffledOtherZ;
    VectorW *= ShuffledOtherW;

    constexpr float64_4 SignMaskX{1.0, -1.0, 1.0, -1.0};
    constexpr float64_4 SignMaskY{1.0, 1.0, -1.0, -1.0};
    constexpr float64_4 SignMaskZ{-1.0, 1.0, 1.0, -1.0};

    float64_4 ResultVector{VectorW};
    ResultVector = Simd::FusedMultiplyAdd(VectorX, SignMaskX, ResultVector);
    ResultVector = Simd::FusedMultiplyAdd(VectorY, SignMaskY, ResultVector);
    ResultVector = Simd::FusedMultiplyAdd(VectorZ, SignMaskZ, ResultVector);

    return FQuaternion{ResultVector};
}

float64 FQuaternion::Length() const
{
    const float64_4 VectorSquared{Quaternion * Quaternion};

    float64_2 Lower{Simd::Extract<0>(VectorSquared)};
    float64_2 Upper{Simd::Extract<1>(VectorSquared)};

    Lower += Upper;

    return Math::SquareRoot(Lower[0] + Lower[1]);
}

FQuaternion& FQuaternion::Normalize()
{
    Quaternion /= Length();
    return *this;
}

FQuaternion FQuaternion::GetNormal() const
{
    return FQuaternion{Quaternion / Length()};
}

float64 FQuaternion::Angle() const
{
    return Math::ArchCosine(W()) * 2.0;
}

FString<SS124> StrUtl::ToString(FQuaternion Source)
{
    FString<SS124> String{};

    const char8* End{fmt::format_to(String.Data(), "X={:+f} Y={:+f} Z={:+f} W={:+f}", Source.X(), Source.Y(), Source.Z(), Source.W())};
    String.SetTerminatorIndex(static_cast<uint32>(End - String.Data()));

    return String;
}
