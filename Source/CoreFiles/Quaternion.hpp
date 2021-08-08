#pragma once

#include "Simd.hpp"

class FVector;

class alignas(32) FQuaternion final
{
public:
/*
    static consteval int32 ComparisonMask()
    {
        return Simd::float64_4::ComparisonMask();
    }

    inline explicit constexpr FQuaternion(ENoInit)
    {
    }

    inline explicit constexpr FQuaternion(const float64 X = 0.0, const float64 Y = 0.0, const float64 Z = 0.0, const float64 W = 0.0)
        : Register(X, Y, Z, W)
    {
    }

    inline constexpr FQuaternion(const FQuaternion& Other)
        : Register(Other.Register)
    {
    }

    inline constexpr FQuaternion(FQuaternion&& Other)
        : Register(Move(Other.Register))
    {
    }

    inline explicit constexpr FQuaternion(const Simd::float64_4& Other)
        : Register(Other)
    {
    }

    inline explicit constexpr FQuaternion(Simd::float64_4&& Other)
        : Register(Move(Other))
    {
    }

    inline FQuaternion& operator=(const FQuaternion& Other)
    {
        Register = Other.Register;
        return *this;
    }

    inline FQuaternion& operator=(FQuaternion&& Other)
    {
        Register = Move(Other.Register);
        return *this;
    }

    inline FQuaternion& operator=(const Simd::float64_4& Other)
    {
        Register = Other;
        return *this;
    }

    inline FQuaternion& operator=(Simd::float64_4&& Other)
    {
        Register = Move(Other);
        return *this;
    }

    inline FQuaternion& operator=(const float64 Value)
    {
        Register = Value;
        return *this;
    }

    UINLINE bool operator==(const FQuaternion& Other) const;

    UINLINE bool operator!=(const FQuaternion& Other) const;

    inline float64 X() const
    {
        return Register[0];
    }

    inline float64& X()
    {
        return Register[0];
    }

    inline float64 Y() const
    {
        return Register[1];
    }

    inline float64& Y()
    {
        return Register[1];
    }

    inline float64 Z() const
    {
        return Register[2];
    }

    inline float64& Z()
    {
        return Register[2];
    }

    inline float64 W() const
    {
        return Register[3];
    }

    inline float64& W()
    {
        return Register[3];
    }

    float64 Length() const;

    FVector RotateVector(FVector Vector) const;

private:

    float64_4 Register;
*/
};

