#pragma once

#include "Definitions.hpp"
#include "TypeTraits.hpp"
#include "Assert.hpp"
#include "Atomic.hpp"

enum class EThreadMode
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

template<typename, EThreadMode>
class TSharedPtr;

template<typename, EThreadMode>
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

        constexpr explicit TPointerBase(ENoInit)
        {
        }

        explicit TPointerBase(Type* InPointer)
            : Pointer{InPointer}
        {
        }

        virtual inline ~TPointerBase() = default;

        template<typename NewPtrType, ECastOp CastOp>
        constexpr NewPtrType CastPointer() const
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

        Type& operator*()
        {
            return *Pointer;
        }

        const Type& operator*() const
        {
            return *Pointer;
        }

        Type* operator->()
        {
            return Pointer;
        }

        const Type* operator->() const
        {
            return Pointer;
        }

        Type* Ptr()
        {
            return Pointer;
        }

        const Type* Ptr() const
        {
            return Pointer;
        }

    protected:

        Type* Pointer;

    };

    struct PACKED MAY_ALIAS FReferenceCounter
    {
        INLINE constexpr FReferenceCounter()
            : StrongReferenceCount{1}
            , WeakReferenceCount{1}
        {
        }

        int16 StrongReferenceCount;
        int16 WeakReferenceCount;
    };

    extern uint32 EndRefCountOffset;
    extern FReferenceCounter* StartRefCountPtr;

    struct PACKED FReferenceCounterWrapper
    {
        FReferenceCounterWrapper()
            : Offset{OFFSET_NONE}
        {
        }

        FReferenceCounterWrapper(ENoInit NoInit)
        {
        }

        FReferenceCounterWrapper(uint32 InOffset)
            : Offset{InOffset}
        {
        }

        FReferenceCounter* operator->()
        {
            return StartRefCountPtr + Offset;
        }

        const FReferenceCounter* operator->() const
        {
            return StartRefCountPtr + Offset;
        }

        const FReferenceCounterWrapper& operator=(uint32 NewOffset)
        {
            Offset = NewOffset;
            return *this;
        }

        uint32 Offset;
    };

    uint32 NewReferenceCounter();

    enum class ESharedMode
    {
        Strong,
        Weak
    };

    template<EThreadMode ThreadMode, ESharedMode SharedMode>
    class INTERNAL_LINKAGE TSharedBase
    {
    protected:

        template<typename, typename, template<typename, EThreadMode> typename, EThreadMode>
        friend struct TObjectCastSmartPointerHelper;

        constexpr explicit TSharedBase(ENoInit)
        {
        }

        INLINE explicit TSharedBase(FReferenceCounterWrapper InRefCounter)
            : ReferenceCounter{InRefCounter}
        {
        }

        virtual ~TSharedBase() = default;

    public:

        INLINE int16 StrongRefCount() const
        {
            if(IsRefCounterValid())
            {
                return ReferenceCounter->StrongReferenceCount;
            }
            return -1;
        }

        INLINE int16 WeakRefCount() const
        {
            if(IsRefCounterValid())
            {
                return ReferenceCounter->WeakReferenceCount;
            }
            return -1;
        }

        INLINE bool IsRefCounterValid() const
        {
            return ReferenceCounter.Offset != OFFSET_NONE;
        }

    protected:

        INLINE int16 AddReference()
        {
            if(IsRefCounterValid())
            {
                if constexpr(SharedMode == ESharedMode::Strong)
                {
                    if constexpr(ThreadMode == EThreadMode::Unsafe)
                    {
                        ++ReferenceCounter->WeakReferenceCount;
                        return ++ReferenceCounter->StrongReferenceCount;
                    }
                    else if constexpr(ThreadMode == EThreadMode::Safe)
                    {
                        Atomic::Add(&ReferenceCounter->WeakReferenceCount, static_cast<int16>(1));
                        return Atomic::Add(&ReferenceCounter->StrongReferenceCount, static_cast<int16>(1)) + 1;
                    }
                }
                else if constexpr(SharedMode == ESharedMode::Weak)
                {
                    if constexpr(ThreadMode == EThreadMode::Unsafe)
                    {
                        return ++ReferenceCounter->WeakReferenceCount;
                    }
                    else if constexpr(ThreadMode == EThreadMode::Safe)
                    {
                        return Atomic::Add(&ReferenceCounter->WeakReferenceCount, static_cast<int16>(1)) + 1;
                    }
                }
            }
            return -1;
        }

        INLINE int16 RemoveReference()
        {
            if(IsRefCounterValid())
            {
                if constexpr(SharedMode == ESharedMode::Strong)
                {
                    if constexpr(ThreadMode == EThreadMode::Unsafe)
                    {
                        --ReferenceCounter->WeakReferenceCount;
                        return --ReferenceCounter->StrongReferenceCount;
                    }
                    else if constexpr(ThreadMode == EThreadMode::Safe)
                    {
                        Atomic::Subtract(&ReferenceCounter->WeakReferenceCount, static_cast<int16>(1));
                        return Atomic::Subtract(&ReferenceCounter->StrongReferenceCount, static_cast<int16>(1)) - 1;
                    }
                }
                else if constexpr(SharedMode == ESharedMode::Weak)
                {
                    if constexpr(ThreadMode == EThreadMode::Unsafe)
                    {
                        return --ReferenceCounter->WeakReferenceCount;
                    }
                    else if constexpr(ThreadMode == EThreadMode::Safe)
                    {
                        return Atomic::Subtract(&ReferenceCounter->WeakReferenceCount, static_cast<int16>(1)) - 1;
                    }
                }
            }
            return -1;
        }

        FReferenceCounterWrapper ReferenceCounter;
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

public:

    using PtrType = Type;
    using TPointer::operator*;
    using TPointer::operator->;

    explicit constexpr TUniquePtr(ENoInit NoInit)
        : TPointer{NoInit}
    {
    }

    explicit TUniquePtr(Type* RESTRICT InPointer NONNULL)
        : TPointer{InPointer}
    {
    }

    TUniquePtr(const TUniquePtr&) = delete;
    TUniquePtr& operator=(const TUniquePtr&) = delete;

    implicit TUniquePtr(decltype(nullptr) NullPointer = nullptr)
        : TPointer{NullPointer}
    {
    }

    TUniquePtr(TUniquePtr&& Other)
        : TPointer{Other.Pointer}
    {
        Other.Pointer = nullptr;
    }

    ~TUniquePtr()
    {
        DeleteIfValid();
    }

    TUniquePtr& operator=(TUniquePtr&& Other)
    {
        DeleteIfValid();

        this->Pointer = Other.Pointer;
        Other.Pointer = nullptr;

        return *this;
    }

    explicit operator bool() const
    {
        return IsValid();
    }

    bool IsValid() const
    {
        return this->Pointer != nullptr;
    }

    template<typename NewPtrType, ECastOp CastOp>
    NODISCARD TUniquePtr<NewPtrType> Cast()
    {
        TUniquePtr<NewPtrType> NewPtr{CastPointer<NewPtrType, CastOp>()};
        this->Pointer = nullptr;
        return NewPtr;
    }

    void Reset()
    {
        if(IsValid())
        {
            delete this->Pointer;
            this->Pointer = nullptr;
        }
    }

private:

    void DeleteIfValid()
    {
        if(IsValid())
        {
            delete this->Pointer;
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

template<typename Type, EThreadMode ThreadMode = EThreadMode::Default>
class TSharedPtr final : public PtrPri::TPointerBase<Type>, public PtrPri::TSharedBase<ThreadMode, PtrPri::ESharedMode::Strong>
{
    template<typename, EThreadMode>
    friend class TSharedPtr;

    template<typename, EThreadMode>
    friend class TWeakPtr;

    template<typename InType, typename... VarArgs>
    friend TSharedPtr<InType> MakeShared(VarArgs&&...);

    template<typename InType, EThreadMode InThreadMode, typename... VarArgs>
    friend TSharedPtr<InType, InThreadMode> MakeShared(VarArgs&&... Args);

    using TPointer = PtrPri::TPointerBase<Type>;
    using TShared = PtrPri::TSharedBase<ThreadMode, PtrPri::ESharedMode::Strong>;
    using TWeakEquiv = TWeakPtr<Type, ThreadMode>;

public:

    using PtrType = Type;
    using TPointer::operator*;
    using TPointer::operator->;

    TSharedPtr(Type* InPtr, PtrPri::FReferenceCounterWrapper RefCounter)
        : TPointer{InPtr}
        , TShared{RefCounter}
    {
    }

    template<typename InPtrType = Type>
    explicit TSharedPtr(InPtrType* InPtr NONNULL)
        : TPointer{InPtr}
        , TShared{PtrPri::NewReferenceCounter()}
    {
    }

    constexpr explicit TSharedPtr(ENoInit NoInit)
        : TPointer{NoInit}
        , TShared{NoInit}
    {
    }

    implicit TSharedPtr(decltype(nullptr) NullPointer = nullptr)
        : TPointer{NullPointer}
        , TShared{OFFSET_NONE}
    {
    }

    TSharedPtr(const TWeakEquiv& Other)
        : TPointer{Other.Pointer}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
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
        Other.ReferenceCounter = OFFSET_NONE;
    }

    template<typename OtherPtrType> requires(TypeTrait::IsStaticCastable<PtrType*, OtherPtrType*>)
    TSharedPtr(const TSharedPtr<OtherPtrType, ThreadMode>& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    template<typename OtherPtrType> requires(TypeTrait::IsStaticCastable<PtrType*, OtherPtrType*>)
    TSharedPtr(TSharedPtr<OtherPtrType, ThreadMode>&& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        Other.ReferenceCounter = OFFSET_NONE;
    }

    ~TSharedPtr()
    {
        RemoveReferenceAndDelete();
    }

    TSharedPtr& operator=(const TWeakEquiv& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = Other.Pointer;
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

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

        Other.Pointer = nullptr;
        Other.ReferenceCounter = OFFSET_NONE;

        return *this;
    }

    template<typename OtherPtrType> requires(TypeTrait::IsStaticCastable<PtrType*, OtherPtrType*>)
    TSharedPtr& operator=(const TSharedPtr<OtherPtrType, ThreadMode>& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = static_cast<PtrType*>(Other.Pointer);
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

        return *this;
    }

    template<typename OtherPtrType> requires(TypeTrait::IsStaticCastable<PtrType*, OtherPtrType*>)
    TSharedPtr& operator=(TSharedPtr&& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = static_cast<PtrType*>(Other.Pointer);
        this->ReferenceCounter = Other.ReferenceCounter;

        Other.Pointer = nullptr;
        Other.ReferenceCounter = OFFSET_NONE;

        return *this;
    }

    template<typename OtherType> requires(PtrPri::IsOtherPointer<OtherType, TSharedPtr, TWeakEquiv>)
    auto operator<=>(const OtherType& Other) const
    {
        return this->Pointer <=> Other.Pointer;
    }

    explicit operator bool() const
    {
        return IsValid();
    }

    bool IsValid() const
    {
        return this->Pointer != nullptr;
    }

    void Reset()
    {
        RemoveReferenceAndDelete();

        this->Pointer = nullptr;
        this->ReferenceCounter = OFFSET_NONE;
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

        this->ReferenceCounter = OFFSET_NONE;
        return NewPointer;
    }

private:

    void RemoveReferenceAndDelete()
    {
        if(TShared::RemoveReference() == 0)
        {
            delete this->Pointer;
        }
    }

};

template<typename Type, typename... VarArgs>
inline TSharedPtr<Type, EThreadMode::Default> MakeShared(VarArgs&&... Args)
{
    return TSharedPtr<Type, EThreadMode::Default>{new Type{MoveIfPossible(Args)...}};
}

template<typename Type, EThreadMode ThreadMode, typename... VarArgs>
inline TSharedPtr<Type, ThreadMode> MakeShared(VarArgs&&... Args)
{
    return TSharedPtr<Type, ThreadMode>{new Type{MoveIfPossible(Args)...}};
}

template<typename Type>
TSharedPtr(TSharedPtr<Type, EThreadMode::Default>)->TSharedPtr<Type, EThreadMode::Default>;

template<typename Type>
TSharedPtr(TWeakPtr<Type, EThreadMode::Default>)->TSharedPtr<Type, EThreadMode::Default>;

template<typename Type, EThreadMode ThreadMode = EThreadMode::Default>
class TWeakPtr final : public PtrPri::TPointerBase<Type>, public PtrPri::TSharedBase<ThreadMode, PtrPri::ESharedMode::Weak>
{
    template<typename, EThreadMode>
    friend class TSharedPtr;

    template<typename, EThreadMode>
    friend class TWeakPtr;

    using TPointer = PtrPri::TPointerBase<Type>;
    using TShared = PtrPri::TSharedBase<ThreadMode, PtrPri::ESharedMode::Weak>;
    using TSharedEquiv = TSharedPtr<Type, ThreadMode>;

public:

    using PtrType = Type;
    using TPointer::operator*;
    using TPointer::operator->;

    TWeakPtr(Type* InPtr, PtrPri::FReferenceCounterWrapper RefCounter)
        : TPointer{InPtr}
        , TShared{RefCounter}
    {
    }

    constexpr explicit TWeakPtr(ENoInit NoInit)
        : TPointer{NoInit}
        , TShared{NoInit}
    {
    }

    constexpr explicit TWeakPtr(decltype(nullptr) NullPointer = nullptr)
        : TPointer{NullPointer}
        , TShared{OFFSET_NONE}
    {
    }

    TWeakPtr(const TSharedEquiv& Other)
     : TPointer{Other.Pointer}
     , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    TWeakPtr(const TWeakPtr& Other)
        : TPointer{Other.Pointer}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    TWeakPtr(TWeakPtr&& Other)
        : TPointer{Other.Pointer}
        , TShared{Other.ReferenceCounter}
    {
        Other.ReferenceCounter = OFFSET_NONE;
    }

    ~TWeakPtr()
    {
        this->RemoveReference();
    }

    TWeakPtr& operator=(const TSharedEquiv& Other)
    {
        this->RemoveReference();

        this->Pointer = Other.Ptr();
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

        return *this;
    }

    TWeakPtr& operator=(const TWeakPtr& Other)
    {
        this->RemoveReference();

        this->Pointer = Other.Pointer;
        this->ReferenceCounter = Other.ReferenceCounter;
        this->AddReference();

        return *this;
    }

    TWeakPtr& operator=(TWeakPtr&& Other)
    {
        this->RemoveReference();

        this->Pointer = Other.Pointer;
        this->ReferenceCounter = Other.ReferenceCounter;

        Other.ReferenceCounter = OFFSET_NONE;

        return *this;
    }

    template<typename OtherType> requires(PtrPri::IsOtherPointer<OtherType, TWeakPtr, TSharedEquiv>)
    auto operator<=>(const OtherType& Other) const
    {
        return this->Pointer <=> Other.Pointer;
    }

    bool IsValid() const
    {
        return this->StrongRefCount() > 0;
    }

    void Reset()
    {
        this->RemoveReference();

        this->Pointer = nullptr;
        this->ReferenceCounter = OFFSET_NONE;
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

        this->ReferenceCounter = OFFSET_NONE;
        return NewPointer;
    }
};

template<typename Type>
TWeakPtr(TSharedPtr<Type, EThreadMode::Default>)->TWeakPtr<Type, EThreadMode::Default>;

template<typename Type>
TWeakPtr(TWeakPtr<Type, EThreadMode::Default>)->TWeakPtr<Type, EThreadMode::Default>;

namespace TypeTrait
{
    template<typename T, template<typename> typename TT>
    struct IsUniquePointer : public FalseType{};

    template<typename T, template<typename, EThreadMode> typename TT>
    struct IsSharedPointer : public FalseType{};

    template<template<typename> typename TT, typename T>
    struct IsUniquePointer<TT<T>, TT> : public TrueType{};

    template<template<typename, EThreadMode> typename TT, typename T, EThreadMode Mode>
    struct IsSharedPointer<TT<T, Mode>, TT> : public TrueType{};
}
