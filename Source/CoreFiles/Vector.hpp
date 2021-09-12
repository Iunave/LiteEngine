#pragma once

#include "Simd.hpp"
#include "Assert.hpp"
#include "TypeTraits.hpp"

class alignas(32) FVector final
{
public:

    static const constinit FVector Zero;
    static const constinit FVector Up;
    static const constinit FVector Down;
    static const constinit FVector Forward;
    static const constinit FVector Backward;
    static const constinit FVector Left;
    static const constinit FVector Right;

    static inline constexpr int32 MaskXYZ{0b00000000000000000000000000000111};
    static inline constexpr int32 MaskXYZW{0b00000000000000000000000000001111};

public:

    explicit constexpr FVector(ENoInit)
    {
    }

    explicit constexpr FVector(float64 X = 0.0, float64 Y = 0.0, float64 Z = 0.0)
        : Vector{X, Y, Z, 1.0}
    {
    }

    constexpr FVector(const FVector& Other)
        : Vector(Other.Vector)
    {
    }

    constexpr FVector(FVector&& Other)
        : Vector(Other.Vector)
    {
    }

    explicit constexpr FVector(float64_4 OtherVector)
        : Vector(OtherVector)
    {
    }

    FVector& operator=(FVector Other)
    {
        Vector = Other.Vector;
        return *this;
    }

    FVector& operator=(float64_4 Other)
    {
        Vector = Other;
        return *this;
    }

    FVector& operator=(const float64 Value)
    {
        Vector = Simd::SetAll<float64_4>(Value);
        return *this;
    }

    float64 X() const
    {
        return Vector[0];
    }

    float64& X()
    {
        return Simd::ToPtr(&Vector)[0];
    }

    float64 Y() const
    {
        return Vector[1];
    }

    float64& Y()
    {
        return Simd::ToPtr(&Vector)[1];
    }

    float64 Z() const
    {
        return Vector[2];
    }

    float64& Z()
    {
        return Simd::ToPtr(&Vector)[2];
    }

    float64 W() const
    {
        return Vector[3];
    }

    float64& W()
    {
        return Simd::ToPtr(&Vector)[3];
    }

    float64_4 Data() const
    {
        return Vector;
    }

    float64_4& Data()
    {
        return Vector;
    }

    bool ExactEquals(FVector Other) const;

    FVector& Clamp(FVector Min, FVector Max);

    FVector GetClamped(FVector Min, FVector Max) const;

    float64 Length() const;

    FVector& Normalize();

    FVector GetNormal() const;

    float64 DotProduct(FVector Other) const;

    FVector CrossProduct(FVector Other) const;

    //calculates the angle in radians between two vectors
    float64 AngleBetween(FVector Other) const;

    float64 operator[](TypeTrait::IsInteger auto Index) const
    {
        ASSERT(Index >= 0 && Index <= 3);
        return Vector[Index];
    }

    float64& operator[](TypeTrait::IsInteger auto Index)
    {
        ASSERT(Index >= 0 && Index <= 3);
        return Simd::ToPtr(&Vector)[Index];
    }

    //calculates the angle in radians between two vectors
    INLINE float64 operator%(FVector Other) const
    {
        return AngleBetween(Other);
    }

    //calculate the dot-product
    INLINE float64 operator&(FVector Other) const
    {
        return DotProduct(Other);
    }

    //calculate the cross-product
    INLINE FVector operator^(FVector Other) const
    {
        return CrossProduct(Other);
    }

    //checks if two vectors are almost equal with tolerance = 0.0001
    bool operator==(FVector Other) const;

    bool operator!=(FVector Other) const
    {
        return !(*this == Other);
    }

    bool operator==(const float64 OtherLength) const
    {
        return this->Length() == OtherLength;
    }

    bool operator!=(const float64 OtherLength) const
    {
        return this->Length() != OtherLength;
    }

    bool operator<=(FVector Other) const
    {
        return this->Length() <= Other.Length();
    }

    bool operator<(FVector Other) const
    {
        return this->Length() < Other.Length();
    }

    bool operator>=(FVector Other) const
    {
        return this->Length() >= Other.Length();
    }

    bool operator>(FVector Other) const
    {
        return this->Length() > Other.Length();
    }

    //flip vector
    INLINE FVector& operator~()
    {
        Vector *= -1.0;
        return *this;
    }

    INLINE FVector operator+(FVector Other) const
    {
        return FVector{Vector + Other.Vector};
    }

    INLINE FVector operator+(float64 Value) const
    {
        return FVector{Vector + Value};
    }

    INLINE FVector& operator+=(FVector Other)
    {
        Vector += Other.Vector;
        return *this;
    }

    INLINE FVector& operator+=(float64 Value)
    {
        Vector += Value;
        return *this;
    }

    INLINE FVector operator-(FVector Other) const
    {
        return FVector{Vector - Other.Vector};
    }

    INLINE FVector operator-(float64 Value) const
    {
        return FVector{Vector - Value};
    }

    INLINE FVector& operator-=(FVector Other)
    {
        Vector -= Other.Vector;
        return *this;
    }

    INLINE FVector& operator-=(float64 Value)
    {
        Vector -= Value;
        return *this;
    }

    INLINE FVector operator*(FVector Other) const
    {
        return FVector{Vector * Other.Vector};
    }

    INLINE FVector operator*(float64 Value) const
    {
        return FVector{Vector * Value};
    }

    INLINE FVector& operator*=(FVector Other)
    {
        Vector *= Other.Vector;
        return *this;
    }

    INLINE FVector& operator*=(float64 Value)
    {
        Vector *= Value;
        return *this;
    }

    INLINE FVector operator/(FVector Other) const
    {
        return FVector{Vector / Other.Vector};
    }

    INLINE FVector operator/(float64 Value) const
    {
        return FVector{Vector / Value};
    }

    INLINE FVector& operator/=(FVector Other)
    {
        Vector /= Other.Vector;
        return *this;
    }

    INLINE FVector& operator/=(float64 Value)
    {
        Vector /= Value;
        return *this;
    }

private:

    float64_4 Vector;
};
