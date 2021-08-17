#pragma once

#if __has_include("Definitions.hpp")
#include "Definitions.hpp"
#else
using char8 = char;
using char16 = char16_t;
using char32 = char32_t;
using uint8 = unsigned char;
using int8 = signed char;
using uint16 = unsigned short;
using int16 = signed short;
using uint32 = unsigned int;
using int32 = signed int;
using uint64 = unsigned long long;
using int64 = signed long long;
using int128 = __int128_t;
using uint128 = __uint128_t;
using float32 = float;
using float64 = double;
using float128 = long double;
#endif

#if __has_include("TypeTraits.hpp")
#include "TypeTraits.hpp"
#else
#include <type_traits>
namespace TypeTrait
{
    template<typename T>
    concept IsInteger = requires()
    {
        std::is_integral<T>::value;
    };

    template<typename T>
    concept IsFloatingPoint = requires()
    {
        std::is_floating_point_v<T>;
    };

    template<typename T>
    concept IsSigned = requires()
    {
        std::is_signed<T>::value;
    };

    template<uint64... Ns>
    struct IndexSequence{};

    template<uint64 N, uint64... Is>
    auto MakeIndexSequenceImpl()
    {
        if constexpr(N == 0)
        {
            return IndexSequence<Is...>();
        }
        else
        {
            return MakeIndexSequenceImpl<N - 1, N - 1, Is...>();
        }
    }

    template<uint64 N>
    using MakeIndexSequence = typename std::decay<decltype(MakeIndexSequenceImpl<N>())>::type;

    template<typename T>
    using Pure = std::remove_cv_t<std::remove_reference_t<T>>;
}
#endif

#if __has_include("Memory.hpp")
#include "Memory.hpp"
#else
namespace Memory
{
    template<typename TargetType, typename SourceType>
    INLINE constexpr void Copy(TargetType* Destination, const SourceType* Source, const uint64 Num)
    {
        __builtin_memcpy(Destination, Source, Num);
    }
}
#endif

#ifdef __AVX__
#define AVX128 __AVX__
#endif
#ifdef __AVX2__
#define AVX256 __AVX2__
#endif
#ifdef __AVX512F__
#define AVX512 __AVX512__ //not yet implemented
#endif

#ifndef BUILTIN
#define BUILTIN(name) __builtin_ia32_##name
#endif

#ifndef MIN_VECTOR_WIDTH
#define MIN_VECTOR_WIDTH(width) __attribute__((min_vector_width(width)))
#endif

#ifndef INLINE
#define INLINE __attribute__((always_inline))
#endif

#ifndef ASSUME_ALIGNED
#define ASSUME_ALIGNED(address, alignment) __builtin_assume_aligned(address, alignment)
#endif


template<typename T>
using Vector16 = T __attribute__((vector_size(16), aligned(16)));

template<typename T>
using Vector32 = T __attribute__((vector_size(32), aligned(32)));

template<typename T>
using Vector64 = T __attribute__((vector_size(64), aligned(64)));

#ifdef AVX128

using char8_16 = Vector16<char8>;
static_assert(alignof(char8_16) == 16);

using char16_8 = Vector16<char16>;
static_assert(alignof(char16_8) == 16);

using char32_4 = Vector16<char32>;
static_assert(alignof(char32_4) == 16);

using uint8_16 = Vector16<uint8>;
static_assert(alignof(uint8_16) == 16);

using int8_16 = Vector16<int8>;
static_assert(alignof(int8_16) == 16);

using uint16_8 = Vector16<uint16>;
static_assert(alignof(uint16_8) == 16);

using int16_8 = Vector16<int16>;
static_assert(alignof(int16_8) == 16);

using uint32_4 = Vector16<uint32>;
static_assert(alignof(uint32_4) == 16);

using int32_4 = Vector16<int32>;
static_assert(alignof(int32_4) == 16);

using uint64_2 = Vector16<uint64>;
static_assert(alignof(uint64_2) == 16);

using int64_2 = Vector16<int64>;
static_assert(alignof(int64_2) == 16);

using float32_4 = Vector16<float32>;
static_assert(alignof(float32_4) == 16);

using float64_2 = Vector16<float64>;
static_assert(alignof(float64_2) == 16);

#endif //AVX128
#ifdef AVX256

using char8_32 = Vector32<char8>;
static_assert(alignof(char8_32) == 32);

using char16_16 = Vector32<char16>;
static_assert(alignof(char16_16) == 32);

using char32_8 = Vector32<char32>;
static_assert(alignof(char32_8) == 32);

using uint8_32 = Vector32<uint8>;
static_assert(alignof(uint8_32) == 32);

using int8_32 = Vector32<int8>;
static_assert(alignof(int8_32) == 32);

using uint16_16 = Vector32<uint16>;
static_assert(alignof(uint16_16) == 32);

