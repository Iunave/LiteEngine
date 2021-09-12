#pragma once

#include "Simd.hpp"

class FVector;

class alignas(32) FQuaternion final
{
public:

    static const constinit FQuaternion Identity;
    static const constinit FQuaternion Up;
    static const constinit FQuaternion Down;
    static const constinit FQuaternion Left;
    static const constinit FQuaternion Right;
    static const constinit FQuaternion Front;
    static const constinit FQuaternion Back;

    static inline constexpr int32 Mask{Simd::Mask<float64_4>()};

public:

    explicit constexpr FQuaternion(ENoInit)
    {
    }

    explicit constexpr FQuaternion(float64 X = 0.0, float64 Y = 0.0, float64 Z = 0.0, float64 W = 1.0)
        : Quaternion{X, Y, Z, W}
    {
    }

    constexpr FQuaternion(const FQuaternion& Other)
        : Quaternion(Other.Quaternion)
    {
    }

    constexpr FQuaternion(FQuaternion&& Other)
        : Quaternion(Other.Quaternion)
    {
    }

    explicit constexpr FQuaternion(float64_4 OtherQuaternion)
        : Quaternion(OtherQuaternion)
    {
    }

    FQuaternion& operator=(FQuaternion Other)
    {
        Quaternion = Other.Quaternion;
        return *this;
    }

    FQuaternion& operator=(float64_4 Other)
    {
        Quaternion = Other;
        return *this;
    }

    FQuaternion& operator=(float64 Value)
    {
        Quaternion = Simd::SetAll<float64_4>(Value);
        return *this;
    }

    float64 X() const
    {
        return Quaternion[0];
    }

    float64& X()
    {
        return Simd::ToPtr(&Quaternion)[0];
    }

    float64 Y() const
    {
        return Quaternion[1];
    }

    float64& Y()
    {
        return Simd::ToPtr(&Quaternion)[1];
    }

    float64 Z() const
    {
        return Quaternion[2];
    }

    float64& Z()
    {
        return Simd::ToPtr(&Quaternion)[2];
    }

    float64 W() const
    {
        return Quaternion[3];
    }

    float64& W()
    {
        return Simd::ToPtr(&Quaternion)[3];
    }

    float64_4 Data() const
    {
        return Quaternion;
    }

    float64_4& Data()
    {
        return Quaternion;
    }

    bool ExactEquals(FQuaternion Other) const;

    float64 Length() const;

    FQuaternion& Normalize();

    FQuaternion GetNormal() const;

    float64 Angle() const;

    //checks if two queternions are almost equal with tolerance = 0.0001
    bool operator==(FQuaternion Other) const;

    INLINE bool operator!=(FQuaternion Other) const
    {
        return !(*this == Other);
    }

    float64 operator[](TypeTrait::IsInteger auto Index) const
    {
        ASSERT(Index >= 0 && Index <= 3);
        return Quaternion[Index];
    }

    float64& operator[](TypeTrait::IsInteger auto Index)
    {
        ASSERT(Index >= 0 && Index <= 3);
        return Simd::ToPtr(&Quaternion)[Index];
    }

    FQuaternion operator*(FQuaternion Other) const;

    INLINE FQuaternion& operator*=(FQuaternion Other)
    {
        return *this = (*this * Other);
    }

private:

    float64_4 Quaternion;
};

