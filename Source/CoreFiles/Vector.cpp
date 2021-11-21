#include "Vector.hpp"
#include "Math.hpp"
#include "String.hpp"

#include <fmt/core.h>

const constinit FVector FVector::Zero{0.0, 0.0, 0.0};
const constinit FVector FVector::Up{0.0, 0.0, 1.0};
const constinit FVector FVector::Down{0.0, 0.0, -1.0};
const constinit FVector FVector::Forward{1.0, 0.0, 0.0};
const constinit FVector FVector::Backward{-1.0, 0.0, 0.0};
const constinit FVector FVector::Left{0.0, 1.0, 0.0};
const constinit FVector FVector::Right{0.0, -1.0, 0.0};

bool FVector::ExactEquals(FVector Other) const
{
    const int32 ComparisonResult{Simd::MoveMask(Vector == Other.Vector)};
    return ComparisonResult == MaskXYZ || ComparisonResult == MaskXYZW;
}

FVector& FVector::Clamp(FVector Min, FVector Max)
{
    Simd::Clamp(Vector, Min.Vector, Max.Vector);
    return *this;
}

FVector FVector::GetClamped(FVector Min, FVector Max) const
{
    FVector ResultVector{Vector};
    Simd::Clamp(ResultVector.Vector, Min.Vector, Max.Vector);
    return ResultVector;
}

float64 FVector::Length() const
{
    const float64_4 VectorSquared{Vector * Vector};
    return Math::SquareRoot(VectorSquared[0] + VectorSquared[1] + VectorSquared[2]);
}

FVector& FVector::Normalize()
{
    Vector /= Length();
    return *this;
}

FVector FVector::GetNormal() const
{
    return FVector{Vector / Length()};
}

float64 FVector::DotProduct(FVector Other) const
{
    const float64_4 MultipliedVector{this->Vector * Other.Vector};
    return MultipliedVector[0] + MultipliedVector[1] + MultipliedVector[2];
}

FVector FVector::CrossProduct(FVector Other) const
{
    const float64_4 ThisPositionReversed{Simd::Shuffle<1, 2, 0, 3>(this->Vector)};
    const float64_4 OtherPositionReversed{Simd::Shuffle<1, 2, 0, 3>(Other.Vector)};

    const float64_4 Result{Simd::FusedMultiplySubtract(this->Vector, OtherPositionReversed, Other.Vector * ThisPositionReversed)};

    return FVector{Simd::Shuffle<1, 2, 0, 3>(Result)};
}

float64 FVector::AngleBetween(FVector Other) const
{
    const float64 LengthsMultiplied{Length() * Other.Length()};
    const float64 DotProductTroughLength{DotProduct(Other) / LengthsMultiplied};

    return Math::ArchCosine(DotProductTroughLength);
}

bool FVector::operator==(FVector Other) const
{
    const float64_4 SubResult{static_cast<int64_4>(Vector - Other.Vector) & Math::SignMask<int64>()};
    const int32 ComparisonResult{Simd::MoveMask(SubResult <= 0.0001)};

    return ComparisonResult == MaskXYZ || ComparisonResult == MaskXYZW;
}

FString<SS124> StrUtl::ToString(FVector Source)
{
    FString<SS124> String{};

    const char8* End{fmt::format_to(String.Data(), "X={:+f} Y={:+f} Z={:+f}", Source.X(), Source.Y(), Source.Z())};
    String.SetTerminatorIndex(static_cast<uint32>(End - String.Data()));

    return String;
}