using int16_16 = Vector32<int16>;
static_assert(alignof(int16_16) == 32);

using uint32_8 = Vector32<uint32>;
static_assert(alignof(uint32_8) == 32);

using int32_8 = Vector32<int32>;
static_assert(alignof(int32_8) == 32);

using uint64_4 = Vector32<uint64>;
static_assert(alignof(uint64_4) == 32);

using int64_4 = Vector32<int64>;
static_assert(alignof(int64_4) == 32);

using uint128_2 = Vector32<uint128>;
static_assert(alignof(uint128_2) == 32);

using int128_2 = Vector32<int128>;
static_assert(alignof(int128_2) == 32);

using float32_8 = Vector32<float32>;
static_assert(alignof(float32_8) == 32);

using float64_4 = Vector32<float64>;
static_assert(alignof(float64_4) == 32);

using float128_2 = Vector32<float128>;
static_assert(alignof(float128_2) == 32);

#endif //AVX256
#ifdef AVX512

using char8_64 = Vector64<char8>;
static_assert(alignof(char8_64) == 64);

using char16_32 = Vector64<char16>;
static_assert(alignof(char16_32) == 64);

using char32_16 = Vector64<char32>;
static_assert(alignof(char32_16) == 64);

using uint8_64 = Vector64<uint8>;
static_assert(alignof(uint8_64) == 64);

using int8_64 = Vector64<int8>;
static_assert(alignof(int8_64) == 64);

using uint16_32 = Vector64<uint16>;
static_assert(alignof(uint16_32) == 64);

using int16_32 = Vector64<int16>;
static_assert(alignof(int16_32) == 64);

using uint32_16 = Vector64<uint32>;
static_assert(alignof(uint32_16) == 64);

using int32_16 = Vector64<int32>;
static_assert(alignof(int32_16) == 64);

using uint64_8 = Vector64<uint64>;
static_assert(alignof(uint64_8) == 64);

using int64_8 = Vector64<int64>;
static_assert(alignof(int64_8) == 64);

using uint128_4 = Vector64<uint128>;
static_assert(alignof(uint128_4) == 64);

using int128_4 = Vector64<int128>;
static_assert(alignof(int128_4) == 64);

using float32_16 = Vector64<float32>;
static_assert(alignof(float32_16) == 64);

using float64_8 = Vector64<float64>;
static_assert(alignof(float64_8) == 64);

using float128_4 = Vector64<float128>;
static_assert(alignof(float128_4) == 64);

#endif //AVX512

namespace Simd
{

    template<typename RegisterType>
    using ElementType = TypeTrait::Pure<decltype(RegisterType{}[0])>;

    template<typename RegisterType>
    inline consteval uint64 ElementSize()
    {
        return sizeof(ElementType<RegisterType>);
    }

    template<typename RegisterType>
    inline consteval uint64 NumElements()
    {
        return alignof(RegisterType) / ElementSize<RegisterType>();
    }

    INLINE void ZeroUpper()
    {
#ifdef AVX256
        BUILTIN(vzeroupper)();
#endif
    }

    INLINE void ZeroAll()
    {
#ifdef AVX256
        BUILTIN(vzeroall)();
#endif
    }

    template<typename RegisterType>
    INLINE constexpr RegisterType SetAll(ElementType<RegisterType> Value)
    {
        auto FillValues = [Value]<uint64... I>(TypeTrait::IndexSequence<I...>) -> RegisterType
        {
            return RegisterType{((void)I, Value)...};
        };

        return FillValues(TypeTrait::MakeIndexSequence<NumElements<RegisterType>()>{});
    }

#ifdef AVX128

    NODISCARD INLINE uint8_16 CopyOrZero(const uint8_16 Source, const uint8_16 Mask)
    {
        return BUILTIN(pshufb128)(Source, Mask);
    }

#endif
#ifdef AVX256

    NODISCARD INLINE uint8_32 CopyOrZero(const uint8_32 Source, const uint8_32 Mask)
    {
        return BUILTIN(pshufb256)(Source, Mask);
    }

#endif
#ifdef AVX128

    template<typename RegisterType> requires(alignof(RegisterType) == 16)
    inline consteval int32 Mask()
    {
        if constexpr(NumElements<RegisterType>() == 16)
        {
            return static_cast<int32>(0b00000000000000001111111111111111);
        }
        else if constexpr(NumElements<RegisterType>() == 8)
        {
            return static_cast<int32>(0b00000000000000000000000011111111);
        }
        else if constexpr(NumElements<RegisterType>() == 4)
        {
            return static_cast<int32>(0b00000000000000000000000000001111);
        }
    }

#endif //AVX128
#ifdef AVX256

