#pragma once

#include "Definitions.hpp"
#include "String.hpp"
#include "Array.hpp"
#include "Simd.hpp"
#include "SmartPointer.hpp"
#include "TypeTraits.hpp"

#include <tuple>

#define FOR_EACH2_1(f, t, n, ...) f(t, n)
#define FOR_EACH2_2(f, t, n, ...) f(t, n), FOR_EACH2_1(f, __VA_ARGS__)
#define FOR_EACH2_3(f, t, n, ...) f(t, n), FOR_EACH2_2(f, __VA_ARGS__)
#define FOR_EACH2_4(f, t, n, ...) f(t, n), FOR_EACH2_3(f, __VA_ARGS__)
#define FOR_EACH2_5(f, t, n, ...) f(t, n), FOR_EACH2_4(f, __VA_ARGS__)
#define FOR_EACH2_6(f, t, n, ...) f(t, n), FOR_EACH2_5(f, __VA_ARGS__)
#define FOR_EACH2_7(f, t, n, ...) f(t, n), FOR_EACH2_6(f, __VA_ARGS__)
#define FOR_EACH2_8(f, t, n, ...) f(t, n), FOR_EACH2_7(f, __VA_ARGS__)
#define FOR_EACH2_9(f, t, n, ...) f(t, n), FOR_EACH2_8(f, __VA_ARGS__)
#define FOR_EACH2_10(f, t, n, ...) f(t, n), FOR_EACH2_9(f, __VA_ARGS__)
#define FOR_EACH2_11(f, t, n, ...) f(t, n), FOR_EACH2_10(f, __VA_ARGS__)
#define FOR_EACH2_12(f, t, n, ...) f(t, n), FOR_EACH2_11(f, __VA_ARGS__)
#define FOR_EACH2_13(f, t, n, ...) f(t, n), FOR_EACH2_12(f, __VA_ARGS__)
#define FOR_EACH2_14(f, t, n, ...) f(t, n), FOR_EACH2_13(f, __VA_ARGS__)
#define FOR_EACH2_15(f, t, n, ...) f(t, n), FOR_EACH2_14(f, __VA_ARGS__)

#define CONCATENATE(A, B) A ## B

#define FOR_EACH2_NARG(...) FOR_EACH2_NARG_(dummy, __VA_ARGS__, FOR_EACH2_RSEQ_N())
#define FOR_EACH2_NARG_(...) FOR_EACH2_ARG_N(__VA_ARGS__)
#define FOR_EACH2_ARG_N(a, b, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, n, ...) n
#define FOR_EACH2_RSEQ_N() 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0

#define FOR_EACH2_(n, f, ...) CONCATENATE(FOR_EACH2_, n)(f, __VA_ARGS__)
#define FOR_EACH2(f, ...) FOR_EACH2_(FOR_EACH2_NARG(dummy, __VA_ARGS__), f, __VA_ARGS__)

#define TEMPLATE_ARG(f, n) f n
#define TEMPLATE_NAME(f, n) n

#define DECLARE_CLASS(Class, ...)\
class Class

#define DECLARE_CLASS_TEMPLATE(Class, ...)\
template<FOR_EACH2(TEMPLATE_ARG, __VA_ARGS__)> \
class Class

#define SPECIALIZE_TYPE_TRAIT(Class, ...)\
template<> \
struct ::TypeTrait::TIsObjectClass<Class>\
{\
    static constexpr bool Value{true};\
};

#define SPECIALIZE_TYPE_TRAIT_TEMPLATE(Class, ...)\
template<FOR_EACH2(TEMPLATE_ARG, __VA_ARGS__)> \
struct ::TypeTrait::TIsObjectClass<Class<FOR_EACH2(TEMPLATE_NAME, __VA_ARGS__)>>\
{\
    static constexpr bool Value{true};\
};

#define SPECIALIZE_INFO(Class, ...)\
template<> \
struct ::Rtti::TObjectInfo<Class>\
{\
    inline static constexpr FString<SS60> GetObjectName()\
    {\
        return #Class;\
    }\
\
    inline static uint32 GetObjectTypeID()\
    {\
        static const uint32 TypeId{GenerateTypeID()};\
        return TypeId;\
    }\
};

#define SPECIALIZE_INFO_TEMPLATE(Class, ...)\
template<FOR_EACH2(TEMPLATE_ARG, __VA_ARGS__)> \
struct ::Rtti::TObjectInfo<Class<FOR_EACH2(TEMPLATE_NAME, __VA_ARGS__)>>\
{\
    inline static constexpr FString<SS60> GetObjectName()\
    {\
        return #Class;\
    }\
\
    inline static uint32 GetObjectTypeID()\
    {\
        static const uint32 TypeId{GenerateTypeID()};\
        return TypeId;\
    }\
};

