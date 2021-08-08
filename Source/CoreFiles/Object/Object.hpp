#pragma once

#include "../Definitions.hpp"
#include "../String.hpp"
#include "../Array.hpp"
#include "../Memory.hpp"
#include "../Simd.hpp"
#include "../SmartPointer.hpp"
#include "../TypeTraits.hpp"
#include "../Atomic.hpp"
#include "Thread/Thread.hpp"
#include "../Log.hpp"

class OObject;

/*
 * Used to register a class with a name and an id
 *
 * OBJECT_CLASS(OMyClass) : public OObject ...
 */
#define OBJECT_CLASS(Class)\
class Class;\
\
namespace TypeTrait\
{\
    template<>\
    struct TIsObjectClass<Class>\
    {\
        static constexpr bool Value{true};\
    };\
}\
\
namespace Rtti\
{\
    template<>\
    inline constexpr FStaticString GetObjectName<Class>()\
    {\
        return #Class;\
    }\
\
    template<>\
    inline uint32 GetObjectTypeID<Class>()\
    {\
        static const uint32 TypeId{GenerateTypeID()};\
        return TypeId;\
    }\
}\
class Class

/*
 * The arguments for the OBJECT_BODY(...) macro is all the direct base classes
 *
 * OBJECT_CLASS(OMyClass) : public OObject
 * {
 *     OBJECT_BODY(OObject)
 * };
 */
#define OBJECT_BODY(...)\
public:\
    static constexpr TypeTrait::AddPointersToTupleArgs<__VA_ARGS__> GetDirectBaseClasses()\
    {\
        return {};\
    }\
\
    static constexpr uint64 NumTotalBaseClasses()\
    {\
        auto NumDirectBaseClasses = []<typename... Ts>()\
        {\
            return sizeof...(Ts);\
        };\
\
        auto FoldNumTotalBaseClasses = []<typename... Ts>()\
        {\
            return (0 + ... + Ts::NumTotalBaseClasses());\
        };\
\
        return NumDirectBaseClasses.operator()<__VA_ARGS__>() + FoldNumTotalBaseClasses.operator()<__VA_ARGS__>();\
    }\
\
    virtual uint32 GetObjectId() const __VA_OPT__(override)\
    {\
        using ThisType = TypeTrait::RemoveCV<TypeTrait::RemovePointer<decltype(this)>>;\
        return Rtti::GetObjectTypeID<ThisType>();\
    }\
\
    virtual int64 OffsetThisFromId(const uint32 TargetId) const __VA_OPT__(override)\
    {\
        using ThisType = TypeTrait::RemoveCV<TypeTrait::RemovePointer<decltype(this)>>;\
        return Rtti::OffsetFromID<ThisType>(reinterpret_cast<int64>(this), TargetId);\
    }\
\
    virtual FStaticString GetClassName() const __VA_OPT__(override)\
    {\
        using ThisType = TypeTrait::RemoveCV<TypeTrait::RemovePointer<decltype(this)>>;\
        return Rtti::GetObjectName<ThisType>();\
    }\
private:

namespace Rtti
{
    template<typename DerivedClass, uint64 Index>
    using BaseClassType = TypeTrait::RemovePointer<TypeTrait::RemoveReference<decltype(std::get<Index>(DerivedClass::GetDirectBaseClasses()))>>;

#if defined(AVX512)
    using TypeIdTRegister = Register64<uint32>;
#elif defined(AVX256)
    using RegisterTypeId = Register32<uint32>;
#elif defined(AVX128)
    using TypeIdTRegister = Register16<uint32>;
#endif

    INLINE uint32 GenerateTypeID()
    {
        static constinit TAtomic<uint32> TypeID{0};
        return ++TypeID;
    }

    template<typename Class>
    constexpr FStaticString GetObjectName();

    template<typename Class>
    uint32 GetObjectTypeID();

    template<typename Derived, uint64 Index>
    using BaseClassAt = TypeTrait::RemovePointer<TypeTrait::RemoveCV<TypeTrait::RemoveReference<decltype(std::get<Index>(Derived::GetDirectBaseClasses()))>>>;

    template<typename Class>
    using IDArrayType = TStaticArray<uint32, Class::NumTotalBaseClasses() + 1>;

    template<typename Derived, typename Class>
    inline IDArrayType<Class> MakeTypeIDArray()
    {
        int32 Offset{static_cast<int32>(Math::PointerOffset<Derived, Class>())};
        Offset <<= 16; //push offset to the upper half

        TStaticArray<uint32, 1> OffsetAndTypeId{Offset | GetObjectTypeID<Class>()};

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
        if constexpr(NumIds <= (Simd::NumElements<RegisterTypeId>() * Index))
        {
            return Index;
        }
        else
        {
            return CalculateNumComparisons<NumIds, Index + 1>();
        }
    }

    inline TDynamicArray<Thread::FThread> IDInitializationThreads{512, EChooseConstructor{}};

    template<typename Class>
    class TIDArray
    {
    private:

        static void* ThreadInitializeIDArray(void* ArrayContainer)
        {
            static_cast<TIDArray*>(ArrayContainer)->InitializeIDArray();
            pthread_exit(nullptr);
        }

    public:

        TIDArray()
        {
            Thread::FThread InitializationThread{};
            InitializationThread.Create(&ThreadInitializeIDArray, this);

            IDInitializationThreads.Append(InitializationThread);
        }

        const IDArrayType<Class>& GetArray() const
        {
            return Array;
        }

    private:

        void InitializeIDArray()
        {
        #if DEBUG
            const FString LogString{FString{"building ID-Array for "} += GetObjectName<Class>()};

            LOG(LogThread, "started task: {}", LogString);
            Array = MakeTypeIDArray<Class, Class>();
            LOG(LogThread, "completed task: {}", LogString);
        #else
            Array = MakeTypeIDArray<Class, Class>();
        #endif
        }

        IDArrayType<Class> Array;
    };

    template<typename Class>
    inline const TIDArray<Class> GlobalIDArray{};

    template<typename Class>
    inline int64 OffsetFromID(const int64 SourcePtr, const uint32 TargetID)
    {
        const IDArrayType<Class>& IDArray{GlobalIDArray<Class>.GetArray()};

        constexpr uint64 NumRegisterElements{Simd::NumElements<RegisterTypeId>()};
        constexpr uint64 NumComparisons{CalculateNumComparisons<IDArrayType<Class>::Num(), 1>()};
        constexpr uint64 NumElementIterations{NumRegisterElements * NumComparisons};
        constexpr int32 ValidMask{Simd::Mask<RegisterTypeId>() >> (NumElementIterations - IDArrayType<Class>::Num())}; //mask out the garbage results with this

        int64 ResultPtr{0};

        #pragma unroll NumComparisons
        for(uint64 Index{0}; Index < NumElementIterations; Index += NumRegisterElements)
        {
            RegisterTypeId StoredIDs{Simd::LoadUnaligned<RegisterTypeId>(IDArray + Index)};
            StoredIDs &= 0x0000FFFF; //mask out the offset part

            int32 ComparisonMask{Simd::MoveMask(TargetID == StoredIDs)};

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

}

template<typename TargetType, typename BaseType>
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
            const uint32 TargetId{Rtti::GetObjectTypeID<TargetTypeClass>()};
            return reinterpret_cast<TargetType>(BasePointer->OffsetThisFromId(TargetId));
        }
    }

    return nullptr;
}

template<typename TargetType, typename BaseType>
inline TargetType ObjectCastExact(BaseType* BasePointer)
{
    using TargetTypeClass = TypeTrait::RemoveCV<TypeTrait::RemovePointer<TargetType>>;

    if(BasePointer != nullptr)
    {
        const uint32 TargetId{Rtti::GetObjectTypeID<TargetTypeClass>()};

        if(BasePointer->GetObjectId() == TargetId)
        {
            return static_cast<TargetType>(BasePointer);
        }
    }

    return nullptr;
}

template<typename TargetType, typename BaseType>
inline NONNULL TargetType ObjectCastChecked(NONNULL BaseType* BasePointer)
{
    using TargetTypeClass = TypeTrait::RemoveCV<TypeTrait::RemovePointer<TargetType>>;

#if DEBUG
    ASSERT(BasePointer != nullptr);

    if constexpr(TypeTrait::IsBaseOf<TargetTypeClass, TypeTrait::RemoveCV<BaseType>>)
    {
        return static_cast<TargetType>(BasePointer);
    }
    else
    {
        const uint32 TargetId{Rtti::GetObjectTypeID<TargetTypeClass>()};
        ASSERT(BasePointer->OffsetThisFromId(TargetId));

        return static_cast<TargetType>(BasePointer);
    }
#else
    return static_cast<TargetType>(BasePointer);
#endif
}

template<typename TargetType, typename BaseType, template<typename, ESPMode> typename PointerClass, ESPMode ThreadMode>
inline PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode> ObjectCast(PointerClass<BaseType, ThreadMode>& BasePointer)
{
    using TargetPointerClass = PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode>;

    TargetType CastPointer{ObjectCast<TargetType>(BasePointer.Ptr())};

    if(CastPointer != nullptr)
    {
        BasePointer.AddReference();
        return TargetPointerClass{CastPointer, BasePointer.ReferenceCounter};
    }
    else
    {
        return TargetPointerClass{CastPointer, nullptr};
    }
}

template<typename TargetType, typename BaseType, template<typename, ESPMode> typename PointerClass, ESPMode ThreadMode>
inline PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode> ObjectCastExact(PointerClass<BaseType, ThreadMode>& BasePointer)
{
    using TargetPointerClass = PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode>;

    TargetType CastPointer{ObjectCastExact<TargetType>(BasePointer.Ptr())};

    if(CastPointer != nullptr)
    {
        BasePointer.AddReference();
        return TargetPointerClass{CastPointer, BasePointer.ReferenceCounter};
    }
    else
    {
        return TargetPointerClass{CastPointer, nullptr};
    }
}

template<typename TargetType, typename BaseType, template<typename, ESPMode> typename PointerClass, ESPMode ThreadMode>
inline PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode> ObjectCastChecked(PointerClass<BaseType, ThreadMode>& BasePointer)
{
    TargetType CastPointer{ObjectCastChecked<TargetType>(BasePointer.Ptr())};

    BasePointer.AddReference();

    return PointerClass<TypeTrait::RemovePointer<TargetType>, ThreadMode>{CastPointer, BasePointer.ReferenceCounter};
}

OBJECT_CLASS(OObject) : public FNonCopyable
{
    OBJECT_BODY()
public:

    OObject() = default;
    virtual ~OObject() = default;

    virtual void PreReplicate()
    {
    }

    virtual void PostReplicate()
    {
    }

protected:

    bool bIsReplicated;
};

inline FStaticString GetClassNameSafe(const OObject* const Object)
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