    template<typename RegisterType> requires(alignof(RegisterType) == 32)
    inline consteval int32 Mask()
    {
        if constexpr(NumElements<RegisterType>() == 32)
        {
            return static_cast<int32>(0b11111111111111111111111111111111);
        }
        else if constexpr(NumElements<RegisterType>() == 16)
        {
            return static_cast<int32>(0b11111111111111111111111111111111);
        }
        else if constexpr(NumElements<RegisterType>() == 8)
        {
            return static_cast<int32>(0b00000000000000000000000011111111);
        }
        else if constexpr(NumElements<RegisterType>() == 4)
        {
            return static_cast<int32>(0b00000000000000000000000000001111);
        }
    }

#endif //AVX256
#ifdef AVX512

    template<typename RegisterType> requires(alignof(RegisterType) == 64)
    inline consteval auto Mask()
    {
        if constexpr(NumElements<RegisterType>() == 64)
        {
            return static_cast<uint64>(~0ULL);
        }
        else if constexpr(NumElements<RegisterType>() == 32)
        {
            return static_cast<uint32>(~0U);
        }
        else if constexpr(NumElements<RegisterType>() == 16)
        {
            return static_cast<uint16>(~0U);
        }
        else if constexpr(NumElements<RegisterType>() == 8)
        {
            return static_cast<uint8>(~0U);
        }
    }

#endif //AVX512
#ifdef AVX128

    template<typename T>
    INLINE constexpr int32 MoveMask(Vector16<T> VectorMask) MIN_VECTOR_WIDTH(128)
    {
        if constexpr(sizeof(T) <= 2)
        {
            return BUILTIN(pmovmskb128)(VectorMask); //no movemask function exist for words actual mask looks like 0b01010101010101010101010101010101
        }
        else if constexpr(sizeof(T) == 4)
        {
            return BUILTIN(movmskps)(VectorMask);
        }
        else if constexpr(sizeof(T) == 8)
        {
            return BUILTIN(movmskpd)(VectorMask);
        }
    }

#endif //AVX128
#ifdef AVX256

    template<typename T>
    INLINE constexpr int32 MoveMask(Vector32<T> VectorMask) MIN_VECTOR_WIDTH(256)
    {
        if constexpr(sizeof(T) <= 2)
        {
            return BUILTIN(pmovmskb256)(VectorMask); //no movemask function exist for words
        }
        else if constexpr(sizeof(T) == 4)
        {
            return BUILTIN(movmskps256)(VectorMask);
        }
        else if constexpr(sizeof(T) == 8)
        {
            return BUILTIN(movmskpd256)(VectorMask);
        }
    }

#endif //AVX256
#ifdef AVX512

    template<typename T>
    INLINE constexpr auto MoveMask(Vector64<T> VectorMask) MIN_VECTOR_WIDTH(512)
    {
        if constexpr(sizeof(T) == 1)
        {
            return BUILTIN(cvtb2mask512)(VectorMask);
        }
        else if constexpr(sizeof(T) == 2)
        {
            return BUILTIN(cvtw2mask512)(VectorMask);
        }
        else if constexpr(sizeof(T) == 4)
        {
            return BUILTIN(cvtd2mask512)(VectorMask);
        }
        else if constexpr(sizeof(T) == 8)
        {
            return BUILTIN(cvtq2mask512)(VectorMask);
        }
    }

#endif //AVX512

    template<typename RegisterType>
    INLINE RegisterType MoveRegister(const int32 IntegerMask) //todo
    {
        if constexpr(alignof(RegisterType) == 16)
        {
            if constexpr(sizeof(ElementSize<RegisterType>()) == 1)
            {
                const int64_2 Shuffle{0x0101010101010101, 0x0000000000000000};
                const int64_2 BitMask{Simd::SetAll<int64_2>(0x7fbfdfeff7fbfdfe)};

                int32_4 VectorMask{Simd::SetAll<int32_4>(IntegerMask)};

                VectorMask = CopyOrZero(VectorMask, Shuffle);
                VectorMask |= BitMask;

                return VectorMask == Simd::SetAll<int32_4>(-1);
            }
        }
        else if constexpr(alignof(RegisterType) == 32)
        {
            if constexpr(ElementSize<RegisterType>())
            {
                const int64_4 ShuffleVector{0x0303030303030303, 0x0202020202020202, 0x0101010101010101, 0x0000000000000000};
                const int64_4 BitMask{Simd::SetAll<int64_4>(0x7fbfdfeff7fbfdfe)};

                int32_8 VectorMask{Simd::SetAll<int32_8>(IntegerMask)};

                VectorMask = CopyOrZero(VectorMask, ShuffleVector);
                VectorMask |= BitMask;

                return VectorMask == Simd::SetAll<int32_8>(-1);
            }
        }
    }

#ifdef AVX128

