#pragma once

#include "Definitions.hpp"
#include "TypeTraits.hpp"
#include "Assert.hpp"
#include "Atomic.hpp"
#include "Object/Allocator.hpp"

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
    enum class ESharedMode
    {
        Strong,
        Weak
    };

    template<typename Type>
    class INTERNAL_LINKAGE TPointerBase
    {
        template<typename, typename, template<typename, EThreadMode> typename, EThreadMode>
        friend struct TObjectCastSmartPointerHelper;

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
        uint16 StrongReferenceCount;
        uint16 WeakReferenceCount;
    };

    extern uint64 EndRefCountOffset;
    extern FReferenceCounter* StartRefCountPtr; //the first reference counter is reserved for invalid offsets

    struct FReferenceCounterWrapper
    {
        INLINE FReferenceCounterWrapper()
            : Offset{OFFSET_NONE}
        {
        }

        INLINE FReferenceCounterWrapper(ENoInit)
        {
        }

        INLINE FReferenceCounterWrapper(uint64 InOffset)
            : Offset{InOffset}
        {
        }

        INLINE const FReferenceCounterWrapper& operator=(uint64 NewOffset)
        {
            Offset = NewOffset;
            return *this;
        }

        INLINE FReferenceCounter* operator->() const
        {
            return StartRefCountPtr + (Offset * (Offset != UINT64_MAX));
        }

        INLINE FReferenceCounter& operator*() const
        {
            return StartRefCountPtr[(Offset * (Offset != UINT64_MAX))];
        }

        uint64 Offset;
    };

    uint64 FindNewRefCounterOffset();

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

        ///@return the strong reference count or UINT16_MAX if invalid
        INLINE uint16 StrongRefCount() const
        {
            return ReferenceCounter->StrongReferenceCount;
        }

        ///@return the weak reference count (always >= strong ref count) or UINT16_MAX if invalid
        INLINE uint16 WeakRefCount() const
        {
            return ReferenceCounter->WeakReferenceCount;
        }

        INLINE bool IsRefCounterValid() const
        {
            return ReferenceCounter.Offset != UINT64_MAX;
        }

    protected:

        INLINE void AddReference()
        {
            const uint16 AmountToAdd{IsRefCounterValid()};

            if constexpr(SharedMode == ESharedMode::Strong)
            {
                if constexpr(ThreadMode == EThreadMode::Unsafe)
                {
                    ReferenceCounter->WeakReferenceCount += AmountToAdd;
                    ReferenceCounter->StrongReferenceCount += AmountToAdd;
                }
                else if constexpr(ThreadMode == EThreadMode::Safe)
                {
                    Atomic::Add(&ReferenceCounter->WeakReferenceCount, AmountToAdd);
                    Atomic::Add(&ReferenceCounter->StrongReferenceCount, AmountToAdd);
                }
            }
            else if constexpr(SharedMode == ESharedMode::Weak)
            {
                if constexpr(ThreadMode == EThreadMode::Unsafe)
                {
                    ++ReferenceCounter->WeakReferenceCount;
                }
                else if constexpr(ThreadMode == EThreadMode::Safe)
                {
                    Atomic::Add(&ReferenceCounter->WeakReferenceCount, AmountToAdd);
                }
            }
        }

        ///@returns true if ref counter is valid and count is now 0, otherwise false
        INLINE bool RemoveReference()
        {
            const uint16 AmountToSubtract{IsRefCounterValid()};

            if constexpr(SharedMode == ESharedMode::Strong)
            {
                if constexpr(ThreadMode == EThreadMode::Unsafe)
                {
                    FReferenceCounter& Counter{*ReferenceCounter};

                    Counter.WeakReferenceCount -= AmountToSubtract;
                    return (Counter.StrongReferenceCount -= AmountToSubtract) == 0;
                }
                else if constexpr(ThreadMode == EThreadMode::Safe)
                {
                    FReferenceCounter& Counter{*ReferenceCounter};

                    Atomic::Subtract(&Counter.WeakReferenceCount, AmountToSubtract);
                    return Atomic::Subtract(&Counter.StrongReferenceCount, AmountToSubtract) == 1;
                }
            }
            else if constexpr(SharedMode == ESharedMode::Weak)
            {
                if constexpr(ThreadMode == EThreadMode::Unsafe)
                {
                    return (ReferenceCounter->WeakReferenceCount -= AmountToSubtract) == 0;
                }
                else if constexpr(ThreadMode == EThreadMode::Safe)
                {
                    return Atomic::Subtract(&ReferenceCounter->WeakReferenceCount, AmountToSubtract) == 1;
                }
            }
        }

        FReferenceCounterWrapper ReferenceCounter;
    };
}

template<typename Type>
class TUniquePtr final : public PtrPri::TPointerBase<Type>
{
    template<typename InType, typename... VarArgs>
    friend TUniquePtr<InType> MakeUnique(VarArgs&&...);

    template<typename Base, typename Derived, typename... VarArgs>
    friend TUniquePtr<Base> MakeUnique(VarArgs&&... Args);

    using TPointer = PtrPri::TPointerBase<Type>;

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

public:

    using PtrType = Type;
    using TPointer::operator*;
    using TPointer::operator->;

    TSharedPtr(Type* InPtr, PtrPri::FReferenceCounterWrapper RefCounter)
        : TPointer{InPtr}
        , TShared{RefCounter}
    {
    }

