#pragma once

#include "Definitions.hpp"
#include "Memory.hpp"
#include "Math.hpp"
#include "TypeTraits.hpp"
#include "Tuple.hpp"
#include "Assert.hpp"
#include <initializer_list>

#ifndef WITH_ARRAY_OFFSET_CHECKS
#define WITH_ARRAY_OFFSET_CHECKS 1
#endif

template<typename>
class TDynamicArray;

template<typename, int64 NumElements>
class TStaticArray;

namespace TypeTrait
{
    template<typename>
    struct IsStaticArray : public FalseType{};

    template<typename T, int64 N>
    struct IsStaticArray<TStaticArray<T, N>> : public TrueType{};

    template<typename>
    struct IsDynamicArray : public FalseType{};

    template<typename T>
    struct IsDynamicArray<TDynamicArray<T>> : public TrueType{};
}

template<typename ElementType>
class TRangedIterator
{
public:

    explicit constexpr TRangedIterator(ElementType* InPointer)
        : ElementPtr(InPointer)
    {
    }

    constexpr TRangedIterator& operator++()
    {
        ++ElementPtr;
        return *this;
    }

    constexpr TRangedIterator& operator--()
    {
        --ElementPtr;
        return *this;
    }

    constexpr ElementType& operator*()
    {
        return *ElementPtr;
    }

    inline constexpr friend bool operator!=(const TRangedIterator Left, const TRangedIterator Right)
    {
        return Left.ElementPtr != Right.ElementPtr;
    }

private:

    ElementType* ElementPtr;
};

template<typename ElementType, int64 NumElements>
class TStaticArray final
{
private:

    using TConstIterator = TRangedIterator<const ElementType>;
    using TMutableIterator = TRangedIterator<ElementType>;

public:

    constexpr TStaticArray(void) = default;

    constexpr explicit TStaticArray(const ElementType* NONNULL ElementPtr, int64 Num)
    {
        while(Num > 0)
        {
            --Num;
            Array[Num] = ElementPtr[Num];
        }
    }

    template<typename... Initializers> requires(sizeof...(Initializers) == NumElements)
    explicit constexpr TStaticArray(Initializers&&... InElements)
        : Array{MoveIfPossible(InElements)...}
    {
    }

    explicit constexpr TStaticArray(const ElementType* Value)
    {
        Memory::Copy(Array, Value, Num() * sizeof(ElementType));
    }

    explicit constexpr TStaticArray(const uint8 Value)
    {
        if(__builtin_is_constant_evaluated() && (sizeof(ElementType) == 1))
        {
            for(int64 Index{0}; Index < Num(); ++Index)
            {
                Array[Index] = Value;
            }
        }
        else
        {
            Memory::Set(Array, Value, Num() * sizeof(ElementType));
        }
    }

    constexpr bool IsIndexValid(const int64 Index) const
    {
        return Index >= 0 && Index < NumElements;
    }

    template<typename ReturnType = int64> requires(TypeTrait::IsInteger<ReturnType>)
    static constexpr ReturnType Num()
    {
        return NumElements;
    }

    template<typename ReturnType = int64> requires(TypeTrait::IsInteger<ReturnType>)
    constexpr ReturnType UsedSize() const
    {
        return NumElements * sizeof(ElementType);
    }

    constexpr ElementType* GetData()
    {
        return &Array[0];
    }

    constexpr const ElementType* GetData() const
    {
        return &Array[0];
    }

    constexpr ElementType* Data()
    {
        return &Array[0];
    }

    constexpr const ElementType* Data() const
    {
        return &Array[0];
    }

    constexpr ElementType* operator-(const int64 DecrementAmount)
    {
        return Array - DecrementAmount;
    }

    constexpr ElementType* operator+(const int64 DecrementAmount)
    {
        return Array + DecrementAmount;
    }

    constexpr const ElementType* operator-(const int64 DecrementAmount) const
    {
        return Array - DecrementAmount;
    }