    template<typename T> requires(sizeof(T) >= 4)
    INLINE constexpr Vector16<T> MaskLoad(const Vector16<T>* const Value, Vector16<T> Mask) MIN_VECTOR_WIDTH(128)
    {
        if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskloadps)(Value, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskloadpd)(Value, Mask);
            }
        }
        else if constexpr(TypeTrait::IsInteger<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskloadd)(Value, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskloadq)(Value, Mask);
            }
        }
    }

#endif //AVX128
#ifdef AVX256

    template<typename T> requires(sizeof(T) >= 4)
    INLINE constexpr Vector32<T> MaskLoad(const Vector32<T>* const Value, Vector32<T> Mask) MIN_VECTOR_WIDTH(256)
    {
        if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskloadps256)(Value, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskloadpd256)(Value, Mask);
            }
        }
        else if constexpr(TypeTrait::IsInteger<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskloadd256)(Value, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskloadq256)(Value, Mask);
            }
        }
    }

#endif //AVX256

    template<int32... Control, typename RegisterType>
    INLINE constexpr RegisterType ShuffleVector(RegisterType Source)
    {
        return RegisterType{__builtin_shufflevector(Source, Source, Control...)};
    }

    template<int32... Control, typename RegisterType, typename TargetType = RegisterType>
    INLINE constexpr TargetType ShuffleVector(RegisterType SourceOne, RegisterType SourceTwo)
    {
        return TargetType{__builtin_shufflevector(SourceOne, SourceTwo, Control...)};
    }

    template<typename TargetType, typename RegisterType> requires(NumElements<TargetType>() == NumElements<RegisterType>())
    INLINE constexpr TargetType ConvertVector(RegisterType Source)
    {
        return TargetType{__builtin_convertvector(Source, TargetType)};
    }

#ifdef AVX256

    template<typename T, uint8 Control>
    INLINE constexpr Vector16<T> Extract(Vector32<T> Source)
    {
        if constexpr(TypeTrait::IsFloatingPoint<T> && sizeof(T) == 4)
        {
            return BUILTIN(vextractf128_ps256)(Source, Control);
        }
        else if constexpr(TypeTrait::IsFloatingPoint<T> && sizeof(T) == 8)
        {
            return BUILTIN(vextractf128_pd256)(Source, Control);
        }
        else if constexpr(TypeTrait::IsInteger<T>)
        {
            return BUILTIN(extract128i256)(Source, Control);
        }
    }

#endif //AVX256
#ifdef AVX128

    /// LeftToMul * RightToMul + ToAdd
    template<typename T> requires(sizeof(T) >= 4)
    INLINE constexpr Vector16<T> FusedMultiplyAdd(Vector16<T> LeftToMul, Vector16<T> RightToMul, Vector16<T> ToAdd) MIN_VECTOR_WIDTH(128)
    {
        if constexpr(sizeof(T) == 4)
        {
            return BUILTIN(vfmaddps)(LeftToMul, RightToMul, ToAdd);
        }
        else if constexpr(sizeof(T) == 8)
        {
            return BUILTIN(vfmaddpd)(LeftToMul, RightToMul, ToAdd);
        }
    }

#endif //AVX128
#ifdef AVX256

    /// LeftToMul * RightToMul + ToAdd
    template<typename T> requires(sizeof(T) >= 4)
    INLINE constexpr Vector32<T> FusedMultiplyAdd(Vector32<T> LeftToMul, Vector32<T> RightToMul, Vector32<T> ToAdd) MIN_VECTOR_WIDTH(256)
    {
        if constexpr(sizeof(T) == 4)
        {
            return BUILTIN(vfmaddps256)(LeftToMul, RightToMul, ToAdd);
        }
        else if constexpr(sizeof(T) == 8)
        {
            return BUILTIN(vfmaddpd256)(LeftToMul, RightToMul, ToAdd);
        }
    }

    /// LeftToMul * RightToMul - ToSub
    template<typename T> requires(TypeTrait::IsSigned<T>)
    INLINE constexpr Vector32<T> FusedMultiplySubtract(Vector32<T> LeftToMul, Vector32<T> RightToMul, Vector32<T> ToSub) MIN_VECTOR_WIDTH(256)
    {
        return FusedMultiplyAdd(LeftToMul, RightToMul, ToSub * SetAll<Vector32<T>>(-1));
    }

#endif //AVX256
#ifdef AVX128

    template<typename T>
    INLINE constexpr Vector16<T> MakeFromGreater(Vector16<T> Left, Vector16<T> Right) MIN_VECTOR_WIDTH(128)
    {
        if constexpr(TypeTrait::IsSigned<T>)
        {
            if constexpr(sizeof(T) == 1)
            {
                return BUILTIN(pmaxsb128)(Left, Right);
            }
            else if constexpr(sizeof(T) == 2)
            {
                return BUILTIN(pmaxsw128)(Left, Right);
            }
            else if constexpr(sizeof(T) == 4)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(maxps)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
                    return BUILTIN(pmaxsd128)(Left, Right);
                }
            }
            else if constexpr(sizeof(T) == 8)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(maxpd)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