/*
 * used to register a class in the type system
 * example usage:
 * OBJECT_CLASS(OMyClass) : public OObject ...
 * OBJECT_CLASS(OMyClass, int, NTTP, typename, Type) : public OObject ...
 */
#define OBJECT_CLASS(Class, ...)\
DECLARE_CLASS##__VA_OPT__(_TEMPLATE)(Class __VA_OPT__(,) __VA_ARGS__);\
SPECIALIZE_TYPE_TRAIT##__VA_OPT__(_TEMPLATE)(Class __VA_OPT__(,) __VA_ARGS__)\
SPECIALIZE_INFO##__VA_OPT__(_TEMPLATE)(Class __VA_OPT__(,) __VA_ARGS__)

/*
 * used to register a class in the type system
 * example usage:
 * OBJECT_CLASS(MyNamespace, OMyClass) : public OObject ...
 * OBJECT_CLASS(OMyClass, int, NTTP, typename, Type) : public OObject ...
 */
#define OBJECT_CLASS_NAMESPACED(Namespace, Class, ...)\
namespace Namespace\
{\
DECLARE_CLASS##__VA_OPT__(_TEMPLATE)(Class __VA_OPT__(,) __VA_ARGS__);\
}\
\
SPECIALIZE_TYPE_TRAIT##__VA_OPT__(_TEMPLATE)(Namespace::Class __VA_OPT__(,) __VA_ARGS__)\
SPECIALIZE_INFO##__VA_OPT__(_TEMPLATE)(Namespace::Class __VA_OPT__(,) __VA_ARGS__)


/*
 * The arguments for the OBJECT_BASES(...) macro is all the direct base classes (if they are objects themselves)
 *
 * OBJECT_CLASS(OMyClass) : public OObject1, public NotAnObject, public OObject2
 * {
 *     OBJECT_BASES(OObject1, OObject2)
 * };
 */
#define OBJECT_BASES(...)\
public:\
\
    static constexpr TypeTrait::AddPointersToTupleArgs<__VA_ARGS__> GetDirectBaseClasses()\
    {\
        return {};\
    }\
\
    static constexpr uint64 NumDirectBaseClasses()\
    {\
        return NUM_ARGUMENTS(__VA_ARGS__);\
    }\
\
    static constexpr uint64 NumTotalBaseClasses()\
    {\
        auto FoldNumTotalBaseClasses = []<typename... Ts>()\
        {\
            return (0 + ... + Ts::NumTotalBaseClasses());\
        };\
\
        return NumDirectBaseClasses() + FoldNumTotalBaseClasses.template operator()<__VA_ARGS__>();\
    }\
\
    virtual uint32 GetObjectID() const __VA_OPT__(override)\
    {\
        using ThisType = TypeTrait::RemoveCV<TypeTrait::RemovePointer<decltype(this)>>;\
        return Rtti::TObjectInfo<ThisType>::GetObjectTypeID();\
    }\
\
    virtual int64 OffsetThisFromID(const uint32 TargetId) const __VA_OPT__(override)\
    {\
        using ThisType = TypeTrait::RemoveCV<TypeTrait::RemovePointer<decltype(this)>>;\
        return Rtti::OffsetFromID<ThisType>(reinterpret_cast<int64>(this), TargetId);\
    }\
\
    virtual FString<SS60> GetClassName() const __VA_OPT__(override)\
    {\
        using ThisType = TypeTrait::RemoveCV<TypeTrait::RemovePointer<decltype(this)>>;\
        return Rtti::TObjectInfo<ThisType>::GetObjectName();\
    }\
\
private:

namespace Rtti
{
    template<typename DerivedClass, uint64 Index>
    using BaseClassType = TypeTrait::RemovePointer<TypeTrait::RemoveReference<decltype(std::get<Index>(DerivedClass::GetDirectBaseClasses()))>>;

#if defined(AVX512)
    using VectorIDType = Vector64<uint32>;
#elif defined(AVX256)
    using VectorIDType = Vector32<uint32>;
#elif defined(AVX128)
    using VectorIDType = Vector16<uint32>;
#endif

    INLINE uint32 GenerateTypeID()
    {
        static constinit uint32 TypeID{0};
        return ++TypeID;
    }

    template<typename Class>
    struct TObjectInfo
    {
    };

    template<typename Derived, uint64 Index>
    using BaseClassAt = TypeTrait::RemovePointer<TypeTrait::RemoveCV<TypeTrait::RemoveReference<decltype(std::get<Index>(Derived::GetDirectBaseClasses()))>>>;

    template<typename Class>
    using IDArrayType = TStaticArray<uint32, Class::NumTotalBaseClasses() + 1>;