    constexpr const ElementType* operator+(const int64 DecrementAmount) const
    {
        return Array + DecrementAmount;
    }

    constexpr ElementType& operator[](const int64 Index)
    {
        ASSERT(IsIndexValid(Index));
        return Array[Index];
    }

    constexpr const ElementType& operator[](const int64 Index) const
    {
        ASSERT(IsIndexValid(Index));
        return Array[Index];
    }

    constexpr TConstIterator begin() const
    {
        return TConstIterator{Array};
    }

    constexpr TMutableIterator begin()
    {
        return TMutableIterator{Array};
    }

    constexpr TConstIterator end() const
    {
        return TConstIterator{&Array[NumElements]};
    }

    constexpr TMutableIterator end()
    {
        return TMutableIterator{&Array[NumElements]};
    }

    ElementType Array[NumElements]; //public because constexpr
};

template<typename ElementType, typename... Elements>
TStaticArray(ElementType, Elements...)->TStaticArray<ElementType, sizeof...(Elements) + 1>;

template<typename ElementType>
TStaticArray(ElementType)->TStaticArray<ElementType, 1>;

/*
 * this namespace contains some numbers that control how the array allocates and deallocates memory
 * specialize for a certain element type if you want to override the default behaviour
 */
namespace ArrayConstants
{
    //beyond this size in bytes we wont allocate any extra memory
    template<typename>
    inline consteval int64 ReserveSizeLimit()
    {
        return 2048LL;
    }

    //the maximum extra amount of bytes we want to allocate for
    template<typename ElementType>
    inline consteval int64 ReserveExtraValue()
    {
        return 25LL * sizeof(ElementType);
    }

    //the minimum amount of unused memory in bytes required for a de-allocation to occur
    template<typename>
    inline consteval int64 UnusedSizeLimit()
    {
        return 1024LL;
    }
}

template<typename ElementType>
class alignas(16) TDynamicArray final
{
private:

    template<typename ItElementType>
    class TRangedArrayIterator
    {
    public:

#if WITH_ARRAY_OFFSET_CHECKS
        explicit TRangedArrayIterator(ItElementType* InPointer, const int64& ArrayNum)
            : ElementPtr(InPointer)
            , CurrentArrayNum(ArrayNum)
            , InitialArrayNum(ArrayNum)
#else
        explicit inline TRangedArrayIterator(ItElementType* InPointer)
            : ElementPtr(InPointer)
#endif
        {
        }

        TRangedArrayIterator& operator++()
        {
            ++ElementPtr;
            return *this;
        }

        TRangedArrayIterator& operator--()
        {
            --ElementPtr;
            return *this;
        }

        ItElementType& operator*()
        {
            return *ElementPtr;
        }

    private:

#if WITH_ARRAY_OFFSET_CHECKS
        inline friend bool operator!=(const TRangedArrayIterator& Left, const TRangedArrayIterator& Right)
        {
            ASSERT(Left.CurrentArrayNum == Left.InitialArrayNum, "array changed during ranged-for iteration");

            return Left.ElementPtr != Right.ElementPtr;
        }
#else
        inline friend bool operator!=(const TRangedArrayIterator Left, const TRangedArrayIterator Right)
        {
            return Left.ElementPtr != Right.ElementPtr;
        }
#endif
        ItElementType* ElementPtr;

#if WITH_ARRAY_OFFSET_CHECKS
        const volatile int64& CurrentArrayNum;

        const int64 InitialArrayNum;
#endif
    };

    using TConstIterator = TRangedArrayIterator<const ElementType>;
    using TMutableIterator = TRangedArrayIterator<ElementType>;

    template<typename Iterator, typename PointerType>
    INLINE Iterator MakeIterator(PointerType Pointer) const
    {
#if WITH_ARRAY_OFFSET_CHECKS
        return Iterator{Pointer, LastIndex};
#else
        return Iterator{Pointer};
#endif
    }