#ifdef AVX512
                    return BUILTIN(pmaxsq128)(Left, Right);
#endif
                }
            }
        }
        else
        {
            if constexpr(sizeof(T) == 1)
            {
                return BUILTIN(pmaxub128)(Left, Right);
            }
            else if constexpr(sizeof(T) == 2)
            {
                return BUILTIN(pmaxuw128)(Left, Right);
            }
            else if constexpr(sizeof(T) == 4)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(maxps)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
                    return BUILTIN(pmaxud128)(Left, Right);
                }
            }
            else if constexpr(sizeof(T) == 8)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(maxpd)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
#ifdef AVX512
                    return BUILTIN(pmaxuq128)(Left, Right);
#endif
                }
            }
        }
    }

#endif //AVX128
#ifdef AVX256

    template<typename T>
    INLINE constexpr Vector32<T> MakeFromGreater(Vector32<T> Left, Vector32<T> Right) MIN_VECTOR_WIDTH(256)
    {
        if constexpr(TypeTrait::IsSigned<T>)
        {
            if constexpr(sizeof(T) == 1)
            {
                return BUILTIN(pmaxsb256)(Left, Right);
            }
            else if constexpr(sizeof(T) == 2)
            {
                return BUILTIN(pmaxsw256)(Left, Right);
            }
            else if constexpr(sizeof(T) == 4)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(maxps)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
                    return BUILTIN(pmaxsd256)(Left, Right);
                }
            }
            else if constexpr(sizeof(T) == 8)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(maxpd256)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
#ifdef AVX512
                    return BUILTIN(pmaxsq256)(Left, Right);
#endif
                }
            }
        }
        else
        {
            if constexpr(sizeof(T) == 1)
            {
                return BUILTIN(pmaxub256)(Left, Right);
            }
            else if constexpr(sizeof(T) == 2)
            {
                return BUILTIN(pmaxuw256)(Left, Right);
            }
            else if constexpr(sizeof(T) == 4)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(maxps256)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
                    return BUILTIN(pmaxud256)(Left, Right);
                }
            }
            else if constexpr(sizeof(T) == 8)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(maxpd256)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
#ifdef AVX512
                    return BUILTIN(pmaxuq256)(Left, Right);
#endif
                }
            }
        }
    }

#endif //AVX256
#ifdef AVX128

    template<typename T>
    INLINE constexpr Vector16<T> MakeFromLesser(Vector16<T> Left, Vector16<T> Right) MIN_VECTOR_WIDTH(128)
    {
        if constexpr(TypeTrait::IsSigned<T>)
        {
            if constexpr(sizeof(T) == 1)
            {
                return BUILTIN(pminsb128)(Left, Right);
            }
            else if constexpr(sizeof(T) == 2)
            {
                return BUILTIN(pminsw128)(Left, Right);
            }
            else if constexpr(sizeof(T) == 4)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(minps)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
                    return BUILTIN(pminsd128)(Left, Right);
                }
            }
            else if constexpr(sizeof(T) == 8)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(minpd)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
#ifdef AVX512
                    return BUILTIN(pminsq128)(Left, Right);
#endif
                }
            }
        }
        else
        {
            if constexpr(sizeof(T) == 1)
            {
                return BUILTIN(pminub128)(Left, Right);
            }
            else if constexpr(sizeof(T) == 2)
            {
                return BUILTIN(pminuw128)(Left, Right);
            }
            else if constexpr(sizeof(T) == 4)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(minps)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
                    return BUILTIN(pminud128)(Left, Right);
                }
            }
            else if constexpr(sizeof(T) == 8)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(minpd)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
#ifdef AVX512
                    return BUILTIN(pminuq128)(Left, Right);
#endif
                }
            }
        }
    }

#endif //AVX128
#ifdef AVX256

    template<typename T>
    INLINE constexpr Vector32<T> MakeFromLesser(Vector32<T> Left, Vector32<T> Right) MIN_VECTOR_WIDTH(256)
    {
        if constexpr(TypeTrait::IsSigned<T>)
        {
            if constexpr(sizeof(T) == 1)
            {
                return BUILTIN(pminsb256)(Left, Right);
            }
            else if constexpr(sizeof(T) == 2)
            {
                return BUILTIN(pminsw256)(Left, Right);
            }
            else if constexpr(sizeof(T) == 4)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(minps)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
                    return BUILTIN(pminsd256)(Left, Right);
                }
            }
            else if constexpr(sizeof(T) == 8)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(minpd256)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
#ifdef AVX512
                    return BUILTIN(pminsq256)(Left, Right);
