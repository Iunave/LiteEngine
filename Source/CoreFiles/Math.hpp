#pragma once

#include "Definitions.hpp"
#include "TypeTraits.hpp"
#include "Assert.hpp"

class FVector;

namespace Math
{

    template<typename T>
    inline constexpr T PI{static_cast<T>(3.141592653589793238462643383279502884197169399375105820974944592307816406286)};

    template<typename T>
    inline constexpr T e{static_cast<T>(2.71828182845904523536028747135266249775724709369995)};

    inline constexpr uint64 Float64One{0b0011111111110000000000000000000000000000000000000000000000000000};
    inline constexpr uint32 Float32One{0b00111111100000000000000000000000};

    //used to `and` away the sign
    template<typename T> requires(std::is_signed_v<T>)
    inline consteval T SignMask()
    {
        return T{-1} >> 1;
    }

    template<typename T>
    inline constexpr T Absolute(T Arg)
    {
        if constexpr(std::is_signed_v<T>)
        {
            return Arg > 0 ? Arg : Arg * -1;
        }
        else
        {
            return Arg;
        }
    }

    template<typename T>
    inline consteval T Smallest()
    {
        if constexpr(std::is_integral_v<T>)
        {
            return T{1};
        }
        else if constexpr(sizeof(T) == 4)
        {
            return __builtin_nextafterf(0.F, 1.F);
        }
        else if constexpr(sizeof(T) == 8)
        {
            return __builtin_nextafter(0.0, 1.0);
        }
        else if constexpr(sizeof(T) == 16)
        {
            return __builtin_nextafterl(0.0, 1.0);
        }
    }

    template<typename T>
    inline consteval T Largest()
    {
        if constexpr(sizeof(T) == 1)
        {
            return 0xFF;
        }
        else if constexpr(sizeof(T) == 2)
        {
            return 0xFFFF;
        }
        else if constexpr(sizeof(T) == 4)
        {
            return 0xFFFFFFFF;
        }
        else if constexpr(sizeof(T) == 8)
        {
            return 0xFFFFFFFFFFFFFFFF;
        }
    }

    template<typename T> requires(std::is_signed_v<T>)
    inline T SignExtend(T Arg)
    {
        return Arg >> ((8 * sizeof(T)) - 1);
    }

    template<typename T>
    inline int32 NumActiveBits(const T Arg)
    {
        if constexpr(sizeof(T) <= 4)
        {
            return __builtin_popcount(Arg);
        }
        else if constexpr(sizeof(T) == 8)
        {
            return __builtin_popcountll(Arg);
        }
    }

    template<typename T>
    inline constexpr T Factorial(T StartNum) //!StartNum
    {
        T Result{StartNum};

        #pragma unroll
        while(StartNum > static_cast<T>(2))
        {
            --StartNum;
            Result *= StartNum;
        }

        return Result;
    }

    template<typename... T>
    inline constexpr auto Average(T... Values)
    {
        return (Values + ...) / sizeof...(T);
    }

    //count the amount of zero bits starting from the most significant bit untill one is set
    template<typename T>
    inline constexpr int32 CountLeadingZeroes(T Arg)
    {
        if constexpr(sizeof(Arg) <= 4)
        {
            return __builtin_clz(Arg);
        }
        else if constexpr(sizeof(Arg) == 8)
        {
            return __builtin_clzll(Arg);
        }
    }

    template<typename T>
    inline constexpr int32 CountTrailingZeroes(T Arg)
    {
        if constexpr(sizeof(Arg) <= 4)
        {
            return __builtin_ctz(Arg);
        }
        else if constexpr(sizeof(Arg) == 8)
        {
            return __builtin_ctzll(Arg);
        }
    }

    /**
     * searches for the first bit that is 1 starting from the LSB
     * \returns the index of the first set bit + 1
     */
    template<typename T> requires(std::is_integral_v<T>)
    inline constexpr int32 FindFirstSet(T Arg)
    {
        if constexpr(sizeof(Arg) <= 4)
        {
            return __builtin_ffs(Arg);
        }
        else if constexpr(sizeof(Arg) == 8)
        {
            return __builtin_ffsll(Arg);
        }
    }

    template<typename T>
    inline T Clamp(const T Arg, const T Min, const T Max)
    {
        return (Arg < Min) ? Min : (Max < Arg) ? Max : Min;
    }

    template<typename T>
    inline constexpr T Max(const T Left, const T Right)
    {
        return Left > Right ? Left : Right;
    }

    template<typename T>
    inline constexpr T Min(const T Left, const T Right)
    {
        return Left > Right ? Right : Left;
    }

    template<typename T>
    inline constexpr T ToPower(const T Arg, const T Num)
    {
        if constexpr(sizeof(T) == 4)
        {
            return __builtin_powf(Arg, Num);
        }
        else if constexpr(sizeof(T) > 4)
        {
            return __builtin_powl(Arg, Num);
        }
    }

    template<typename T>
    inline constexpr T SquareRoot(const T Arg)
    {
        if constexpr(sizeof(T) == 4)
        {
            return __builtin_sqrtf(Arg);
        }
        else if constexpr(sizeof(T) > 4)
        {
            return __builtin_sqrtl(Arg);
        }
    }