    template<typename>
    friend class TArray;

    template<typename>
    friend class TRangedArrayIterator;

public:

    TDynamicArray()
        : ElementPointer(nullptr)
        , LastIndex(-1)
    {
    }

    TDynamicArray(const TDynamicArray& Other)
        : ElementPointer(nullptr)
        , LastIndex(-1)
    {
        ReserveUndefined(Other.Num());

        InitializeFromOther(Other);
    }

    TDynamicArray(TDynamicArray&& Other)
        : ElementPointer(Other.ElementPointer)
        , LastIndex(Other.LastIndex)
    {
        Other.ElementPointer = nullptr;
        Other.LastIndex = -1;
    }

    //used to initialize from a malloced pointer
    TDynamicArray(ElementType* RESTRICT NONNULL Pointer, const int64 NumElements, EChooseConstructor)
        : ElementPointer(Pointer)
        , LastIndex(NumElements - 1)
    {
        ASSERT(Pointer);
    }

    //used to reserve a set amount
    explicit TDynamicArray(const int64 NumElementsToAllocate, EChooseConstructor)
        : ElementPointer{Memory::Allocate<ElementType>(NumElementsToAllocate * ElementSize())}
        , LastIndex(-1)
    {
    }

    TDynamicArray(const ElementType* RawArray, const int64 NumElements)
        : LastIndex(-1)
    {
        ASSERT(RawArray);

        ElementPointer = Memory::Allocate<ElementType>(NumElements * ElementSize());

        if constexpr(TypeTrait::IsTriviallyConstructible<ElementType>)
        {
            LastIndex = NumElements - 1;
            Memory::Copy(ElementPointer, RawArray, NumElements * ElementSize());
        }
        else
        {
            while(LastIndex < NumElements - 1)
            {
                ++LastIndex;
                ConstructElement(LastIndex, RawArray[LastIndex]);
            }
        }
    }

    TDynamicArray(std::initializer_list<ElementType> InitializerList)
        : LastIndex(-1)
    {
        ElementPointer = Memory::Allocate<ElementType>(InitializerList.size() * ElementSize());

        do
        {
            ++LastIndex;
            ConstructElement(LastIndex, InitializerList.begin()[LastIndex]);
        }
        while(LastIndex < static_cast<int64>(InitializerList.size() - 1));
    }

    constexpr ~TDynamicArray()
    {
        DestroyElements();

        Memory::Free(ElementPointer);
    }

    TDynamicArray& operator=(const TDynamicArray& Other)
    {
        ASSERT(this != &Other);

        DestroyElements();

        LastIndex = -1;
        ElementPointer = Memory::ReAllocate(ElementPointer, Other.UsedSize());

        InitializeFromOther(Other);

        return *this;
    }

    TDynamicArray& operator=(TDynamicArray&& Other)
    {
        ASSERT(this != &Other);

        Empty<true>();

        LastIndex = Other.LastIndex;
        ElementPointer = Other.ElementPointer;

        Other.ElementPointer = nullptr;
        Other.LastIndex = -1;

        return *this;
    }

    template<typename LeftElementType, typename RightElementType> requires(TypeTrait::AreLogicallyComparable<LeftElementType, RightElementType> || TypeTrait::AreBitwiseComparable<LeftElementType, RightElementType>)
    friend bool operator==(const TDynamicArray<LeftElementType>& Left, const TDynamicArray<RightElementType>& Right);

    template<typename LeftElementType, typename RightElementType>
    friend bool operator!=(const TDynamicArray<LeftElementType>& Left, const TDynamicArray<RightElementType>& Right);

    TConstIterator begin() const
    {
        return MakeIterator<TConstIterator>(ElementPointer);
    }

    TMutableIterator begin()
    {
        return MakeIterator<TMutableIterator>(ElementPointer);
    }

    TConstIterator end() const
    {
        return MakeIterator<TConstIterator>(&ElementPointer[Num()]);
    }