#endif
                }
            }
        }
        else
        {
            if constexpr(sizeof(T) == 1)
            {
                return BUILTIN(pminub256)(Left, Right);
            }
            else if constexpr(sizeof(T) == 2)
            {
                return BUILTIN(pminuw256)(Left, Right);
            }
            else if constexpr(sizeof(T) == 4)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(minps256)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
                    return BUILTIN(pminud256)(Left, Right);
                }
            }
            else if constexpr(sizeof(T) == 8)
            {
                if constexpr(TypeTrait::IsFloatingPoint<T>)
                {
                    return BUILTIN(minpd256)(Left, Right);
                }
                else if constexpr(TypeTrait::IsInteger<T>)
                {
#ifdef AVX512
                    return BUILTIN(pminuq256)(Left, Right);
#endif
                }
            }
        }
    }

#endif //AVX256
#ifdef AVX128

    template<typename T>
    INLINE constexpr void Clamp(Vector16<T>& Source, Vector16<T> Min, Vector16<T> Max) MIN_VECTOR_WIDTH(128)
    {
        Source = MakeFromGreater(Source, Min);
        Source = MakeFromLesser(Source, Max);
    }

#endif //AVX128
#ifdef AVX256

    template<typename T>
    INLINE constexpr void Clamp(Vector32<T>& Source, Vector32<T> Min, Vector32<T> Max) MIN_VECTOR_WIDTH(256)
    {
        Source = MakeFromGreater(Source, Min);
        Source = MakeFromLesser(Source, Max);
    }

#endif //AVX256
#ifdef AVX128

    template<typename T> requires(TypeTrait::IsSigned<T> && TypeTrait::IsInteger<T>)
    INLINE constexpr Vector16<T> Absolute(Vector16<T> Source) MIN_VECTOR_WIDTH(128)
    {
        if constexpr(sizeof(T) == 1)
        {
            return BUILTIN(pabsb128)(Source);
        }
        else if constexpr(sizeof(T) == 2)
        {
            return BUILTIN(pabsw128)(Source);
        }
        else if constexpr(sizeof(T) == 4)
        {
            return BUILTIN(pabsd128)(Source);
        }
        else if constexpr(sizeof(T) == 8)
        {
#ifdef AVX512
            return BUILTIN(pabsq128)(Source);
#endif
        }
    }

#endif //AVX128
#ifdef AVX256

    template<typename T> requires(TypeTrait::IsSigned<T> && TypeTrait::IsInteger<T>)
    INLINE constexpr Vector32<T> Absolute(Vector32<T> Source) MIN_VECTOR_WIDTH(256)
    {
        if constexpr(sizeof(T) == 1)
        {
            return BUILTIN(pabsb256)(Source);
        }
        else if constexpr(sizeof(T) == 2)
        {
            return BUILTIN(pabsw256)(Source);
        }
        else if constexpr(sizeof(T) == 4)
        {
            return BUILTIN(pabsd256)(Source);
        }
        else if constexpr(sizeof(T) == 8)
        {
#ifdef AVX512
            return BUILTIN(pabsq256)(Source);
#endif
        }
    }

#endif //AVX256
#ifdef AVX128

    template<typename T> requires(sizeof(T) >= 4)
    INLINE Vector16<T> MaskLoad(const T* Source NONNULL, Vector16<T> Mask) MIN_VECTOR_WIDTH(128)
    {
        if constexpr(TypeTrait::IsInteger<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskloadd)(Source, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskloadq)(Source, Mask);
            }
        }
        else if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskloadps)(Source, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskloadpd)(Source, Mask);
            }
        }
    }

#endif //AVX128
#ifdef AVX256

    template<typename T> requires(sizeof(T) >= 4)
    INLINE Vector32<T> MaskLoad(const T* Source NONNULL, Vector32<T> Mask) MIN_VECTOR_WIDTH(256)
    {
        if constexpr(TypeTrait::IsInteger<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskloadd256)(Source, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskloadq256)(Source, Mask);
            }
        }
        else if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskloadps256)(Source, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskloadpd256)(Source, Mask);
            }
        }
    }

#endif //AVX256

    template<typename RegisterType>
    NODISCARD INLINE constexpr RegisterType LoadUnaligned(const ElementType<RegisterType>* RESTRICT NONNULL Data)
    {
        struct PACKED MAY_ALIAS FSource
        {
            RegisterType Register;
        };

        return reinterpret_cast<RegisterType>(reinterpret_cast<const FSource*>(Data)->Register);
    }

    template<typename RegisterType>
    NODISCARD INLINE constexpr RegisterType LoadAligned(const ElementType<RegisterType>* RESTRICT NONNULL Data)
    {
        return *reinterpret_cast<const RegisterType*>(Data);
    }

    template<typename RegisterType>
    INLINE constexpr void StoreUnaligned(ElementType<RegisterType>* Target, const RegisterType Data)
    {
        struct PACKED MAY_ALIAS FSource
        {
            RegisterType Register;
        };

        reinterpret_cast<FSource*>(Target)->Register = Data;
    }

    template<typename RegisterType>
    INLINE constexpr void StoreAligned(ElementType<RegisterType>* Target, const RegisterType Data)
    {
        *reinterpret_cast<RegisterType*>(Target) = Data;
    }

