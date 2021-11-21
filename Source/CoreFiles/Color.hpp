#pragma once

#include "Definitions.hpp"
#include "Simd.hpp"

struct ATTRIBUTE(packed, may_alias) RGBA8I final
{
    uint8 R;
    uint8 G;
    uint8 B;
    uint8 A;
};

struct ATTRIBUTE(packed, may_alias) RGBA10I final
{
    uint64 R : 10;
    uint64 G : 10;
    uint64 B : 10;
    uint64 A : 10;
};

struct ATTRIBUTE(packed, may_alias) RGBA16I final
{
    uint16 R;
    uint16 G;
    uint16 B;
    uint16 A;
};

struct ATTRIBUTE(packed, may_alias) RGBA32F final
{
    float32 R;
    float32 G;
    float32 B;
    float32 A;
};

class FColor final
{
public:

    using ColorType = float64_4;
    using ChannelType = Simd::ElementType<ColorType>;

    static const constinit FColor Red;
    static const constinit FColor Yellow;
    static const constinit FColor Green;
    static const constinit FColor Turquoise;
    static const constinit FColor Blue;
    static const constinit FColor Pink;
    static const constinit FColor Black;
    static const constinit FColor White;

    static inline constexpr int32 ColorMask{Simd::Mask<float64_4>()};

public:

    explicit constexpr FColor(ENoInit)
    {
    }

    constexpr FColor(ChannelType R = 0.0, ChannelType G = 0.0, ChannelType B = 0.0, ChannelType A = 1.0)
        : Color{R, G, B, A}
    {
    }

    FColor(RGBA8I InColor);

    FColor(RGBA16I InColor);

    FColor(RGBA32F InColor);

    constexpr FColor(const FColor& Other)
        : Color(Other.Color)
    {
    }

    constexpr FColor(FColor&& Other)
        : Color(Other.Color)
    {
    }

    explicit constexpr FColor(ColorType OtherColor)
        : Color(OtherColor)
    {
    }

    FColor& operator=(FColor Other)
    {
        Color = Other.Color;
        return *this;
    }

    FColor& operator=(ColorType Other)
    {
        Color = Other;
        return *this;
    }

    FColor& operator=(const ChannelType Value)
    {
        Color = Simd::SetAll<ColorType>(Value);
        return *this;
    }

    explicit operator RGBA8I() const;

    explicit operator RGBA16I() const;

    explicit operator RGBA32F() const;

    ChannelType R() const
    {
        return Color[0];
    }

    ChannelType& R()
    {
        return Simd::ToPtr(&Color)[0];
    }

    ChannelType G() const
    {
        return Color[1];
    }

    ChannelType& G()
    {
        return Simd::ToPtr(&Color)[1];
    }

    ChannelType B() const
    {
        return Color[2];
    }

    ChannelType& B()
    {
        return Simd::ToPtr(&Color)[2];
    }

    ChannelType A() const
    {
        return Color[3];
    }

    ChannelType& A()
    {
        return Simd::ToPtr(&Color)[3];
    }

    ColorType Data() const
    {
        return Color;
    }

    ColorType& Data()
    {
        return Color;
    }

    bool ExactEquals(FColor Other) const;

    //checks if two colors are almost equal with tolerance = 0.0001
    bool operator==(FColor Other) const;

    INLINE bool operator!=(FColor Other) const
    {
        return !(*this == Other);
    }

    ChannelType operator[](TypeTrait::IsInteger auto Index) const
    {
        ASSERT(Index >= 0 && Index <= 3);
        return Color[Index];
    }

    ChannelType& operator[](TypeTrait::IsInteger auto Index)
    {
        ASSERT(Index >= 0 && Index <= 3);
        return Simd::ToPtr(&Color)[Index];
    }

    INLINE FColor operator+(FColor Other) const
    {
        return FColor{Color + Other.Color};
    }

    INLINE FColor operator+(ChannelType Value) const
    {
        return FColor{Color + Value};
    }

    INLINE FColor& operator+=(FColor Other)
    {
        Color += Other.Color;
        return *this;
    }

    INLINE FColor& operator+=(ChannelType Value)
    {
        Color += Value;
        return *this;
    }

    INLINE FColor operator-(FColor Other) const
    {
        return FColor{Color - Other.Color};
    }

    INLINE FColor operator-(ChannelType Value) const
    {
        return FColor{Color - Value};
    }

    INLINE FColor& operator-=(FColor Other)
    {
        Color -= Other.Color;
        return *this;
    }

    INLINE FColor& operator-=(ChannelType Value)
    {
        Color -= Value;
        return *this;
    }

    INLINE FColor operator*(FColor Other) const
    {
        return FColor{Color * Other.Color};
    }

    INLINE FColor operator*(ChannelType Value) const
    {
        return FColor{Color * Value};
    }

    INLINE FColor& operator*=(FColor Other)
    {
        Color *= Other.Color;
        return *this;
    }

    INLINE FColor& operator*=(ChannelType Value)
    {
        Color *= Value;
        return *this;
    }

    INLINE FColor operator/(FColor Other) const
    {
        return FColor{Color / Other.Color};
    }

    INLINE FColor operator/(ChannelType Value) const
    {
        return FColor{Color / Value};
    }

    INLINE FColor& operator/=(FColor Other)
    {
        Color /= Other.Color;
        return *this;
    }

    INLINE FColor& operator/=(ChannelType Value)
    {
        Color /= Value;
        return *this;
    }

private:

    ColorType Color;
};
