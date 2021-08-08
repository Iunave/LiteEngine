#pragma once

#include "Definitions.hpp"
#include "TypeTraits.hpp"
#include "Assert.hpp"
#include "Atomic.hpp"

enum class ESPMode
{
    Safe = 1,
    Unsafe = 0,
    Default = Unsafe
};

enum class ECastOp
{
    Reinterpret,
    Dynamic,
    Static,
    Const
};

template<typename>
class TUniquePtr;

template<typename, ESPMode>
class TSharedPtr;

template<typename, ESPMode>
class TSharedRef;

template<typename, ESPMode>
class TWeakPtr;

namespace PtrPri
{
    template<typename OtherType, typename... Ts>
    concept IsOtherPointer = requires()
    {
        TypeTrait::IsAnyOf<TypeTrait::Pure<OtherType>, Ts...>;
    };

    template<typename ThisPtrType, typename OtherPtrType>
    concept InitReqs = requires()
    {
       TypeTrait::IsBaseOf<ThisPtrType, OtherPtrType>
       || ((TypeTrait::IsTrivial<ThisPtrType> && TypeTrait::IsTrivial<OtherPtrType>) && (sizeof(ThisPtrType) == sizeof(OtherPtrType)));
    };

    template<typename Type>
    class INTERNAL_LINKAGE TPointerBase
    {
    public:

        TPointerBase(const TPointerBase&) = delete;
        TPointerBase(TPointerBase&&) = delete;

        TPointerBase& operator=(const TPointerBase&) = delete;
        TPointerBase& operator=(TPointerBase&&) = delete;

    protected:

        inline constexpr explicit TPointerBase(ENoInit)
        {
        }

        inline explicit TPointerBase(Type* InPointer)
            : Pointer(InPointer)
        {
        }

        virtual inline ~TPointerBase() = default;

        template<typename NewPtrType, ECastOp CastOp>
        inline constexpr NewPtrType CastPointer() const
        {
            if constexpr(CastOp == ECastOp::Reinterpret)
            {
                return reinterpret_cast<NewPtrType>(Pointer);
            }
            else if constexpr(CastOp == ECastOp::Static)
            {
                return static_cast<NewPtrType>(Pointer);
            }
            else if constexpr(CastOp == ECastOp::Dynamic)
            {
                return ObjectCast<NewPtrType>(Pointer);
            }
            else if constexpr(CastOp == ECastOp::Const)
            {
                return const_cast<NewPtrType>(Pointer);
            }
        }

    public:

        inline Type& operator*()
        {
            ASSERT(IsValid());
            return *Pointer;
        }

        inline const Type& operator*() const
        {
            ASSERT(IsValid());
            return *Pointer;
        }

        inline Type* operator->()
        {
            ASSERT(IsValid());
            return Pointer;
        }

        inline const Type* operator->() const
        {
            ASSERT(IsValid());
            return Pointer;
        }

        inline explicit operator bool() const
        {
            return IsValid();
        }

        inline bool IsValid() const
        {
            return Pointer != nullptr;
        }

        inline Type* Ptr()
        {
            return Pointer;
        }

        inline const Type* Ptr() const
        {
            return Pointer;
        }

    protected:

        inline void Delete()
        {
            delete Pointer;
        }

        Type* Pointer;

    };

    enum class ESharedMode
    {
        Strong,
        Weak
    };

    template<ESPMode InThreadMode, ESharedMode SharedMode>
    class TSharedBase
    {
    protected:

        class TReferenceCounter
        {
        public:

            INLINE TReferenceCounter()
                : StrongReferenceCount{1}
                , WeakReferenceCount{1}
            {
            }

            virtual ~TReferenceCounter() = default;

            INLINE int32 AddStrongReference()
            {
                ++StrongReferenceCount;
                ++WeakReferenceCount;
                return StrongReferenceCount;
            }

            INLINE int32 AddWeakReference()
            {
                ++WeakReferenceCount;
                return WeakReferenceCount;
            }

            INLINE int32 RemoveStrongReference()
            {
                --StrongReferenceCount;
                --WeakReferenceCount;
                return StrongReferenceCount;
            }

            INLINE int32 RemoveWeakReference()
            {
                --WeakReferenceCount;
                return WeakReferenceCount;
            }

            INLINE int32 GetStrongReferenceCount() const
            {
                return StrongReferenceCount;
            }

            INLINE int32 GetWeakReferenceCount() const
            {
                return WeakReferenceCount;
            }