    TMutableIterator end()
    {
        return MakeIterator<TMutableIterator>(&ElementPointer[Num()]);
    }

    static constexpr int64 ElementSize()
    {
        return sizeof(ElementType);
    }

    ElementType* GetData()
    {
        return ElementPointer;
    }

    const ElementType* GetData() const
    {
        return ElementPointer;
    }

    ElementType* Data()
    {
        return ElementPointer;
    }

    const ElementType* Data() const
    {
        return ElementPointer;
    }

    ElementType* Back()
    {
        if EXPECT(LastIndex >= 0, true)
        {
            return &ElementPointer[LastIndex];
        }
        return nullptr;
    }

    const ElementType* Back() const
    {
        if EXPECT(LastIndex >= 0, true)
        {
            return &ElementPointer[LastIndex];
        }
        return nullptr;
    }

    ElementType* operator-(const int64 DecrementAmount)
    {
        return ElementPointer - DecrementAmount;
    }

    ElementType* operator+(const int64 DecrementAmount)
    {
        return ElementPointer + DecrementAmount;
    }

    const ElementType* operator-(const int64 DecrementAmount) const
    {
        return ElementPointer - DecrementAmount;
    }

    const ElementType* operator+(const int64 DecrementAmount) const
    {
        return ElementPointer + DecrementAmount;
    }

    ElementType& operator[](const int64 Index)
    {
        ASSERT(IsIndexValid(Index));
        return ElementPointer[Index];
    }

    const ElementType& operator[](const int64 Index) const
    {
        ASSERT(IsIndexValid(Index));
        return ElementPointer[Index];
    }

    bool IsEmpty() const
    {
        return LastIndex == -1;
    }

    template<typename RetType = int64>
    RetType Num() const
    {
        return LastIndex + 1;
    }

    int64 UsedSize() const
    {
        return Num() * ElementSize();
    }

    int64 AllocatedSize() const
    {
        return Memory::AllocatedSize(ElementPointer);
    }

    bool IsIndexValid(const int64 Index) const
    {
        return Index <= LastIndex && Index >= 0;
    }

    template<bool bDeallocate = false> requires(TypeTrait::IsDefaultConstructible<ElementType>)
    ElementType Pop()
    {
        ElementType* LastElement{Back()};
        ElementType PoppedElement{};

        if EXPECT(LastElement != nullptr, true)
        {
            PoppedElement = Move(*LastElement);

            if constexpr(!TypeTrait::IsTriviallyDestructible<ElementType>)
            {
                LastElement->ElementType::~ElementType();
            }

            if constexpr(bDeallocate)
            {
                DeallocateUnusedIfNeeded();
            }

            --LastIndex;
        }

        return PoppedElement;
    }

    template<typename... Ts>
    int64 Append(Ts&&... Args)
    {
        ReAllocateIfNeeded<true>();
        return ConstructElement(++LastIndex, MoveIfPossible(Args)...);
    }

    template<>
    int64 Append<const TDynamicArray&>(const TDynamicArray& Other)
    {
        ReAllocateIfNeeded<false>(Other.UsedSize());

        if constexpr(TypeTrait::IsTriviallyConstructible<ElementType>)
        {
            Memory::Copy(&ElementPointer[Num()], Other.ElementPointer, Other.UsedSize());
            LastIndex += Other.Num();
        }
        else
        {
            int64 Index{0};
            while(Index < Other.Num())
            {
                new(&ElementPointer[Index + Num()]) ElementType{Other[Index]};
                ++Index;
            }
            LastIndex = Index + LastIndex;
        }

        return LastIndex - Other.Num();
    }

    template<typename... Ts>
    int64 PushBack(Ts&&... Args)
    {
        PushBackMemory<true>(0);
        return ConstructElement(0, MoveIfPossible(Args)...);
    }