    template<typename T>
    inline constexpr T ToRoot(const T Arg, const T Num)
    {
        return T{1} / ToPower(Arg, Num);
    }

    template<typename T>
    inline T Cosine(const T Arg)
    {
        if constexpr(sizeof(T) == 4)
        {
            return __builtin_cosf(Arg);
        }
        else if constexpr(sizeof(T) > 4)
        {
            return __builtin_cosl(Arg);
        }
    }

    template<typename T>
    inline T ArchCosine(const T Arg)
    {
        if constexpr(sizeof(T) == 4)
        {
            return __builtin_acosf(Arg);
        }
        else if constexpr(sizeof(T) > 4)
        {
            return __builtin_acosl(Arg);
        }
    }

    template<typename T>
    inline T Sine(const T Arg)
    {
        if constexpr(sizeof(T) == 4)
        {
            return __builtin_sinf(Arg);
        }
        else if constexpr(sizeof(T) > 4)
        {
            return __builtin_sinl(Arg);
        }
    }

    template<typename T>
    inline T ArchSine(const T Arg)
    {
        if constexpr(sizeof(T) == 4)
        {
            return __builtin_asinf(Arg);
        }
        else if constexpr(sizeof(T) > 4)
        {
            return __builtin_asinl(Arg);
        }
    }

    template<typename T>
    inline T Tangent(const T Arg)
    {
        if constexpr(sizeof(T) == 4)
        {
            return __builtin_tanf(Arg);
        }
        else if constexpr(sizeof(T) > 4)
        {
            return __builtin_tanl(Arg);
        }
    }

    template<typename T>
    inline T ArchTangent(const T Arg)
    {
        if constexpr(sizeof(T) == 4)
        {
            return __builtin_atanf(Arg);
        }
        else if constexpr(sizeof(T) > 4)
        {
            return __builtin_atanl(Arg);
        }
    }

    inline int32 CountTrailingZeroes(TypeTrait::IsInteger auto Arg)
    {
        ASSERT(Arg != 0);

        if constexpr(sizeof(Arg) <= 4)
        {
            return __builtin_ctz(Arg);
        }
        else if constexpr(sizeof(Arg) == 8)
        {
            return __builtin_ctzll(Arg);
        }
    }

    // new ptr = ptr + PointerOffset
    template<typename Derived, typename Base>
    inline int64 PointerOffset()
    {
        using UnqualifiedDerived = TypeTrait::RemoveCV<Derived>;
        using UnqualifiedBase = TypeTrait::RemoveCV<Base>;

        UnqualifiedDerived* DerivedPointer{reinterpret_cast<UnqualifiedDerived*>(alignof(UnqualifiedBase))};
        UnqualifiedBase* BasePointer{static_cast<UnqualifiedBase*>(DerivedPointer)};

        return reinterpret_cast<int64>(BasePointer) - reinterpret_cast<int64>(DerivedPointer);
    }

    template<typename T> requires(TypeTrait::IsInteger<T>)
    inline T AddIf(const T Left, const T Right, bool Condition)
    {
        return Left + (Right & -static_cast<TypeTrait::AddUnsigned<T>>(Condition));
    }

    template<typename T> requires(TypeTrait::IsInteger<T>)
    inline T SubIf(const T Left, const T Right, bool Condition)
    {
        return Left - (Right & -static_cast<TypeTrait::AddUnsigned<T>>(Condition));
    }

    template<typename T>
    inline bool AddCheckOverflow(const T Left, const T Right, T* OutValue)
    {
        if constexpr(TypeTrait::IsInteger<T>)
        {
            return __builtin_add_overflow(Left, Right, OutValue);
        }
        else if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            *OutValue = (Left + Right);
            return *OutValue == std::numeric_limits<T>::infinity();
        }
    }

    template<typename T>
    inline bool SubCheckOverflow(const T Left, const T Right, T* OutValue)
    {
        if constexpr(TypeTrait::IsInteger<T>)
        {
            return __builtin_sub_overflow(Left, Right, OutValue);
        }
        else if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            *OutValue = (Left - Right);
            return *OutValue == std::numeric_limits<T>::infinity();
        }
    }

    template<typename T>
    inline bool MulCheckOverflow(const T Left, const T Right, T* OutValue)
    {
        if constexpr(TypeTrait::IsInteger<T>)
        {
            return __builtin_mul_overflow(Left, Right, OutValue);
        }
        else if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            *OutValue = (Left * Right);
            return *OutValue == std::numeric_limits<T>::infinity();
        }
    }

    template<typename ChoiceType, typename... TChoices>
    inline ChoiceType ChooseVar(const uint64 Condition, TChoices... Choices)
    {
        struct FChoiceWrapper
        {
            const ChoiceType Choices[sizeof...(Choices)];
        };

        return FChoiceWrapper{Choices...}.Choices[Condition];
    }

    template<typename T>
    inline constexpr T DistanceAbsolute(T First, T Second)
    {
        return Min(Second - First, First - Second);
    }

    bool IsInSphere(float64 SphereRadius, FVector SpherePosition, FVector TargetPosition);

    uint64 GenerateRandom(uint64 Min = 0, uint64 Max = UINT64_MAX);
}