        protected:

            using CountType = TypeTrait::Conditional<InThreadMode == ESPMode::Unsafe, int32, TAtomic<int32>>;

            CountType StrongReferenceCount;
            CountType WeakReferenceCount;
        };

        inline constexpr explicit TSharedBase(ENoInit)
        {
        }

        INLINE explicit TSharedBase(TReferenceCounter* InRefCounter)
            : ReferenceCounter(InRefCounter)
        {
        }

        inline virtual ~TSharedBase() = default;

    public:

        constexpr static ESPMode ThreadMode()
        {
            return InThreadMode;
        }

        INLINE int32 StrongRefCount() const
        {
            if(IsRefCounterValid())
            {
                return ReferenceCounter->GetStrongReferenceCount();
            }
            return -1;
        }

        INLINE int32 WeakRefCount() const
        {
            if(IsRefCounterValid())
            {
                return ReferenceCounter->GetWeakReferenceCount();
            }
            return -1;
        }

    protected:

        INLINE int32 AddReference()
        {
            if(IsRefCounterValid())
            {
                if constexpr(SharedMode == ESharedMode::Strong)
                {
                    return ReferenceCounter->AddStrongReference();
                }
                else if constexpr(SharedMode == ESharedMode::Weak)
                {
                    return ReferenceCounter->AddWeakReference();
                }
            }
            return -1;
        }

        INLINE int32 RemoveReference()
        {
            if(IsRefCounterValid())
            {
                if constexpr(SharedMode == ESharedMode::Strong)
                {
                    return ReferenceCounter->RemoveStrongReference();
                }
                else if constexpr(SharedMode == ESharedMode::Weak)
                {
                    return ReferenceCounter->RemoveWeakReference();
                }
            }
            return -1;
        }

        INLINE bool IsRefCounterValid() const
        {
            return ReferenceCounter != nullptr;
        }

        INLINE void Delete()
        {
            delete ReferenceCounter;
        }

        TReferenceCounter* ReferenceCounter;

    };
}

template<typename Type>
class TUniquePtr final : public PtrPri::TPointerBase<Type>
{
    using TPointer = PtrPri::TPointerBase<Type>;

    template<typename InType, typename... VarArgs>
    friend TUniquePtr<InType> MakeUnique(VarArgs&&...);

    template<typename Base, typename Derived, typename... VarArgs>
    friend TUniquePtr<Base> MakeUnique(VarArgs&&... Args);

private:

    inline explicit constexpr TUniquePtr(ENoInit NoInit)
        : TPointer(NoInit)
    {
    }

public:

    inline explicit TUniquePtr(Type* RESTRICT InPointer NONNULL)
        : TPointer(InPointer)
    {
    }

    using PtrType = Type;
    using TPointer::operator*;
    using TPointer::operator->;
    using TPointer::operator bool;

    TUniquePtr(const TUniquePtr&) = delete;
    TUniquePtr& operator=(const TUniquePtr&) = delete;

    inline implicit TUniquePtr(decltype(nullptr) NullPointer = nullptr)
        : TPointer(NullPointer)
    {
    }

    inline TUniquePtr(TUniquePtr&& Other)
        : TPointer(Other.Pointer)
    {
        Other.Pointer = nullptr;
    }

    inline ~TUniquePtr()
    {
        DeleteIfValid();
    }

    inline TUniquePtr& operator=(TUniquePtr&& Other)
    {
        DeleteIfValid();

        TPointer::Pointer = Other.Pointer;
        Other.Pointer = nullptr;

        return *this;
    }

    template<typename NewPtrType, ECastOp CastOp>
    NODISCARD TUniquePtr<NewPtrType> Cast()
    {
        TUniquePtr<NewPtrType> NewPtr{CastPointer<NewPtrType, CastOp>()};

        TPointer::Pointer = nullptr;

        return NewPtr;
    }

    inline void Reset()
    {
        DeleteIfValid();
        TPointer::Pointer = nullptr;
    }

private:

    inline void DeleteIfValid()
    {
        if(TPointer::IsValid())
        {
            TPointer::Delete();
        }
    }

};

template<typename Type, typename... VarArgs>
inline TUniquePtr<Type> MakeUnique(VarArgs&&... Args)
{
    return TUniquePtr<Type>{new Type{MoveIfPossible(Args)...}};
}