    template<typename... Ts>
    int64 InsertAtPushBack(const int64 TargetIndex LIFETIME_BOUND, Ts&&... Args)
    {
        PushBackMemory<true>(TargetIndex);
        return ConstructElement(TargetIndex, MoveIfPossible(Args)...);
    }

    template<typename... Ts>
    int64 InsertAtSwap(const int64 TargetIndex LIFETIME_BOUND, Ts&&... Args)
    {
        MoveElementToEnd<true>(TargetIndex);
        return ConstructElement(TargetIndex, MoveIfPossible(Args)...);
    }

    //returns the new index to the first element that has been moved
    template<bool bDeallocateMemory = false>
    int64 RemoveAtSwap(const int64 TargetIndex, const int64 NumToRemove = 1)
    {
        ASSERT(NumToRemove >= 0 && (TargetIndex + NumToRemove) <= Num());

        DestroyElements(TargetIndex, NumToRemove);

        const int64 NumAfterDeletionArea{Num() - (TargetIndex + NumToRemove)};
        const int64 NumToMoveIntoDeletionArea{Math::Min(NumToRemove, NumAfterDeletionArea)};
        const int64 FirstElementToMoveIndex{Num() - NumToMoveIntoDeletionArea};

        Memory::Copy(&ElementPointer[TargetIndex], &ElementPointer[FirstElementToMoveIndex], NumToMoveIntoDeletionArea * ElementSize());

        LastIndex -= NumToRemove;

        if constexpr(bDeallocateMemory)
        {
            DeallocateUnusedIfNeeded();
        }

        return FirstElementToMoveIndex;
    }

    //returns the new index to the first element that has been moved
    template<bool bDeallocateMemory = false>
    int64 RemoveAtCollapse(const int64 TargetIndex, const int64 NumToRemove = 1)
    {
        ASSERT(NumToRemove >= 0 && (TargetIndex + NumToRemove) <= Num());

        DestroyElements(TargetIndex, NumToRemove);

        const int64 NumBytesToMove{ElementSize() * (Num() - TargetIndex - NumToRemove)};
        const int64 FirstElementToMoveIndex{TargetIndex + NumToRemove};

        Memory::Move(&ElementPointer[TargetIndex], &ElementPointer[FirstElementToMoveIndex], NumBytesToMove);

        LastIndex -= NumToRemove;

        if constexpr(bDeallocateMemory)
        {
            DeallocateUnusedIfNeeded();
        }

        return FirstElementToMoveIndex;
    }

    void ReserveUndefined(const int64 NumElementsToReserve)
    {
        int64 NewSize{AllocatedSize() + (NumElementsToReserve * ElementSize())};
        ElementPointer = Memory::ReAllocate(ElementPointer, NewSize);
    }

    void ReserveZeroed(const int64 NumElementsToReserve)
    {
        ReserveUndefined(NumElementsToReserve);
        Memory::Set(&ElementPointer[LastIndex + 1], 0, AllocatedSize() - UsedSize());
    }

    //might not actually deallocate anything
    void DeallocateUnusedIfNeeded()
    {
        static constexpr bool bExpectToReAllocate{ElementSize() >= (ArrayConstants::UnusedSizeLimit<ElementType>() / 2)};

        const int64 UnusedSize{AllocatedSize() - UsedSize()};

        if EXPECT(UnusedSize >= ArrayConstants::UnusedSizeLimit<ElementType>(), bExpectToReAllocate)
        {
            ElementPointer = Memory::ReAllocate(ElementPointer, UsedSize());
        }
    }

    void DeallocateUnused()
    {
        if EXPECT(AllocatedSize() > UsedSize(), true)
        {
            ElementPointer = Memory::ReAllocate(ElementPointer, UsedSize());
        }
    }

    void ResizeTo(const int64 NewElementNum)
    {
        ASSERT(NewElementNum >= 0);

        const int64 NumDeallocatedElements{Num() - NewElementNum};

        DestroyElements(Num() - NumDeallocatedElements);

        LastIndex = NewElementNum - 1;
        ElementPointer = Memory::ReAllocate(ElementPointer, NewElementNum * ElementSize());
    }

