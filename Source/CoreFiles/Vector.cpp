#include "Vector.hpp"
#include "Math.hpp"

const constinit FVector FVector::ZeroVector{0.0, 0.0, 0.0};
const constinit FVector FVector::UpVector{0.0, 0.0, 1.0};
const constinit FVector FVector::DownVector{0.0, 0.0, -1.0};
const constinit FVector FVector::ForwardVector{1.0, 0.0, 0.0};
const constinit FVector FVector::BackwardVector{-1.0, 0.0, 0.0};
const constinit FVector FVector::LeftVector{0.0, 1.0, 0.0};
const constinit FVector FVector::RightVector{0.0, -1.0, 0.0};

void FVector::Set(const float64 X, const float64 Y, const float64 Z)
{
    Register = float64_4{X, Y, Z, 1.0};
}

bool FVector::ExactEquals(FVector Other) const
{
    const int32 ComparisonResult{Simd::MoveMask(Register == Other.Register)};
    return ComparisonResult == MaskXYZ() || ComparisonResult == MaskXYZW();
}

FVector& FVector::Clamp(FVector Min, FVector Max)
{
    Register = Simd::MakeFromGreater(Register, Min.Register);
    Register = Simd::MakeFromLesser(Register, Max.Register);

    return *this;
}

FVector FVector::GetClamped(FVector Min, FVector Max) const
{
    float64_4 ResultVector;

    ResultVector = Simd::MakeFromGreater(Register, Min.Register);
    ResultVector = Simd::MakeFromLesser(Register, Max.Register);

    return FVector{Move(ResultVector)};
}

float64 FVector::Length() const
{
    const float64_4 VectorSquared{Register * Register};

    return Math::SquareRoot(VectorSquared[0] + VectorSquared[1] + VectorSquared[2]);
}

FVector& FVector::Normalize()
{
    const float64 VectorLength{Length()};
    const float64_4 Divend{VectorLength + static_cast<float64>(VectorLength == 0.0)};

    Register /= Divend;

    return *this;
}

FVector FVector::GetNormal() const
{
    const float64 VectorLength{Length()};
    const float64_4 Divend{VectorLength + static_cast<float64>(VectorLength == 0.0)};

    return FVector{Register / Divend};
}

float64 FVector::DotProduct(FVector Other) const
{
    const float64_4 MultipliedVector{this->Register * Other.Register};

    return MultipliedVector[0] + MultipliedVector[1] + MultipliedVector[2];
}

FVector FVector::CrossProduct(FVector Other) const
{
    const float64_4 ThisPositionReversed{Simd::ShuffleVector<1, 2, 0, 3>(this->Register)};
    const float64_4 OtherPositionReversed{Simd::ShuffleVector<1, 2, 0, 3>(Other.Register)};

    const float64_4 Result{Simd::FusedMultiplySubtract(this->Register, OtherPositionReversed, Other.Register * ThisPositionReversed)};

    return FVector{Simd::ShuffleVector<1, 2, 0, 3>(Result)};
}

float64 FVector::AngleBetween(FVector Other) const
{
    const float64 LengthsMultiplied{Length() * Other.Length()};
    const float64 DotProductTroughLength{DotProduct(Other) / (LengthsMultiplied + static_cast<float64>(LengthsMultiplied == 0.0))};

    return Math::ArchCosine(DotProductTroughLength);
}

float64 FVector::operator%(FVector Other) const
{
    return AngleBetween(Move(Other));
}

float64 FVector::operator&(FVector Other) const
{
    return DotProduct(Move(Other));
}

FVector FVector::operator^(FVector Other) const
{
    return CrossProduct(Move(Other));
}

bool FVector::operator==(FVector Other) const
{
    const float64_4 SubResult{static_cast<int64_4>(Register - Other.Register) & Simd::SetAll<int64_4>(Math::SignMask<int64>())};

    const int32 ComparisonResult{Simd::MoveMask(SubResult <= 0.0001_float64_4)};

    return ComparisonResult == MaskXYZ() || ComparisonResult == MaskXYZW();
}

bool FVector::operator!=(FVector Other) const
{
    return !(*this == Move(Other));
}

bool FVector::operator==(const float64 OtherLength) const
{
    return this->Length() == OtherLength;
}

bool FVector::operator!=(const float64 OtherLength) const
{
    return this->Length() != OtherLength;
}

bool FVector::operator<=(FVector Other) const
{
    return this->Length() <= Other.Length();
}

bool FVector::operator<(FVector Other) const
{
    return this->Length() < Other.Length();
}

bool FVector::operator>=(FVector Other) const
{
    return this->Length() >= Other.Length();
}

bool FVector::operator>(FVector Other) const
{
    return this->Length() > Other.Length();
}

FVector& FVector::operator~()
{
    Register *= Simd::SetAll<float64_4>(-1.0);
    return *this;
}