    template<typename Derived, typename Class>
    inline IDArrayType<Class> MakeTypeIDArray()
    {
        int32 Offset{static_cast<int32>(Math::PointerOffset<Derived, Class>())};
        Offset <<= 16; //push offset to the upper half

        TStaticArray<uint32, 1> OffsetAndTypeId{Offset | TObjectInfo<Class>::GetObjectTypeID()};

        auto ExpandTuple = [&OffsetAndTypeId]<uint64... I>(TypeTrait::IndexSequence<I...>)
        {
            return ArrUtil::JoinArrays(OffsetAndTypeId, MakeTypeIDArray<Derived, BaseClassAt<Class, I>>()...);
        };

        constexpr uint64 NumDirectBaseClasses{std::tuple_size_v<decltype(Class::GetDirectBaseClasses())>};

        if constexpr(NumDirectBaseClasses > 0)
        {
            return ExpandTuple(TypeTrait::MakeIndexSequence<NumDirectBaseClasses>{});
        }
        else
        {
            return OffsetAndTypeId;
        }
    }

    template<uint64 NumIds, uint64 Index>
    inline consteval uint64 CalculateNumComparisons()
    {
        if constexpr(NumIds <= (Simd::NumElements<VectorIDType>() * Index))
        {
            return Index;
        }
        else
        {
            return CalculateNumComparisons<NumIds, Index + 1>();
        }
    }

    template<typename Class>
    inline const IDArrayType<Class> GlobalIDArray alignas(VectorIDType){MakeTypeIDArray<Class, Class>()}; //we have this global so all are initialized before main

    template<typename Class>
    inline int64 OffsetFromID(const int64 SourcePtr, const uint32 TargetID)
    {
        const IDArrayType<Class>& IDArray{GlobalIDArray<Class>};

        constexpr uint64 NumRegisterElements{Simd::NumElements<VectorIDType>()};
        constexpr uint64 NumComparisons{CalculateNumComparisons<IDArrayType<Class>::Num(), 1>()};
        constexpr uint64 NumElementIterations{NumRegisterElements * NumComparisons};
        constexpr Simd::MaskType<VectorIDType> ValidMask{Simd::Mask<VectorIDType>() >> (NumElementIterations - IDArrayType<Class>::Num())}; //mask out the garbage results with this

        int64 ResultPtr{0};

        #pragma unroll NumComparisons
        for(uint64 Index{0}; Index < NumElementIterations; Index += NumRegisterElements)
        {
            VectorIDType StoredIDs{Simd::LoadAligned<VectorIDType>(IDArray + Index)};
            StoredIDs &= 0x0000FFFF; //mask out the offset part

            Simd::MaskType<VectorIDType> ComparisonMask{Simd::MoveMask(TargetID == StoredIDs)};

            if(Index == (NumElementIterations - NumRegisterElements)) //this should be optimized out
            {
                ComparisonMask &= ValidMask;
            }

            const int32 MatchIndex{Math::FindFirstSet(ComparisonMask)};
            const bool bIsMatch{MatchIndex != 0};

            const uint32 MatchedElement{IDArray[((MatchIndex - 1) + Index) + !bIsMatch]}; //FindFirstSet returns index + 1, we need index
            const int16 Offset{static_cast<int16>(MatchedElement >> 16)}; //push the id to the low 16 bits

            ResultPtr += (SourcePtr + Offset) * bIsMatch;
        }

        return ResultPtr;
    }

} //namespace Rtti

template<typename TargetType, typename BaseType> requires(TypeTrait::IsObjectClass<TypeTrait::RemovePointer<TargetType>> && TypeTrait::IsObjectClass<BaseType>)
inline TargetType ObjectCast(BaseType* BasePointer)
{
    using TargetTypeClass = TypeTrait::RemoveCV<TypeTrait::RemovePointer<TargetType>>;

    if(BasePointer != nullptr)
    {
        if constexpr(TypeTrait::IsBaseOf<TargetTypeClass, TypeTrait::RemoveCV<BaseType>>)
        {
            return static_cast<TargetType>(BasePointer);
        }
        else
        {
            const uint32 TargetId{Rtti::TObjectInfo<TargetTypeClass>::GetObjectTypeID()};
            return reinterpret_cast<TargetType>(BasePointer->OffsetThisFromID(TargetId));
        }
    }

    return nullptr;
}