    template<bool bDeallocateMemory = true>
    void Empty()
    {
        DestroyElements();

        LastIndex = -1;

        if constexpr(bDeallocateMemory)
        {
            Memory::Free(ElementPointer);

            ElementPointer = nullptr;
        }
    }

    template<typename ArrayType> requires(TypeTrait::IsStaticArray<TypeTrait::Pure<ArrayType>>::Value || TypeTrait::IsDynamicArray<TypeTrait::Pure<ArrayType>>::Value)
    void Overwrite(int64 StartIndex, ArrayType&& Replacement)
    {
        ASSERT(IsIndexValid(StartIndex));

        const int64 EndReplacementIndex{StartIndex + (Replacement.Num() - 1)};

        if EXPECT(EndReplacementIndex > LastIndex, false)
        {
            LastIndex = EndReplacementIndex;
            ElementPointer = Memory::ReAllocate(ElementPointer, LastIndex * ElementSize());
        }

        if constexpr(TypeTrait::IsTriviallyConstructible<ElementType>)
        {
            Memory::Copy(&ElementPointer[StartIndex], Replacement.GetData(), Replacement.UsedSize());
        }
        else
        {
            while(StartIndex <= EndReplacementIndex)
            {
                new(&ElementPointer[StartIndex]) ElementType{MoveIfPossible(Replacement[StartIndex])};
                ++StartIndex;
            }
        }
    }

    template<typename... Ts> requires(TypeTrait::PureEqual<ElementType, Ts> && ...)
    void Overwrite(int64 StartIndex, Ts&&... Replacements)
    {
        TStaticArray<ElementType, sizeof...(Replacements)> HoldingArray{Forward<Ts>(Replacements)...};
        Overwrite(StartIndex, Move(HoldingArray));
    }

    template<typename TargetElementType>
    NODISCARD TDynamicArray<TargetElementType> Convert() const &
    {
        TDynamicArray<TargetElementType> ResultArray{};
        ResultArray.ReserveUndefined(Num());

        while(ResultArray.LastIndex < LastIndex)
        {
            ResultArray.LastIndex += 1;
            ResultArray[ResultArray.LastIndex] = static_cast<TargetElementType>(ElementPointer[ResultArray.LastIndex]);
        }

        return ResultArray;
    }

    template<typename TargetElementType>
    NODISCARD TDynamicArray<TargetElementType> Convert() &&
    {
        TDynamicArray<TargetElementType> ResultArray{reinterpret_cast<TargetElementType*>(ElementPointer), LastIndex, sizeof(TargetElementType) * Num()};

        for(int64 Index{0}; Index < LastIndex; ++Index)
        {
            ResultArray[Index] = static_cast<TargetElementType&&>(ElementPointer[Index]);
        }

        ResultArray.ElementPointer = reinterpret_cast<TargetElementType*>(Memory::ReAllocate(ElementPointer, ResultArray.AllocatedSize()));

        ElementPointer = nullptr;
        LastIndex = -1;

        return ResultArray;
    }

private:

    using ConstElementType = TypeTrait::Conditional
    <
    TypeTrait::IsPointer<ElementType>,
    const TypeTrait::RemovePointer<TypeTrait::RemoveCV<ElementType>>* const,
    const TypeTrait::RemoveCV<ElementType>&
    >;

public:

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"

    int64 FindIndex(ConstElementType ElementToFind) const requires(TypeTrait::AreLogicallyComparable<ElementType, ElementType>)
    {
        for(int64 Index{0}; Index < Num(); ++Index)
        {
            if(ElementToFind == ElementPointer[Index])
            {
                return Index;
            }
        }
        return INDEX_NONE;
    }

