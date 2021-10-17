#pragma once

#include "Definitions.hpp"
#include <type_traits>
#include <cstdint>

#define MAKE_TYPE_SPECIALIZATION(Name, In, Out) template<> struct Name<In>{using Type = Out;};
#define MAKE_VALUE_SPECIALIZATION(Name, In, Out) template<> struct Name<In>{inline static constexpr bool Value{Out};};

namespace TypeTrait
{
    struct TrueType
    {
        static constexpr bool Value{true};
    };

    struct FalseType
    {
        static constexpr bool Value{false};
    };

    template<typename T, uint64 Size = 0>
    NORETURN consteval void FindSize()
    {
        static_assert(sizeof(T) != Size);

        FindSize<T, Size + 1>();
    }

    template<typename T, uint64 Align = 0>
    NORETURN consteval void FindAlign()
    {
        static_assert(alignof(T) != Align);

        FindAlign<T, Align + 1>();
    }

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
    using Decay = typename std::decay<T>::type;

    template<typename A, typename B>
    struct AreTypesEqualImpl
    {
        enum{Value = false};
    };

    template<typename T>
    struct AreTypesEqualImpl<T, T>
    {
        enum{Value = true};
    };

    template<typename A, typename B>
    const constinit bool AreTypesEqual = AreTypesEqualImpl<A, B>::Value;

    template<typename T>
    concept IsTrivial = __is_trivial(T);

    template<typename T>
    concept IsInteger = std::is_integral<T>::value;

    template<typename T>
    concept IsPointer = std::is_pointer<T>::value;

    template<typename T>
    struct RemovePointerImpl
    {
        using Type = T;
    };

    template<typename T>
    struct RemovePointerImpl<T*>
    {
        using Type = T;
    };

    template<typename T>
    using RemovePointer = typename RemovePointerImpl<T>::Type;

    template<typename T>
    struct IsBitwiseComparableImpl
    {
        enum{Value = std::is_trivial<T>::value};
    };

    template<typename T>
    concept IsBitwiseComparable = IsBitwiseComparableImpl<T>::Value;

    template<typename T>
    struct RemoveCVImpl{using Type = T;};

    template<typename T>
    using RemoveCV = typename RemoveCVImpl<T>::Type;

    template<typename T>
    struct RemoveCVImpl<const T>{using Type = T;};

    template<typename T>
    struct RemoveCVImpl<volatile T>{using Type = T;};

    template<typename T>
    struct RemoveCVImpl<const volatile T>{using Type = T;};

    template<typename T>
    struct RemoveReferenceImpl{using Type = T;};

    template<typename T>
    using RemoveReference = typename RemoveReferenceImpl<T>::Type;

    template<typename T>
    struct RemoveReferenceImpl<T&>{using Type = T;};

    template<typename T>
    struct RemoveReferenceImpl<T&&>{using Type = T;};

    template<typename A, typename B>
    inline const constinit bool AreBitwiseComparable = (AreTypesEqual<RemoveCV<A>, RemoveCV<B>>) && (IsBitwiseComparable<RemoveCV<A>>);

    template<typename T>
    concept IsDefaultConstructible = std::is_default_constructible<T>::value;

    template<typename T, typename... Types>
    inline const constinit bool IsAnyOf = (AreTypesEqual<T, Types> || ...);

    template<typename T, typename... Types>
    inline const constinit bool IsAllOf = (AreTypesEqual<T, Types> && ...);

    template<typename T>
    concept IsFloatingPoint = std::is_floating_point_v<T>;

    template<bool Condition, typename TrueType, typename FalseType>
    struct ConditionalImpl;

    template<typename TrueType, typename FalseType>
    struct ConditionalImpl<true, TrueType, FalseType>
    {
        using Type = TrueType;
    };

    template<typename TrueType, typename FalseType>
    struct ConditionalImpl<false, TrueType, FalseType>
    {
        using Type = FalseType;
    };

    template<bool Condition, typename TrueType, typename FalseType>
    using Conditional = typename ConditionalImpl<Condition, TrueType, FalseType>::Type;

    template<typename T, template<typename...> typename TT>
    struct IsSpecializationOf : private FalseType{};

    template<template<typename...> typename TT, typename... Args>
    struct IsSpecializationOf<TT<Args...>, TT> : private TrueType{};

    template<typename Base, typename Derived>
    concept IsBaseOf = std::is_base_of<Base, Derived>::value;

    template<typename... VarArgs>
    using AddPointersToTupleArgs = std::tuple<VarArgs*...>;