template<typename TargetType, typename BaseType> requires(TypeTrait::IsObjectClass<TypeTrait::RemovePointer<TargetType>> && TypeTrait::IsObjectClass<BaseType> && TypeTrait::IsStaticCastable<TargetType, BaseType*>)
inline TargetType ObjectCastExact(BaseType* BasePointer)
{
    using TargetTypeClass = TypeTrait::RemoveCV<TypeTrait::RemovePointer<TargetType>>;

    if(BasePointer != nullptr)
    {
        const uint32 TargetId{Rtti::TObjectInfo<TargetTypeClass>::GetObjectTypeID()};

        if(BasePointer->GetObjectID() == TargetId)
        {
            return static_cast<TargetType>(BasePointer);
        }
    }

    return nullptr;
}

template<typename TargetType, typename BaseType> requires(TypeTrait::IsObjectClass<TypeTrait::RemovePointer<TargetType>> && TypeTrait::IsObjectClass<BaseType>)
inline TargetType ObjectCastChecked(BaseType* BasePointer)
{
    using TargetTypeClass = TypeTrait::RemoveCV<TypeTrait::RemovePointer<TargetType>>;

    ASSERT(BasePointer != nullptr);

    if constexpr(TypeTrait::IsStaticCastable<TargetType, BaseType*>)
    {
#if DEBUG
        const uint32 TargetId{Rtti::TObjectInfo<TargetTypeClass>::GetObjectTypeID()};
        ASSERT(BasePointer->OffsetThisFromID(TargetId) != nullptr);

        return static_cast<TargetType>(BasePointer);
#else
        return static_cast<TargetType>(BasePointer);
#endif
    }
    else
    {
        const uint32 TargetId{Rtti::TObjectInfo<TargetTypeClass>::GetObjectTypeID()};
        return reinterpret_cast<TargetType>(BasePointer->OffsetThisFromID(TargetId));
    }
}

namespace PtrPri
{
    template<typename TargetType, typename BaseType, template<typename, EThreadMode> typename PointerClass, EThreadMode ThreadMode>
    struct TObjectCastSmartPointerHelper final
    {
        inline static PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode> ObjectCast(PointerClass<BaseType, ThreadMode>& BasePointer)
        {
            using TargetPointerClass = PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode>;

            TargetType CastPointer{::ObjectCast<TargetType>(BasePointer.Pointer)};

            if(CastPointer != nullptr)
            {
                BasePointer.AddReference();
                return TargetPointerClass{CastPointer, BasePointer.ReferenceCounter};
            }
            else
            {
                return TargetPointerClass{nullptr};
            }
        }

        inline static PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode> ObjectCastExact(PointerClass<BaseType, ThreadMode>& BasePointer)
        {
            using TargetPointerClass = PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode>;

            TargetType CastPointer{::ObjectCastExact<TargetType>(BasePointer.Ptr())};

            if(CastPointer != nullptr)
            {
                BasePointer.AddReference();
                return TargetPointerClass{CastPointer, BasePointer.ReferenceCounter};
            }
            else
            {
                return TargetPointerClass{nullptr};
            }
        }

        inline static PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode> ObjectCastChecked(PointerClass<BaseType, ThreadMode>& BasePointer)
        {
            using TargetPointerClass = PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode>;

            TargetType CastPointer{::ObjectCastChecked<TargetType>(BasePointer.Pointer)};

            BasePointer.AddReference();

            return TargetPointerClass{CastPointer, BasePointer.ReferenceCounter};
        }
    };
} //namespace PtrPri

template<typename TargetType, typename BaseType, template<typename, EThreadMode> typename PointerClass, EThreadMode ThreadMode>
INLINE PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode> ObjectCast(PointerClass<BaseType, ThreadMode>& BasePointer)
{
    return PtrPri::TObjectCastSmartPointerHelper<TargetType, BaseType, PointerClass, ThreadMode>::ObjectCast(BasePointer);
}

template<typename TargetType, typename BaseType, template<typename, EThreadMode> typename PointerClass, EThreadMode ThreadMode>
INLINE PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode> ObjectCastExact(PointerClass<BaseType, ThreadMode>& BasePointer)
{
    return PtrPri::TObjectCastSmartPointerHelper<TargetType, BaseType, PointerClass, ThreadMode>::ObjectCastExact(BasePointer);
}

template<typename TargetType, typename BaseType, template<typename, EThreadMode> typename PointerClass, EThreadMode ThreadMode>
INLINE PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode> ObjectCastChecked(PointerClass<BaseType, ThreadMode>& BasePointer)
{
    return PtrPri::TObjectCastSmartPointerHelper<TargetType, BaseType, PointerClass, ThreadMode>::ObjectCastChecked(BasePointer);
}

OBJECT_CLASS(OObject)
class OObject
{
    OBJECT_BASES()
public:

    OObject() = default;
    virtual ~OObject() = default;
};

inline FString<SS60> GetClassNameSafe(const OObject* const Object)
{
    if(Object)
    {
        return Object->GetClassName();
    }
    else
    {
        return "null";
    }
}