#ifdef AVX128

    template<typename T> requires(sizeof(T) == 4 || sizeof(T) == 8)
    INLINE constexpr void MaskStore(Vector16<T>* Target, const Vector16<T> Mask, const Vector16<T> Data) MIN_VECTOR_WIDTH(128)
    {
        if constexpr(TypeTrait::IsInteger<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                BUILTIN(maskstored)(Target, Mask, Data);
            }
            else if constexpr(sizeof(T) == 8)
            {
                BUILTIN(maskstoreq)(Target, Mask, Data);
            }
        }
        else if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                BUILTIN(maskstoreps)(Target, Mask, Data);
            }
            else if constexpr(sizeof(T) == 8)
            {
                BUILTIN(maskstorepd)(Target, Mask, Data);
            }
        }
    }

#endif //AVX128
#ifdef AVX256

    template<typename T> requires(sizeof(T) == 4 || sizeof(T) == 8)
    INLINE constexpr void MaskStore(Vector32<T>* Target, const Vector32<T> Mask, const Vector32<T> Data) MIN_VECTOR_WIDTH(256)
    {
        if constexpr(TypeTrait::IsInteger<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                BUILTIN(maskstored256)(Target, Mask, Data);
            }
            else if constexpr(sizeof(T) == 8)
            {
                BUILTIN(maskstoreq256)(Target, Mask, Data);
            }
        }
        else if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                BUILTIN(maskstoreps256)(Target, Mask, Data);
            }
            else if constexpr(sizeof(T) == 8)
            {
                BUILTIN(maskstorepd256)(Target, Mask, Data);
            }
        }
    }

#endif //AVX256
#ifdef AVX128

    template<typename T> requires(sizeof(T) == 4 || sizeof(T) == 8)
    NODISCARD INLINE constexpr Vector16<T> MaskLoad(const Vector16<T>* Source, const Vector16<T> Mask) MIN_VECTOR_WIDTH(128)
    {
        if constexpr(TypeTrait::IsInteger<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskloadd)(Source, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskloadq)(Source, Mask);
            }
        }
        else if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskloadps)(Source, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskloadpd)(Source, Mask);
            }
        }
    }

#endif //AVX128
#ifdef AVX256

    template<typename T> requires(sizeof(T) == 4 || sizeof(T) == 8)
    NODISCARD INLINE constexpr Vector32<T> MaskLoad(const Vector32<T>* Source, const Vector32<T> Mask) MIN_VECTOR_WIDTH(256)
    {
        if constexpr(TypeTrait::IsInteger<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskstored256)(Source, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskstoreq256)(Source, Mask);
            }
        }
        else if constexpr(TypeTrait::IsFloatingPoint<T>)
        {
            if constexpr(sizeof(T) == 4)
            {
                return BUILTIN(maskstoreps256)(Source, Mask);
            }
            else if constexpr(sizeof(T) == 8)
            {
                return BUILTIN(maskstorepd256)(Source, Mask);
            }
        }
    }

#endif //AVX256

    /*
     * copies and moves the elements  in the Source by ShuffleAmount to the left
     * ShuffleLeft({0, 1, 1, 0, 1, 0, 1, 1}, 3) == {0, 1, 0, 1, 1, 0, 0, 0}
     */
    template<typename RegisterType, typename ElementType = ElementType<RegisterType>>
    NODISCARD INLINE RegisterType ShuffleLeft(RegisterType Source, const int32 ShuffleAmount)
    {
        static constexpr uint64 NumElements{Simd::NumElements<RegisterType>()};
        static constexpr uint64 StackSize{sizeof(ElementType) * NumElements * 3};

        ElementType* StackBuffer{static_cast<ElementType*>(STACK_ALLOCATE(StackSize))};
        Memory::Set(StackBuffer, 0, StackSize);

        StoreUnaligned(&StackBuffer[NumElements], Source);
        return LoadUnaligned<RegisterType>(&StackBuffer[NumElements - ShuffleAmount]);
    }

    /*
     * copies and moves the elements  in the Source by ShuffleAmount to the right
     * ShuffleRight({0, 1, 1, 0, 1, 0, 1, 1}, 3) == {0, 0, 0, 0, 1, 1, 0, 1}
     */
    template<typename RegisterType>
    NODISCARD INLINE RegisterType ShuffleRight(RegisterType Source, const int32 ShuffleAmount)
    {
        return ShuffleLeft(Source, ShuffleAmount * -1);
    }

    template<typename T>
    INLINE T* ToPtr(Vector16<T>* Source)
    {
        return reinterpret_cast<T*>(ASSUME_ALIGNED(Source, 16));
    }

    template<typename T>
    INLINE T* ToPtr(Vector32<T>* Source)
    {
        return reinterpret_cast<T*>(ASSUME_ALIGNED(Source, 32));
    }

    template<typename T>
    INLINE T* ToPtr(Vector64<T>* Source)
    {
        return reinterpret_cast<T*>(ASSUME_ALIGNED(Source, 64));
    }

    template<typename T>
    INLINE const T* ToPtr(const Vector16<T>* Source)
    {
        return reinterpret_cast<const T*>(ASSUME_ALIGNED(Source, 16));
    }

    template<typename T>
    INLINE const T* ToPtr(const Vector32<T>* Source)
    {
        return reinterpret_cast<const T*>(ASSUME_ALIGNED(Source, 32));
    }

    template<typename T>
    INLINE const T* ToPtr(const Vector64<T>* Source)
    {
        return reinterpret_cast<const T*>(ASSUME_ALIGNED(Source, 64));
    }

}