    ElementType* FindPointer(ConstElementType ElementToFind) const requires(TypeTrait::AreLogicallyComparable<ElementType, ElementType>)
    {
        for(int64 Index{0}; Index < Num(); ++Index)
        {
            if(ElementToFind == ElementPointer[Index])
            {
                return &ElementPointer[Index];
            }
        }
        return nullptr;
    }

    TTuple<int64, ElementType*> Find(ConstElementType ElementToFind) const requires(TypeTrait::AreLogicallyComparable<ElementType, ElementType>)
    {
        for(int64 Index{0}; Index < Num(); ++Index)
        {
            if(ElementToFind == ElementPointer[Index])
            {
                return {Index, &ElementPointer[Index]};
            }
        }
        return {INDEX_NONE, nullptr};
    }

#pragma clang diagnostic pop
#pragma clang diagnostic pop

    INLINE static int64 TotalBytesToAllocate(const int64 BytesToAllocate)
    {
        constexpr auto EvaluateExtra = []() consteval -> int64
        {
            const int64 Min{Math::Min(ArrayConstants::ReserveExtraValue<ElementType>(), ArrayConstants::ReserveSizeLimit<ElementType>())};
            const int64 Max{Math::Max(Min - TDynamicArray::ElementSize(), 0LL)};
            return Max;
        };

        return EvaluateExtra() + BytesToAllocate;
    }

private:

    void InitializeFromOther(const TDynamicArray& Other)
    {
        if constexpr(TypeTrait::IsTriviallyConstructible<ElementType>)
        {
            LastIndex = Other.LastIndex;

            Memory::Copy(ElementPointer, Other.ElementPointer, UsedSize());
        }
        else
        {
            while(LastIndex < Other.LastIndex)
            {
                ++LastIndex;
                ConstructElement(LastIndex, Other[LastIndex]);
            }
        }
    }

    void DestroyElements(int64 StartIndex = 0)
    {
        DestroyElements(StartIndex, Num());
    }

    void DestroyElements(int64 StartIndex, int64 NumToDestroy)
    {
        if constexpr(!TypeTrait::IsTriviallyDestructible<ElementType>)
        {
            NumToDestroy = StartIndex + NumToDestroy;

            while(StartIndex < NumToDestroy)
            {
                ElementPointer[StartIndex].ElementType::~ElementType();
                ++StartIndex;
            }
        }
    }

    template<bool bExtra>
    void ReAllocateIfNeeded(const int64 BytesToAllocate = ElementSize())
    {
        int64 NewSize;

        if constexpr(bExtra)
        {
            NewSize = TotalBytesToAllocate(BytesToAllocate) + AllocatedSize();
        }
        else
        {
            NewSize = BytesToAllocate + AllocatedSize();
        }

        static constexpr bool bExpectToReallocate{ElementSize() >= (ArrayConstants::ReserveSizeLimit<ElementType>() / 2)};

        if EXPECT((BytesToAllocate + UsedSize()) > AllocatedSize(), bExpectToReallocate)
        {
            ElementPointer = Memory::ReAllocate(ElementPointer, NewSize);
        }
    }

    template<typename... Ts>
    int64 ConstructElement(const int64 Location LIFETIME_BOUND, Ts&&... Args)
    {
        ASSERT(IsIndexValid(Location));

        new(&ElementPointer[Location]) ElementType{MoveIfPossible(Args)...};

        return Location;
    }

    template<typename InElementType = ElementType> requires(TypeTrait::PureEqual<InElementType, ElementType>)
    int64 ConstructElement(const int64 Location LIFETIME_BOUND, InElementType&& Element)
    {
        ASSERT(IsIndexValid(Location));

        if constexpr(TypeTrait::IsTriviallyConstructible<ElementType>)
        {
            Memory::Copy(&ElementPointer[Location], &Element, ElementSize());
        }
        else
        {
            new(&ElementPointer[Location]) ElementType{MoveIfPossible(Element)};
        }

        return Location;
    }

