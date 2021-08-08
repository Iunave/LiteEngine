#pragma once

#include "Simd.hpp"

class alignas(32) FVector final
{
public:

    static const constinit FVector ZeroVector;
    static const constinit FVector UpVector;
    static const constinit FVector DownVector;
    static const constinit FVector ForwardVector;
    static const constinit FVector BackwardVector;
    static const constinit FVector LeftVector;
    static const constinit FVector RightVector;

public:

    static consteval int32 MaskXYZ()
    {
        return static_cast<int32>(0b00000000000000000000000000000111);
    }

    static consteval int32 MaskXYZW()
    {
        return static_cast<int32>(0b00000000000000000000000000001111);
    }

    inline explicit constexpr FVector(ENoInit)
    {
    }

    inline constexpr explicit FVector(float64 X, float64 Y = 0.0, float64 Z = 0.0)
        : Register{X, Y, Z, 1.0}
    {
    }

    inline constexpr FVector(const FVector& Other)
        : Register(Other.Register)
    {
    }

    inline constexpr FVector(FVector&& Other)
        : Register(Move(Other.Register))
    {
    }

    inline explicit constexpr FVector(float64_4 OtherVector)
        : Register(Move(OtherVector))
    {
    }

    inline FVector& operator=(FVector Other)
    {
        Register = Move(Other.Register);
        return *this;
    }

    inline FVector& operator=(float64_4 Other)
    {
        Register = Move(Other);
        return *this;
    }

    inline FVector& operator=(const float64 Value)
    {
        Register = Simd::SetAll<float64_4>(Value);
        return *this;
    }

    inline float64 X() const
    {
        return Register[0];
    }

    inline float64& X()
    {
        return Simd::ToPtr(&Register)[0];
    }

    inline float64 Y() const
    {
        return Register[1];
    }

    inline float64& Y()
    {
        return Simd::ToPtr(&Register)[1];
    }

    inline float64 Z() const
    {
        return Register[2];
    }

    inline float64& Z()
    {
        return Simd::ToPtr(&Register)[2];
    }

    UINLINE void Set(const float64 X, const float64 Y, const float64 Z);

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

    //calculates the angle in radians between two vectors
    FLATTEN float64 operator%(FVector Other) const;

    //calculate the dot-product
    FLATTEN float64 operator&(FVector Other) const;

    //calculate the cross-product
    FLATTEN FVector operator^(FVector Other) const;

    //checks if two vectors are almost equal with tolerance = 0.0001
    bool operator==(FVector Other) const;

    //checks if two vectors are almost not equal with tolerance < 0.0001
    bool operator!=(FVector Other) const;

    //compare the magnitude of 2 vectors
    bool operator==(const float64 OtherLength) const;
    bool operator!=(const float64 OtherLength) const;
    bool operator<=(FVector Other) const;
    bool operator<(FVector Other) const;
    bool operator>=(FVector Other) const;
    bool operator>(FVector Other) const;

    //flip vector
    UINLINE FVector& operator~();

    INLINE FVector operator+(FVector Other) const
    {
        return FVector{Register + Other.Register};
    }

    INLINE FVector& operator+=(FVector Other)
    {
        Register += Other.Register;
        return *this;
    }

    INLINE FVector operator-(FVector Other) const
    {
        return FVector{Register - Other.Register};
    }

    INLINE FVector& operator-=(FVector Other)
    {
        Register -= Other.Register;
        return *this;
    }

    INLINE FVector operator*(FVector Other) const
    {
        return FVector{Register * Other.Register};
    }

    INLINE FVector& operator*=(FVector Other)
    {
        Register *= Other.Register;
        return *this;
    }

    INLINE FVector operator/(FVector Other) const
    {
        return FVector{Register / Other.Register};
    }

    INLINE FVector& operator/=(FVector Other)
    {
        Register /= Other.Register;
        return *this;
    }

private:

    float64_4 Register;

};