    template<typename... Ts>
    struct AreAllUniqueImpl{};

    template<typename T>
    struct AreAllUniqueImpl<T>{};

    template<typename T>
    struct AddUnsignedImpl{using Type = T;};

    template<typename T>
    using AddUnsigned = typename AddUnsignedImpl<T>::Type;

    MAKE_TYPE_SPECIALIZATION(AddUnsignedImpl, int8, uint8)
    MAKE_TYPE_SPECIALIZATION(AddUnsignedImpl, const int8, const uint8)
    MAKE_TYPE_SPECIALIZATION(AddUnsignedImpl, int16, uint16)
    MAKE_TYPE_SPECIALIZATION(AddUnsignedImpl, const int16, const uint16)
    MAKE_TYPE_SPECIALIZATION(AddUnsignedImpl, int32, uint32)
    MAKE_TYPE_SPECIALIZATION(AddUnsignedImpl, const int32, const uint32)
    MAKE_TYPE_SPECIALIZATION(AddUnsignedImpl, int64, uint64)
    MAKE_TYPE_SPECIALIZATION(AddUnsignedImpl, const int64, const uint64)

    template<typename T>
    struct AddSignedImpl{using Type = T;};

    template<typename T>
    using AddSigned = typename AddSignedImpl<T>::Type;

    MAKE_TYPE_SPECIALIZATION(AddSignedImpl, uint8, int8)
    MAKE_TYPE_SPECIALIZATION(AddSignedImpl, const uint8, const int8)
    MAKE_TYPE_SPECIALIZATION(AddSignedImpl, uint16, int16)
    MAKE_TYPE_SPECIALIZATION(AddSignedImpl, const uint16, const int16)
    MAKE_TYPE_SPECIALIZATION(AddSignedImpl, uint32, int32)
    MAKE_TYPE_SPECIALIZATION(AddSignedImpl, const uint32, const int32)
    MAKE_TYPE_SPECIALIZATION(AddSignedImpl, uint64, int64)
    MAKE_TYPE_SPECIALIZATION(AddSignedImpl, const uint64, const int64)

    template<typename T>
    concept IsSigned = std::is_signed<T>::value;

    template<typename T>
    concept IsUnsigned = std::is_unsigned<T>::value;

    template<typename A, typename B>
    concept AreLogicallyComparable = requires(A lhs, B rhs)
    {
        lhs == rhs;
    };

    template<typename FunctionT, typename ParamT>
    concept FunctionHasParameter = requires(FunctionT&& Function, ParamT Parameter)
    {
        Function(Parameter);
    };

    template<typename To, typename From>
    concept IsStaticCastable = requires(From Source)
    {
        static_cast<To>(Source);
    };

    template<typename T>
    concept IsMoveConstructible = std::is_move_constructible_v<T>;

    template<typename T>
    concept IsTriviallyConstructible = std::is_trivial<T>::value || std::is_trivially_copy_constructible<T>::value;

    template<typename T>
    concept IsTriviallyDestructible = std::is_trivially_destructible<T>::value;

    template<typename T>
    using Pure = RemoveCV<RemoveReference<T>>;

    template<typename A, typename B>
    concept PureEqual = AreTypesEqual<Pure<A>, Pure<B>>;

    template<typename T>
    const constinit bool IsLvalue = std::is_lvalue_reference_v<T>;

    template<typename T>
    const constinit bool IsRvalue = std::is_rvalue_reference_v<T>;

    template<typename T>
    const constinit bool IsConst = std::is_const_v<T>;

    template<typename T>
    using RawType = TypeTrait::RemoveCV<TypeTrait::RemoveReference<TypeTrait::RemovePointer<T>>>;

    template<typename Class>
    struct TIsObjectClass
    {
        inline static constexpr bool Value{false};
    };

    template<typename Class>
    concept IsObjectClass = TIsObjectClass<TypeTrait::RemoveCV<Class>>::Value;
}

template<typename T>
INLINE constexpr TypeTrait::RemoveReference<T>&& Forward(TypeTrait::RemoveReference<T>& Arg)
{
    return static_cast<TypeTrait::RemoveReference<T>&&>(Arg);
}

template<typename T> requires(!TypeTrait::IsRvalue<T> && !TypeTrait::IsConst<T>)
INLINE constexpr TypeTrait::RemoveReference<T>&& Move(T&& Arg)
{
    return static_cast<TypeTrait::RemoveReference<T>&&>(Arg);
}

template<typename T>
INLINE constexpr T&& MoveIfPossible(T&& Arg)
{
    return static_cast<T&&>(Arg);
}



