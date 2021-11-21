#include "Color.hpp"
#include "Math.hpp"
#include "String.hpp"

#include <fmt/core.h>

const constinit FColor FColor::Red{1.0, 0.0, 0.0};
const constinit FColor FColor::Yellow{1.0, 1.0, 0.0};
const constinit FColor FColor::Green{0.0, 1.0, 0.0};
const constinit FColor FColor::Turquoise{0.0, 1.0, 1.0};
const constinit FColor FColor::Blue{0.0, 0.0, 1.0};
const constinit FColor FColor::Pink{1.0, 0.0, 1.0};
const constinit FColor FColor::Black{0.0, 0.0, 0.0};
const constinit FColor FColor::White{1.0, 1.0, 1.0};

constexpr float64_4 MaxColorValue8{Simd::SetAll<float64_4>(Math::Largest<uint8>())};
constexpr float64_4 MaxColorValue16{Simd::SetAll<float64_4>(Math::Largest<uint16>())};

FColor::FColor(RGBA8I InColor)
{
    Color = Simd::ConvertVector<float64_4>(*reinterpret_cast<uint8_4*>(&InColor));
    Color /= MaxColorValue8;
}

FColor::FColor(RGBA16I InColor)
{
    Color = Simd::ConvertVector<float64_4>(*reinterpret_cast<uint16_4*>(&InColor));
    Color /= MaxColorValue16;
}

FColor::FColor(RGBA32F InColor)
    : Color{Simd::ConvertVector<float64_4>(*reinterpret_cast<float32_4*>(&InColor))}
{
}

bool FColor::ExactEquals(FColor Other) const
{
    const int32 ComparisonResult{Simd::MoveMask(Color == Other.Color)};
    return ComparisonResult == ColorMask;
}

bool FColor::operator==(FColor Other) const
{
    const float64_4 SubResult{static_cast<int64_4>(Color - Other.Color) & Math::SignMask<int64>()};
    const int32 ComparisonResult{Simd::MoveMask(SubResult <= 0.0001)};

    return ComparisonResult == ColorMask;
}

FColor::operator RGBA8I() const
{
    const float64_4 Multiplied{Color * MaxColorValue8};
    const uint8_4 ConvertedVector{Simd::ConvertVector<uint8_4>(Multiplied)};

    RGBA8I Result;
    *reinterpret_cast<uint8_4*>(&Result) = ConvertedVector;
    return Result;
}

FColor::operator RGBA16I() const
{
    const float64_4 Multiplied{Color * MaxColorValue16};
    const uint16_4 ConvertedVector{Simd::ConvertVector<uint16_4>(Multiplied)};

    RGBA16I Result;
    *reinterpret_cast<uint16_4*>(&Result) = ConvertedVector;
    return Result;
}

FColor::operator RGBA32F() const
{
    const float32_4 ConvertedVector{BUILTIN(cvtpd2ps256)(Color)};

    RGBA32F Result;
    *reinterpret_cast<float32_4*>(&Result) = ConvertedVector;
    return Result;
}

FString<SS124> StrUtl::ToString(FColor Source)
{
    FString<SS124> String{};

    const char8* End{fmt::format_to(String.Data(), "R={:+f} G={:+f} B={:+f} A={:+f}", Source.R(), Source.G(), Source.B(), Source.A())};
    String.SetTerminatorIndex(static_cast<uint32>(End - String.Data()));

    return String;
}