    template<bool bExtra>
    void PushBackMemory(const int64 FromIndex, const int64 NumPlacesToMove = 1)
    {
        ASSERT(IsIndexValid(FromIndex) || LastIndex == -1);

        ReAllocateIfNeeded<bExtra>(ElementSize() * NumPlacesToMove);

        const int64 NumBytesToMove{ElementSize() * (Num() - FromIndex)};

        Memory::Move(&ElementPointer[FromIndex + NumPlacesToMove], &ElementPointer[FromIndex], NumBytesToMove);

        LastIndex += NumPlacesToMove;
    }

    template<bool bExtra>
    void MoveElementToEnd(const int64 TargetIndex)
    {
        ReAllocateIfNeeded<bExtra>();
        ++LastIndex;

        if EXPECT(LastIndex > TargetIndex, true)
        {
            ASSERT(IsIndexValid(TargetIndex));
            Memory::Move(&ElementPointer[LastIndex], &ElementPointer[TargetIndex], ElementSize());
        }
    }

private:

    ElementType* ElementPointer;

    int64 LastIndex;

};

template<typename LeftElementType, typename RightElementType> requires(TypeTrait::AreLogicallyComparable<LeftElementType, RightElementType> || TypeTrait::AreBitwiseComparable < LeftElementType, RightElementType >)
bool operator==(const TDynamicArray<LeftElementType>& Left, const TDynamicArray<RightElementType>& Right)
{
    if EXPECT(Left.Num() == Right.Num(), false)
    {
        if constexpr(TypeTrait::AreBitwiseComparable<LeftElementType, RightElementType>)
        {
            return Memory::Compare(Left.ElementPointer, Right.ElementPointer, Left.Num()) == 0;
        }
        else if constexpr(TypeTrait::AreLogicallyComparable<LeftElementType, RightElementType>)
        {
            for(int64 Index{0}; Index < Left.Num(); ++Index)
            {
                if EXPECT(Left[Index] != Right[Index], false)
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"

template<typename LeftElementType, typename RightElementType>
bool operator!=(const TDynamicArray<LeftElementType>& Left, const TDynamicArray<RightElementType>& Right)
{
    return !(Left == Right);
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop

namespace ArrUtil
{
    template<typename T, int64... I>
    inline constexpr TStaticArray<T, (I + ...)> JoinArrays(const TStaticArray<T, I>&... Arrays)
    {
        TStaticArray<T, (I + ...)> ResultArray{};
        T* ResultBegin{ResultArray.Data()};

        ((Memory::Copy(ResultBegin, Arrays.Data(), I * sizeof(T)), ResultBegin += I), ...);

        return ResultArray;
    }
}

namespace fmt
{
    template<int64 Num>
    struct formatter<TStaticArray<char8, Num>> : public formatter<const char8*>
    {
        template<typename FormatContext>
        auto format(const TStaticArray<char8, Num>& p, FormatContext& ctx)
        {
            return formatter<const char8*>::format(p.GetData(), ctx);
        }
    };

    template<int64 Num>
    struct formatter<TStaticArray<const char8, Num>> : public formatter<const char8*>
    {
        template<typename FormatContext>
        auto format(const TStaticArray<char8, Num>& p, FormatContext& ctx)
        {
            return formatter<const char8*>::format(p.GetData(), ctx);
        }
    };

    template<>
    struct formatter<TDynamicArray<char8>> : public formatter<const char8*>
    {
        template<typename FormatContext>
        auto format(const TDynamicArray<char8>& p, FormatContext& ctx)
        {
            return formatter<const char8*>::format(p.GetData(), ctx);
        }
    };

    template<>
    struct formatter<TDynamicArray<const char8>> : public formatter<const char8*>
    {
        template<typename FormatContext>
        auto format(const TDynamicArray<char8>& p, FormatContext& ctx)
        {
            return formatter<const char8*>::format(p.GetData(), ctx);
        }
    };
}