template<typename Type>
TUniquePtr(TUniquePtr<Type>)->TUniquePtr<Type>;

template<typename Type, ESPMode ThreadMode = ESPMode::Default>
class TSharedPtr final : public PtrPri::TPointerBase<Type>, public PtrPri::TSharedBase<ThreadMode, PtrPri::ESharedMode::Strong>
{
    template<typename, ESPMode>
    friend class TSharedPtr;

    template<typename, ESPMode>
    friend class TSharedRef;

    template<typename, ESPMode>
    friend class TWeakPtr;

    template<typename InType, typename... VarArgs>
    friend TSharedPtr<InType> MakeShared(VarArgs&&...);

    template<typename InType, ESPMode InThreadMode, typename... VarArgs>
    friend TSharedPtr<InType, InThreadMode> MakeShared(VarArgs&&... Args);

    using TPointer = PtrPri::TPointerBase<Type>;
    using TShared = PtrPri::TSharedBase<ThreadMode, PtrPri::ESharedMode::Strong>;
    using TWeakEquiv = TWeakPtr<Type, ThreadMode>;
    using TSharedRefEquiv = TSharedRef<Type, ThreadMode>;

public:

    TSharedPtr(Type* InPtr, typename TShared::TReferenceCounter* RefCounter)
        : TPointer(InPtr)
        , TShared(RefCounter)
    {
    }

    explicit TSharedPtr(Type* RESTRICT InPtr NONNULL)
        : TPointer(InPtr)
        , TShared(new typename TShared::TReferenceCounter{})
    {
    }

    using PtrType = Type;
    using TPointer::operator*;
    using TPointer::operator->;
    using TPointer::operator bool;

    constexpr explicit TSharedPtr(ENoInit NoInit)
        : TPointer(NoInit)
        , TShared(NoInit)
    {
    }