#ifdef AVX128

inline consteval char8_16 operator""_char8_16(uint64 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<char8_16>(Value);
}

inline consteval int8_16 operator""_int8_16(uint64 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<int8_16>(Value);
}

inline consteval uint8_16 operator""_uint8_16(uint64 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<uint8_16>(Value);
}

inline consteval int16_8 operator"" _int16_8(uint64 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<int16_8>(Value);
}

inline consteval uint16_8 operator""_uint16_8(uint64 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<uint16_8>(Value);
}

inline consteval int32_4 operator""_int32_4(uint64 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<int32_4>(Value);
}

inline consteval uint32_4 operator""_uint32_4(uint64 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<uint32_4>(Value);
}

inline consteval int64_2 operator""_int64_2(uint64 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<int64_2>(Value);
}

inline consteval uint64_2 operator""_uint64_2(uint64 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<uint64_2>(Value);
}

inline consteval float32_4 operator""_float32_4(float128 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<float32_4>(Value);
}

inline consteval float64_2 operator""_float64_2(float128 Value) MIN_VECTOR_WIDTH(128)
{
    return Simd::SetAll<float64_2>(Value);
}

#endif //AVX128
#ifdef AVX256

inline consteval char8_32 operator""_char8_32(uint64 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<char8_32>(Value);
}

inline consteval int8_32 operator""_int8_32(uint64 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<int8_32>(Value);
}

inline consteval uint8_32 operator""_uint8_32(uint64 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<uint8_32>(Value);
}

inline consteval int16_16 operator""_int16_16(uint64 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<int16_16>(Value);
}

inline consteval uint16_16 operator""_uint16_16(uint64 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<uint16_16>(Value);
}

inline consteval int32_8 operator""_int32_8(uint64 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<int32_8>(Value);
}

inline consteval uint32_8 operator""_uint32_8(uint64 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<uint32_8>(Value);
}

inline consteval int64_4 operator""_int64_4(uint64 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<int64_4>(Value);
}

inline consteval uint64_4 operator""_uint64_4(uint64 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<uint64_4>(Value);
}

inline consteval float32_8 operator""_float32_8(float128 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<float32_8>(Value);
}

inline consteval float64_4 operator""_float64_4(float128 Value) MIN_VECTOR_WIDTH(256)
{
    return Simd::SetAll<float64_4>(Value);
}

#endif //AVX256
#ifdef AVX512

inline consteval char8_64 operator""_char8_64(uint64 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<char8_64>(Value);
}

inline consteval int8_64 operator""_int8_64(uint64 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<int8_64>(Value);
}

inline consteval uint8_64 operator""_uint8_64(uint64 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<uint8_64>(Value);
}

inline consteval int16_32 operator""_int16_32(uint64 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<int16_32>(Value);
}

inline consteval uint16_32 operator""_uint16_32(uint64 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<uint16_32>(Value);
}

inline consteval int32_16 operator""_int32_16(uint64 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<int32_16>(Value);
}

inline consteval uint32_16 operator""_uint32_16(uint64 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<uint32_16>(Value);
}

inline consteval int64_8 operator""_int64_8(uint64 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<int64_8>(Value);
}

inline consteval uint64_8 operator""_uint64_8(uint64 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<uint64_8>(Value);
}

inline consteval float32_16 operator""_float32_16(float128 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<float32_16>(Value);
}

inline consteval float64_8 operator""_float64_8(float128 Value) MIN_VECTOR_WIDTH(512)
{
    return Simd::SetAll<float64_8>(Value);
}

#endif //AVX512

namespace Avx = Simd; //use whatever looks best

