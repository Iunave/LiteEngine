#pragma once

#include "Simd.hpp"
/*
class alignas(16) FColor final
{
public:

    static consteval int32 ComparisonMask()
    {
        return Simd::float32_4::ComparisonMask();
    }

    inline explicit constexpr FColor(ENoInit)
    {
    }

    inline explicit constexpr FColor(const float32 Value = 0)
        : Vector(Value)
    {
    }

    inline explicit constexpr FColor(const float32 R, const float32 G, const float32 B, const float32 A = 1.0f)
        : Vector(R, G, B, A)
    {
    }

    inline constexpr FColor(const FColor& Other)
        : Vector(Other.Vector)
    {
    }

    inline constexpr FColor(FColor&& Other)
        : Vector(Move(Other.Vector))
    {
    }

    inline explicit constexpr FColor(const Simd::float32_4& Other)
        : Vector(Other)
    {
    }

    inline explicit constexpr FColor(Simd::float32_4&& Other)
        : Vector(Move(Other))
    {
    }

    inline FColor& operator=(const FColor& Other)
    {
        Vector = Other.Vector;
        return *this;
    }

    inline FColor& operator=(FColor&& Other)
    {
        Vector = Move(Other.Vector);
        return *this;
    }

    inline FColor& operator=(const Simd::float32_4& Other)
    {
        Vector = Other;
        return *this;
    }

    inline FColor& operator=(Simd::float32_4&& Other)
    {
        Vector = Move(Other);
        return *this;
    }

    inline FColor& operator=(const float32 Value)
    {
        Vector = Value;
        return *this;
    }

    inline float32 R() const
    {
        return Vector[0];
    }

    inline float32& R()
    {
        return Vector[0];
    }

    inline float32 G() const
    {
        return Vector[1];
    }

    inline float32& G()
    {
        return Vector[1];
    }

    inline float32 B() const
    {
        return Vector[2];
    }

    inline float32& B()
    {
        return Vector[2];
    }

    inline float32 A() const
    {
        return Vector[3];
    }

    inline float32& A()
    {
        return Vector[3];
    }

    DEFINE_ARITHMETIC_OPERATORS(FColor, Vector)
    DEFINE_ARITHMETIC_OPERATORS_ARG(FColor, Vector, float32)

private:

    Simd::float32_4 Vector;

};
*/