    implicit TSharedPtr(decltype(nullptr) NullPointer = nullptr)
        : TPointer{NullPointer}
        , TShared{NullPointer}
    {
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    TSharedPtr(const PointerClass<OtherPtrType, ThreadMode>& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    TSharedPtr(PointerClass<OtherPtrType, ThreadMode>&& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        if constexpr(TypeTrait::AreTypesEqual<PointerClass<OtherPtrType, ThreadMode>, TWeakPtr<OtherPtrType, ThreadMode>>)
        {
            this->AddReference();
        }
        else
        {
            Other.ReferenceCounter = nullptr;
        }
    }

    TSharedPtr(const TSharedPtr& Other)
        : TPointer{Other.Pointer}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    TSharedPtr(TSharedPtr&& Other)
        : TPointer{Other.Pointer}
        , TShared{Other.ReferenceCounter}
    {
        Other.ReferenceCounter = nullptr;
    }

    ~TSharedPtr()
    {
        RemoveReferenceAndDelete();
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    TSharedPtr& operator=(const PointerClass<OtherPtrType, ThreadMode>& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = static_cast<PtrType*>(Other.Ptr());
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

        return *this;
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    TSharedPtr& operator=(PointerClass<OtherPtrType, ThreadMode>&& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = static_cast<PtrType*>(Other.Pointer);
        this->ReferenceCounter = Other.ReferenceCounter;

        if constexpr(TypeTrait::AreTypesEqual<PointerClass<OtherPtrType, ThreadMode>, TWeakPtr<OtherPtrType, ThreadMode>>)
        {
            this->AddReference();
        }
        else
        {
            Other.ReferenceCounter = nullptr;
        }

        return *this;
    }

    TSharedPtr& operator=(const TSharedPtr& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = Other.Pointer;
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

        return *this;
    }

    TSharedPtr& operator=(TSharedPtr&& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = Other.Pointer;
        this->ReferenceCounter = Other.ReferenceCounter;

        Other.ReferenceCounter = nullptr;

        return *this;
    }

    template<typename OtherType> requires(PtrPri::IsOtherPointer<OtherType, TSharedPtr, TWeakEquiv, TSharedRefEquiv>)
    auto operator<=>(const OtherType& Other) const
    {
        return this->Pointer <=> Other.Pointer;
    }

    void Reset()
    {
        RemoveReferenceAndDelete();

        this->Pointer = nullptr;
        this->ReferenceCounter = nullptr;
    }

    template<typename NewPtrType, ECastOp CastOp>
    NODISCARD TSharedPtr<NewPtrType, ThreadMode> Cast() &
    {
        TSharedPtr<NewPtrType, ThreadMode> NewPointer{TPointer::template CastPointer<NewPtrType*, CastOp>(), nullptr};

        if(NewPointer.IsValid())
        {
            NewPointer.ReferenceCounter = this->ReferenceCounter;
            this->AddReference();
        }

        return NewPointer;
    }

    template<typename NewPtrType, ECastOp CastOp>
    NODISCARD TSharedPtr<NewPtrType, ThreadMode> Cast() &&
    {
        TSharedPtr<NewPtrType, ThreadMode> NewPointer{TPointer::template CastPointer<NewPtrType*, CastOp>(), this->ReferenceCounter};

        ASSERT(NewPointer.IsValid());

        this->ReferenceCounter = nullptr;
        return NewPointer;
    }

private:

    void RemoveReferenceAndDelete()
    {
        if(TShared::RemoveReference() == 0)
        {
            TPointer::Delete();

            if(TShared::WeakRefCount() == 0)
            {
                TShared::Delete();
            }
        }
    }

};

template<typename Type, typename... VarArgs>
inline TSharedPtr<Type, ESPMode::Default> MakeShared(VarArgs&&... Args)
{
    return TSharedPtr<Type, ESPMode::Default>{new Type{MoveIfPossible(Args)...}};
}

template<typename Type, ESPMode ThreadMode, typename... VarArgs>
inline TSharedPtr<Type, ThreadMode> MakeShared(VarArgs&&... Args)
{
    return TSharedPtr<Type, ThreadMode>{new Type{MoveIfPossible(Args)...}};
}

template<typename Type>
TSharedPtr(TSharedPtr<Type, ESPMode::Default>)->TSharedPtr<Type, ESPMode::Default>;

template<typename Type>
TSharedPtr(TWeakPtr<Type, ESPMode::Default>)->TSharedPtr<Type, ESPMode::Default>;

template<typename Type, ESPMode ThreadMode = ESPMode::Default>
class TSharedRef final : public PtrPri::TPointerBase<Type>, public PtrPri::TSharedBase<ThreadMode, PtrPri::ESharedMode::Strong>
{
    template<typename, ESPMode>
    friend class TSharedPtr;

    template<typename, ESPMode>
    friend class TSharedRef;

    template<typename, ESPMode>
    friend class TWeakPtr;

    using TPointer = PtrPri::TPointerBase<Type>;
    using TShared = PtrPri::TSharedBase<ThreadMode, PtrPri::ESharedMode::Strong>;
    using TWeakEquiv = TWeakPtr<Type, ThreadMode>;
    using TSharedPtrEquiv = TSharedPtr<Type, ThreadMode>;

public:

    inline TSharedRef(Type* InPtr, typename TShared::TReferenceCounter* NONNULL RefCounter)
        : TPointer(InPtr)
        , TShared(RefCounter)
    {
    }

    inline explicit TSharedRef(Type* RESTRICT InPtr NONNULL)
        : TPointer(InPtr)
        , TShared(new typename TShared::TReferenceCounter{})
    {
    }

    template<typename InType, typename... VarArgs>
    friend TSharedPtr<InType> MakeShared(VarArgs&&...);

    template<typename InType, ESPMode InThreadMode, typename... VarArgs>
    friend TSharedPtr<InType, InThreadMode> MakeShared(VarArgs&&... Args);


    using PtrType = Type;
    using TPointer::operator*;
    using TPointer::operator->;
    using TPointer::operator bool;

    inline constexpr explicit TSharedRef(ENoInit NoInit)
        : TPointer(NoInit)
        , TShared(NoInit)
    {
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    inline TSharedRef(const PointerClass<OtherPtrType, ThreadMode>& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    inline TSharedRef(PointerClass<OtherPtrType, ThreadMode>&& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        if constexpr(TypeTrait::AreTypesEqual<PointerClass<OtherPtrType, ThreadMode>, TWeakPtr<OtherPtrType, ThreadMode>>)
        {
            this->AddReference();
        }
        else
        {
            Other.ReferenceCounter = nullptr;
        }
    }

    inline TSharedRef(const TSharedRef& Other)
        : TPointer{Other.Pointer}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    inline TSharedRef(TSharedRef&& Other)
        : TPointer{Other.Pointer}
        , TShared{Other.ReferenceCounter}
    {
        Other.ReferenceCounter = nullptr;
    }

    inline ~TSharedRef()
    {
        RemoveReferenceAndDelete();
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    inline TSharedRef& operator=(const PointerClass<OtherPtrType, ThreadMode>& Other)
    {
        ASSERT(Other.IsValid());

        RemoveReferenceAndDelete();

        this->Pointer = static_cast<PtrType*>(Other.Ptr());
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

        return *this;
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    inline TSharedRef& operator=(PointerClass<OtherPtrType, ThreadMode>&& Other)
    {
        ASSERT(Other.IsValid());

        RemoveReferenceAndDelete();

        this->Pointer = static_cast<PtrType*>(Other.Pointer);
        this->ReferenceCounter = Other.ReferenceCounter;

        if constexpr(TypeTrait::AreTypesEqual<PointerClass<OtherPtrType, ThreadMode>, TWeakPtr<OtherPtrType, ThreadMode>>)
        {
            this->AddReference();
        }
        else
        {
            Other.ReferenceCounter = nullptr;
        }

        return *this;
    }

    inline TSharedRef& operator=(const TSharedRef& Other)
    {
        ASSERT(Other.IsValid());

        RemoveReferenceAndDelete();

        this->Pointer = Other.Pointer;
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

        return *this;
    }

    inline TSharedRef& operator=(TSharedRef&& Other)
    {
        ASSERT(Other.IsValid());

        RemoveReferenceAndDelete();

        this->Pointer = Other.Pointer;
        this->ReferenceCounter = Other.ReferenceCounter;
        Other.ReferenceCounter = nullptr;

        return *this;
    }

    template<typename OtherType> requires(PtrPri::IsOtherPointer<OtherType, TSharedRef, TSharedPtrEquiv, TWeakEquiv>)
    inline auto operator<=>(const OtherType& Other) const
    {
        return this->Pointer <=> Other.Pointer;
    }

    inline bool IsValid() const
    {
        return true;
    }

    template<typename NewPtrType, ECastOp CastOp>
    NODISCARD TSharedRef<NewPtrType, ThreadMode> Cast() &
    {
        TSharedRef<NewPtrType, ThreadMode> NewPointer{TPointer::template CastPointer<NewPtrType*, CastOp>(), nullptr};

        ASSERT(NewPointer.IsValid());

        NewPointer.ReferenceCounter = this->ReferenceCounter;
        this->AddReference();
        return NewPointer;
    }

    template<typename NewPtrType, ECastOp CastOp>
    NODISCARD TSharedRef<NewPtrType, ThreadMode> Cast() &&
    {
        TSharedRef<NewPtrType, ThreadMode> NewPointer{TPointer::template CastPointer<NewPtrType*, CastOp>(), this->ReferenceCounter};

        ASSERT(NewPointer.IsValid());

        this->ReferenceCounter = nullptr;
        return NewPointer;
    }

private:

    void RemoveReferenceAndDelete()
    {
        if(TShared::RemoveReference() == 0)
        {
            TPointer::Delete();

            if(TShared::WeakRefCount() == 0)
            {
                TShared::Delete();
            }
        }
    }

};

template<typename Type>
TSharedRef(TSharedRef<Type, ESPMode::Default>)->TSharedRef<Type, ESPMode::Default>;

template<typename Type>
TSharedRef(TSharedPtr<Type, ESPMode::Default>)->TSharedRef<Type, ESPMode::Default>;

template<typename Type>
TSharedRef(TWeakPtr<Type, ESPMode::Default>)->TSharedRef<Type, ESPMode::Default>;

template<typename Type, ESPMode ThreadMode = ESPMode::Default>
class TWeakPtr final : public PtrPri::TPointerBase<Type>, public PtrPri::TSharedBase<ThreadMode, PtrPri::ESharedMode::Weak>
{
    template<typename, ESPMode>
    friend class TSharedPtr;

    template<typename, ESPMode>
    friend class TSharedRef;

    template<typename, ESPMode>
    friend class TWeakPtr;

    using TPointer = PtrPri::TPointerBase<Type>;
    using TShared = PtrPri::TSharedBase<ThreadMode, PtrPri::ESharedMode::Weak>;
    using TSharedPtrEquiv = TSharedPtr<Type, ThreadMode>;
    using TSharedRefEquiv = TSharedRef<Type, ThreadMode>;

public:

    inline TWeakPtr(Type* InPtr, typename TShared::TReferenceCounter* NONNULL RefCounter)
        : TPointer(InPtr)
        , TShared(RefCounter)
    {
    }

    inline constexpr explicit TWeakPtr(ENoInit NoInit)
        : TPointer(NoInit)
        , TShared(NoInit)
    {
    }

    using PtrType = Type;
    using TPointer::operator*;
    using TPointer::operator->;
    using TPointer::operator bool;

    inline explicit TWeakPtr(decltype(nullptr) NullPointer = nullptr)
        : TPointer{NullPointer}
        , TShared{NullPointer}
    {
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    inline TWeakPtr(const PointerClass<OtherPtrType, ThreadMode>& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    inline TWeakPtr(PointerClass<OtherPtrType, ThreadMode>&& Other)
         : TPointer{static_cast<PtrType*>(Other.Pointer)}
         , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    inline TWeakPtr(const TWeakPtr& Other)
        : TPointer{Other.Pointer}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    inline TWeakPtr(TWeakPtr&& Other)
        : TPointer{Other.Pointer}
        , TShared{Other.ReferenceCounter}
    {
        Other.ReferenceCounter = nullptr;
    }

    inline ~TWeakPtr()
    {
        RemoveReferenceAndDelete();
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    inline TWeakPtr& operator=(const PointerClass<OtherPtrType, ThreadMode>& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = static_cast<PtrType*>(Other.Ptr());
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

        return *this;
    }

    template<template<typename, ESPMode> typename PointerClass, typename OtherPtrType> requires(PtrPri::InitReqs<PtrType, OtherPtrType>)
    inline TWeakPtr& operator=(PointerClass<OtherPtrType, ThreadMode>&& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = static_cast<PtrType*>(Other.Ptr());
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

        return *this;
    }

    inline TWeakPtr& operator=(const TWeakPtr& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = Other.Pointer;
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

        return *this;
    }

    inline TWeakPtr& operator=(TWeakPtr&& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = Other.Pointer;
        this->ReferenceCounter = Other.ReferenceCounter;

        Other.ReferenceCounter = nullptr;

        return *this;
    }

    template<typename OtherType> requires(PtrPri::IsOtherPointer<OtherType, TWeakPtr, TSharedPtrEquiv, TSharedRefEquiv>)
    inline auto operator<=>(const OtherType& Other) const
    {
        return this->Pointer <=> Other.Pointer;
    }

    inline bool IsValid() const
    {
        return this->StrongRefCount() > 0;
    }

    void Reset()
    {
        RemoveReferenceAndDelete();

        this->Pointer = nullptr;
        this->ReferenceCounter = nullptr;
    }

    template<typename NewPtrType, ECastOp CastOp>
    NODISCARD TWeakPtr<NewPtrType, ThreadMode> Cast() &
    {
        TWeakPtr<NewPtrType, ThreadMode> NewPointer{TPointer::template CastPointer<NewPtrType*, CastOp>(), nullptr};

        if(NewPointer.IsValid())
        {
            NewPointer.ReferenceCounter = this->ReferenceCounter;
            this->AddReference();
        }

        return NewPointer;
    }

    template<typename NewPtrType, ECastOp CastOp>
    NODISCARD TWeakPtr<NewPtrType, ThreadMode> Cast() &&
    {
        TWeakPtr<NewPtrType, ThreadMode> NewPointer{TPointer::template CastPointer<NewPtrType*, CastOp>(), this->ReferenceCounter};

        ASSERT(NewPointer.IsValid());

        this->ReferenceCounter = nullptr;
        return NewPointer;
    }

private:

    void RemoveReferenceAndDelete()
    {
        if(TShared::RemoveReference() == 0)
        {
            TShared::Delete();
        }
    }

};

template<typename Type>
TWeakPtr(TSharedPtr<Type, ESPMode::Default>)->TWeakPtr<Type, ESPMode::Default>;

template<typename Type>
TWeakPtr(TWeakPtr<Type, ESPMode::Default>)->TWeakPtr<Type, ESPMode::Default>;

namespace TypeTrait
{
    template<typename T, template<typename> typename TT>
    struct IsUniquePointer : public FalseType{};

    template<typename T, template<typename, ESPMode> typename TT>
    struct IsSharedPointer : public FalseType{};

    template<template<typename> typename TT, typename T>
    struct IsUniquePointer<TT<T>, TT> : public TrueType{};

    template<template<typename, ESPMode> typename TT, typename T, ESPMode Mode>
    struct IsSharedPointer<TT<T, Mode>, TT> : public TrueType{};
}