    explicit TSharedPtr(PtrType* InPtr NONNULL)
        : TPointer{InPtr}
        , TShared{PtrPri::FindNewRefCounterOffset()}
    {
    }

    constexpr explicit TSharedPtr(ENoInit NoInit)
        : TPointer{NoInit}
        , TShared{NoInit}
    {
    }

    implicit TSharedPtr(decltype(nullptr) NullPointer = nullptr)
        : TPointer{NullPointer}
        , TShared{UINT64_MAX}
    {
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
        Other.ReferenceCounter = UINT64_MAX;
    }

    template<typename OtherPtrType> requires(TypeTrait::IsStaticCastable<PtrType*, OtherPtrType*>)
    TSharedPtr(const TWeakPtr<OtherPtrType, ThreadMode>& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
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
        Other.ReferenceCounter = UINT64_MAX;
    }

    ~TSharedPtr()
    {
        RemoveReferenceAndDelete();
    }

    template<typename OtherPtrType> requires(TypeTrait::IsStaticCastable<PtrType*, OtherPtrType*>)
    TSharedPtr& operator=(const TWeakPtr<OtherPtrType, ThreadMode>& Other)
    {
        RemoveReferenceAndDelete();

        this->Pointer = static_cast<PtrType*>(Other.Pointer);
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
        Other.ReferenceCounter = UINT64_MAX;

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
        Other.ReferenceCounter = UINT64_MAX;

        return *this;
    }

    template<typename OtherType>
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
        this->ReferenceCounter = UINT64_MAX;
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

        this->ReferenceCounter = UINT64_MAX;
        return NewPointer;
    }

private:

    void RemoveReferenceAndDelete()
    {
        if(TShared::RemoveReference())
        {
            if constexpr(TypeTrait::IsObjectClass<Type>)
            {
                if constexpr(!TypeTrait::IsTriviallyDestructible<Type>)
                {
                    this->Pointer->Type::~Type();
                }

                FObjectAllocationManager::Instance().FreeObject(this->Pointer);
            }
            else
            {
                delete this->Pointer;
            }
        }
    }

};

template<typename Type, typename... VarArgs> requires(!TypeTrait::IsObjectClass<Type>)
inline TSharedPtr<Type, EThreadMode::Default> MakeShared(VarArgs&&... Args)
{
    return TSharedPtr<Type, EThreadMode::Default>{new Type{MoveIfPossible(Args)...}, PtrPri::FindNewRefCounterOffset()};
}

template<typename Type, EThreadMode ThreadMode, typename... VarArgs> requires(!TypeTrait::IsObjectClass<Type>)
inline TSharedPtr<Type, ThreadMode> MakeShared(VarArgs&&... Args)
{
    return TSharedPtr<Type, ThreadMode>{new Type{MoveIfPossible(Args)...}, PtrPri::FindNewRefCounterOffset()};
}

template<typename Type, typename... VarArgs> requires(TypeTrait::IsObjectClass<Type>)
HOT inline TSharedPtr<Type, EThreadMode::Default> MakeShared(VarArgs&&... Args)
{
    return TSharedPtr<Type, EThreadMode::Default>{FObjectAllocationManager::Instance().template PlaceObject<Type>(MoveIfPossible(Args)...), PtrPri::FindNewRefCounterOffset()};
}

template<typename Type, EThreadMode ThreadMode, typename... VarArgs> requires(TypeTrait::IsObjectClass<Type>)
HOT inline TSharedPtr<Type, ThreadMode> MakeShared(VarArgs&&... Args)
{
    return TSharedPtr<Type, ThreadMode>{FObjectAllocationManager::Instance().template PlaceObject<Type>(MoveIfPossible(Args)...), PtrPri::FindNewRefCounterOffset()};
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
        , TShared{UINT64_MAX}
    {
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
        Other.ReferenceCounter = UINT64_MAX;
    }

    template<typename OtherPtrType> requires(TypeTrait::IsStaticCastable<PtrType*, OtherPtrType*>)
    TWeakPtr(const TWeakPtr<OtherPtrType, ThreadMode>& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    template<typename OtherPtrType> requires(TypeTrait::IsStaticCastable<PtrType*, OtherPtrType*>)
    TWeakPtr(TWeakPtr<OtherPtrType, ThreadMode>&& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        Other.ReferenceCounter = UINT64_MAX;
    }

    template<typename OtherPtrType> requires(TypeTrait::IsStaticCastable<PtrType*, OtherPtrType*>)
    TWeakPtr(const TSharedPtr<OtherPtrType, ThreadMode>& Other)
        : TPointer{static_cast<PtrType*>(Other.Pointer)}
        , TShared{Other.ReferenceCounter}
    {
        this->AddReference();
    }

    ~TWeakPtr()
    {
        this->RemoveReference();
    }

    template<typename OtherPtrType> requires(TypeTrait::IsStaticCastable<PtrType*, OtherPtrType*>)
    TWeakPtr& operator=(const TSharedPtr<OtherPtrType, ThreadMode>& Other)
    {
        this->RemoveReference();

        this->Pointer = static_cast<PtrType*>(Other.Pointer);
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

        Other.ReferenceCounter = UINT64_MAX;

        return *this;
    }

    template<typename OtherType>
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
        this->ReferenceCounter = UINT64_MAX;
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

        this->ReferenceCounter = UINT64_MAX;
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
