#pragma once

#include "Simd.hpp"
#include "Iterator.hpp"
#include "Math.hpp"

#ifndef NULL_CHAR
#define NULL_CHAR static_cast<const char8>('\0')
#endif

template<typename, int64>
class TStaticArray;

class FStaticString;

class FQuaternion;
class FVector;
class FColor;

enum EStackSize : uint32
{
    SS0 = 0,
    SS28 = 28,
    SS60 = 60,
    SS124 = 124,
    SS252 = 252,
    SS508 = 508
};

enum class EActiveArray : bool
{
    Stack = 0,
    Heap = 1,
};

/*
 * this string will store up to InStackSize characters on the stack
 * if the characters exceed InStackSize in size, it will switch over to heap allocation
 * not trivially copyable
 * now with constexpr, yay!
 * todo clion does not like the PACKED, remember to uncomment it when done working with the class
 */
template<EStackSize InStackSize = SS60>
class PACKED alignas(8) FString final
{
private:

    friend class FString<SS0>;
    friend class FString<SS28>;
    friend class FString<SS60>;
    friend class FString<SS124>;
    friend class FString<SS252>;
    friend class FString<SS508>;

    using TConstIterator = TRangedIterator<const char8>;
    using TMutableIterator = TRangedIterator<char8>;

public:

    //how much to reserve extra if we need to reallocate
    inline static constexpr uint32 ReserveSize{32};

    //the number of characters that can fit in the stack
    inline static constexpr uint32 StackSize{static_cast<uint32>(InStackSize)};

private:

    union PACKED UCharacterArray
    {
        /*
        constexpr UCharacterArray() requires(InStackSize > SS0)
            : Stack{}
        {
            Stack[0] = NULL_CHAR;
        }

        constexpr UCharacterArray() requires(InStackSize == SS0)
            : Heap{nullptr}
        {
        }
        */ //cant do this becouse clang has a bug

        //do this instead
        constexpr UCharacterArray()
        {
            if constexpr(InStackSize > SS0)
            {
                Stack[0] = NULL_CHAR;
            }
            else
            {
                Heap = nullptr;
            }
        }

        constexpr UCharacterArray(const FString& Other)
            : Stack{}
        {
            if EXPECT(Other.Num() <= StackSize, StackSize > SS0)
            {
                Memory::Copy(Stack, Other.Data(), Other.Num());
            }
            else
            {
                Heap = Memory::Allocate<char8>(Other.Num());
                Memory::Copy(Heap, Other.Data(), Other.Num());
            }
        }

        constexpr UCharacterArray(FString&& Other)
            : Stack{}
        {
            if EXPECT(Other.Num() <= StackSize, StackSize > SS0)
            {
                Memory::Copy(Stack, Other.Data(), Other.Num());
            }
            else
            {
                Heap = Other.CharacterArray.Heap;
                Other.TerminatorIndex = 0;
            }
        }

        template<EStackSize OtherSS>
        constexpr UCharacterArray(const FString<OtherSS>& Other)
            : Stack{}
        {
            if EXPECT(Other.Num() <= StackSize, OtherSS <= InStackSize)
            {
                Memory::Copy(Stack, Other.Data(), Other.Num());
            }
            else
            {
                Heap = Memory::Allocate<char8>(Other.Num());
                Memory::Copy(Heap, Other.Data(), Other.Num());
            }
        }

        template<EStackSize OtherSS>
        constexpr UCharacterArray(FString<OtherSS>&& Other)
            : Stack{}
        {
            if EXPECT(Other.Num() <= InStackSize, OtherSS <= InStackSize)
            {
                Memory::Copy(Stack, Other.Data(), Other.Num());
            }
            else
            {
                Heap = Other.CharacterArray.Heap;
                Other.TerminatorIndex = 0;
            }
        }

        explicit constexpr UCharacterArray(uint32 NumChars)
        {
            if EXPECT(NumChars <= StackSize, StackSize > SS0)
            {
                Stack[NumChars - 1] = NULL_CHAR;
            }
            else
            {
                Heap = Memory::Allocate<char8>(NumChars);
                Heap[NumChars - 1] = NULL_CHAR;
            }
        }

        template<uint64 NumChars> requires(NumChars <= StackSize)
        implicit constexpr UCharacterArray(const char8(&String)[NumChars])
            : Stack{}
        {
            Memory::Copy(Stack, String, NumChars);
        }

        template<uint64 NumChars> requires(NumChars > StackSize)
        implicit UCharacterArray(const char8(&String)[NumChars])
            : Heap{}
        {
            Heap = Memory::Allocate<char8>(NumChars);
            Memory::Copy(Heap, String, NumChars);
        }

        explicit constexpr UCharacterArray(const char8* String, uint32 NumChars)
        {
            if EXPECT(NumChars <= StackSize, StackSize > SS0)
            {
                Memory::Copy(Stack, String, NumChars);
            }
            else
            {
                Heap = Memory::Allocate<char8>(NumChars);
                Memory::Copy(Heap, String, NumChars);
            }
        }

        char8* Heap;
        char8 Stack[StackSize];
    };

public:

    constexpr explicit FString(ENoInit)
    {
    }

    constexpr FString()
        : CharacterArray{}
        , TerminatorIndex{0}
    {
    }

    explicit constexpr FString(uint32 NumChars)
        : CharacterArray{NumChars}
        , TerminatorIndex{NumChars - 1}
    {
    }

    template<uint64 NumChars>
    implicit constexpr FString(const char8(&String)[NumChars])
        : CharacterArray{String}
        , TerminatorIndex{NumChars - 1}
    {
    }

    explicit constexpr FString(const char8* String, uint32 NumChars)
        : CharacterArray{String, NumChars}
        , TerminatorIndex{NumChars - 1}
    {
    }

    constexpr FString(const FString& Other)
        : CharacterArray{Other}
        , TerminatorIndex{Other.TerminatorIndex}
    {
    }

    constexpr FString(FString&& Other)
        : CharacterArray{Move(Other)}
        , TerminatorIndex{Other.TerminatorIndex}
    {
    }

    template<EStackSize OtherSS>
    constexpr FString(const FString<OtherSS>& Other)
        : CharacterArray{Other}
        , TerminatorIndex{Other.TerminatorIndex}
    {
    }

    template<EStackSize OtherSS>
    constexpr FString(FString<OtherSS>&& Other)
        : CharacterArray{Move(Other)}
        , TerminatorIndex{Other.TerminatorIndex}
    {
    }

    constexpr ~FString()
    {
        if EXPECT(ActiveArray() == EActiveArray::Heap, StackSize == SS0)
        {
            Memory::Free(CharacterArray.Heap);
        }
    }

    template<uint64 NumChars>
    INLINE FString& operator=(const char8 (&String)[NumChars])
    {
        if constexpr(NumChars > StackSize)
        {
            return Assign_Heap(String, NumChars);
        }
        else if constexpr(NumChars <= StackSize)
        {
            return Assign_Stack(String, NumChars);
        }
    }

    FString& operator=(const FString& Other);

    FString& operator=(FString&& Other);

    template<EStackSize OtherSS>
    FString& operator=(const FString<OtherSS>& Other);

    template<EStackSize OtherSS>
    FString& operator=(FString<OtherSS>&& Other);

private:

    FString& Assign_Stack(const char8* String, uint32 NumChars);

    FString& Assign_Heap(const char8* String, uint32 NumChars);

public:

    template<uint64 NumChars>
    INLINE FString& operator+=(const char8 (&String)[NumChars])
    {
        return Concat_Assign(String, NumChars);
    }

    template<uint64 NumChars>
    INLINE FString& Concat_Assign(const char8 (&String)[NumChars])
    {
        return Concat_Assign(String, NumChars);
    }

    template<uint64 NumChars>
    INLINE friend FString& operator+=(const char8 (&String)[NumChars], const FString& Other)
    {
        return Other.PushBack_Assign(String, NumChars);
    }

    template<EStackSize OtherSS>
    INLINE FString& operator+=(const FString<OtherSS>& Other)
    {
        return Concat_Assign(Other.RawString(), Other.Num());
    }

    template<EStackSize OtherSS>
    INLINE FString& Concat_Assign(const FString<OtherSS>& Other)
    {
        return Concat_Assign(Other.RawString(), Other.Num());
    }

    INLINE FString& operator+=(const FStaticString& Other)
    {
        return Concat_Assign(Other);
    }

    FString& Concat_Assign(const FStaticString& String);

    template<uint64 NumChars>
    INLINE FString operator+(const char8 (&String)[NumChars]) const
    {
        return Concat_New(String, NumChars);
    }

    template<uint64 NumChars>
    INLINE FString Concat_New(const char8 (&String)[NumChars]) const
    {
        return Concat_New(String, NumChars);
    }

    template<uint64 NumChars>
    INLINE friend FString operator+(const char8 (&String)[NumChars], const FString& Other)
    {
        return Other.PushBack_New(String, NumChars);
    }

    template<EStackSize OtherSS>
    INLINE FString operator+(const FString<OtherSS>& Other) const
    {
        return Concat_New(Other.RawString(), Other.Num());
    }

    template<EStackSize OtherSS>
    INLINE FString Concat_New(const FString<OtherSS>& Other) const
    {
        return Concat_New(Other.RawString(), Other.Num());
    }

    INLINE FString operator+(const FStaticString& Other) const
    {
        return Concat_New(Other);
    }

    FString Concat_New(const FStaticString& String) const;

    FString& Concat_Assign(const char8* String, uint32 NumChars);

    FString Concat_New(const char8* String, uint32 NumChars) const;

public:

    template<uint64 NumChars>
    INLINE FString& operator<<=(const char8 (&String)[NumChars])
    {
        return PushBack_Assign(String, NumChars);
    }

    template<uint64 NumChars>
    INLINE FString& PushBack_Assign(const char8 (&String)[NumChars])
    {
        return PushBack_Assign(String, NumChars);
    }

    template<EStackSize OtherSS>
    INLINE FString& operator<<=(const FString<OtherSS>& Other)
    {
        return PushBack_Assign(Other.RawString(), Other.Num());
    }

    template<EStackSize OtherSS>
    INLINE FString& PushBack_Assign(const FString<OtherSS>& Other)
    {
        return PushBack_Assign(Other.RawString(), Other.Num());
    }

    template<uint64 NumChars>
    INLINE FString operator<<(const char8 (&String)[NumChars]) const
    {
        return PushBack_New(String, NumChars);
    }

    template<uint64 NumChars>
    INLINE FString PushBack_New(const char8 (&String)[NumChars]) const
    {
        return PushBack_New(String, NumChars);
    }

    template<EStackSize OtherSS>
    INLINE FString operator<<(const FString<OtherSS>& Other) const
    {
        return PushBack_New(Other.RawString(), Other.Num());
    }

    template<EStackSize OtherSS>
    INLINE FString PushBack_New(const FString<OtherSS>& Other) const
    {
        return PushBack_New(Other.RawString(), Other.Num());
    }

    FString& PushBack_Assign(const char8* String, uint32 NumChars);

    FString PushBack_New(const char8* String, uint32 NumChars) const;

public:

    //removes all characters from begin (inclusive) to end (exclusive) and collapses the string
    FString& Erase_Assign(uint32 Begin, uint32 End);

    //removes all characters from begin (inclusive) to end (exclusive) and collapses the string
    FString Erase_New(uint32 Begin, uint32 End) const;

public:

    template<uint64 NumChars>
    INLINE FString& Replace_Assign(uint32 Begin, const char8 (&String)[NumChars])
    {
        return Replace_Assign(Begin, String, NumChars);
    }

    template<EStackSize OtherSS>
    INLINE FString& Replace_Assign(uint32 Begin, const FString<OtherSS>& Other)
    {
        return Replace_Assign(Begin, Other.RawString(), Other.Num());
    }

    template<uint64 NumChars>
    INLINE FString Replace_New(uint32 Begin, const char8 (&String)[NumChars]) const
    {
        return Replace_New(Begin, String, NumChars);
    }

    template<EStackSize OtherSS>
    INLINE FString Replace_New(uint32 Begin, const FString<OtherSS>& Other) const
    {
        return Replace_New(Begin, Other.RawString(), Other.Num());
    }

    FString& Replace_Assign(uint32 Begin, const char8* String, uint32 NumChars);

    FString Replace_New(uint32 Begin, const char8* String, uint32 NumChars) const;

public:

    template<uint64 NumChars>
    INLINE FString& Insert_Assign(uint32 Begin, const char8 (&String)[NumChars])
    {
        return Insert_Assign(Begin, String, NumChars);
    }

    template<EStackSize OtherSS>
    INLINE FString& Insert_Assign(uint32 Begin, const FString<OtherSS>& Other)
    {
        return Insert_Assign(Begin, Other.RawString(), Other.Num());
    }

    template<uint64 NumChars>
    INLINE FString Insert_New(uint32 Begin, const char8 (&String)[NumChars]) const
    {
        return Insert_New(Begin, String, NumChars);
    }

    template<EStackSize OtherSS>
    INLINE FString Insert_New(uint32 Begin, const FString<OtherSS>& Other) const
    {
        return Insert_New(Begin, Other.RawString(), Other.Num());
    }

    FString& Insert_Assign(uint32 Begin, const char8* String, uint32 NumChars);

    FString Insert_New(uint32 Begin, const char8* String, uint32 NumChars) const;

public:

    template<uint64 NumChars>
    INLINE char8* FindSubstring(const char8 (&String)[NumChars])
    {
        return FindSubstring(String, NumChars);
    }

    template<uint64 NumChars>
    INLINE const char8* FindSubstring(const char8 (&String)[NumChars]) const
    {
        return FindSubstring(String, NumChars);
    }

    template<EStackSize OtherSS>
    INLINE char8* FindSubstring(const FString<OtherSS>& Other)
    {
        return FindSubstring(Other.RawString(), Other.Num());
    }

    template<EStackSize OtherSS>
    INLINE const char8* FindSubstring(const FString<OtherSS>& Other) const
    {
        return FindSubstring(Other.RawString(), Other.Num());
    }

    char8* FindSubstring(const char8* String, uint32 NumChars);

    const char8* FindSubstring(const char8* String, uint32 NumChars) const;

public:

    template<uint64 NumChars>
    INLINE bool operator==(const char8 (&String)[NumChars]) const
    {
        return CompareEqual(String, NumChars);
    }

    template<EStackSize OtherSS>
    bool operator==(const FString<OtherSS>& Other) const
    {
        return CompareEqual(Other.Data(), Other.Num());
    }

    template<uint64 NumChars>
    INLINE bool operator!=(const char8 (&String)[NumChars]) const
    {
        return !CompareEqual(String, NumChars);
    }

    template<EStackSize OtherSS>
    INLINE bool operator!=(const FString<OtherSS>& Other) const
    {
        return !CompareEqual(Other.Data(), Other.Num());
    }

    INLINE bool CompareEqual(const char8* String, uint32 NumChars) const
    {
        if EXPECT(Num() == NumChars, false)
        {
            return Memory::Compare(Data(), String, Num()) == 0;
        }
        return false;
    }

public:

    void SetTerminatorIndex(uint32 NewIndex);

    void Empty();

    constexpr bool IsEmpty() const
    {
        return TerminatorIndex == 0;
    }

    INLINE constexpr EActiveArray ActiveArray() const
    {
        return static_cast<EActiveArray>(Num() > StackSize);
    }

    INLINE constexpr char8 operator[](uint32 Index) const
    {
        ASSERT(Index < Num());
        return RawString()[Index];
    }

    INLINE constexpr char8& operator[](uint32 Index)
    {
        ASSERT(Index < Num());
        return RawString()[Index];
    }

    //returns the size of this string including the null terminator
    INLINE constexpr uint32 Num() const
    {
        return TerminatorIndex + 1;
    }

    //returns the size of this string including the null terminator
    INLINE constexpr uint32 Length() const
    {
        return TerminatorIndex + 1;
    }

    INLINE constexpr const char8* RawString() const
    {
        if(ActiveArray() == EActiveArray::Stack)
        {
            return CharacterArray.Stack;
        }
        else
        {
            return CharacterArray.Heap;
        }
    }

    INLINE constexpr char8* RawString()
    {
        if(ActiveArray() == EActiveArray::Stack)
        {
            return CharacterArray.Stack;
        }
        else
        {
            return CharacterArray.Heap;
        }
    }

    INLINE constexpr const char8* Data() const
    {
        return RawString();
    }

    INLINE constexpr char8* Data()
    {
        return RawString();
    }

    INLINE constexpr char8* Start()
    {
        if(ActiveArray() == EActiveArray::Stack)
        {
            return CharacterArray.Stack;
        }
        else
        {
            return CharacterArray.Heap;
        }
    }

    INLINE constexpr const char8* Start() const
    {
        if(ActiveArray() == EActiveArray::Stack)
        {
            return CharacterArray.Stack;
        }
        else
        {
            return CharacterArray.Heap;
        }
    }

    INLINE constexpr char8* End()
    {
        if(ActiveArray() == EActiveArray::Stack)
        {
            return CharacterArray.Stack + TerminatorIndex;
        }
        else
        {
            return CharacterArray.Heap + TerminatorIndex;
        }
    }

    INLINE constexpr const char8* End() const
    {
        if(ActiveArray() == EActiveArray::Stack)
        {
            return CharacterArray.Stack + TerminatorIndex;
        }
        else
        {
            return CharacterArray.Heap + TerminatorIndex;
        }
    }

    INLINE constexpr TConstIterator begin() const
    {
        return TConstIterator{RawString()};
    }

    INLINE constexpr TMutableIterator begin()
    {
        return TMutableIterator{RawString()};
    }

    INLINE constexpr TConstIterator end() const
    {
        return TConstIterator{&RawString()[TerminatorIndex]};
    }

    INLINE constexpr TMutableIterator end()
    {
        return TMutableIterator{&RawString()[TerminatorIndex]};
    }

private:

    UCharacterArray CharacterArray;
    uint32 TerminatorIndex;
};

static_assert(sizeof(FString<SS28>) == 32);
static_assert(sizeof(FString<SS60>) == 64);
static_assert(sizeof(FString<SS124>) == 128);
static_assert(sizeof(FString<SS252>) == 256);
static_assert(sizeof(FString<SS508>) == 512);

namespace StrPri
{
    template<uint64 NumChars>
    inline consteval EStackSize LeastNeededStackSize()
    {
        if constexpr(NumChars <= SS28)
        {
            return SS28;
        }
        else if constexpr(NumChars <= SS60)
        {
            return SS60;
        }
        else if constexpr(NumChars <= SS124)
        {
            return SS124;
        }
        else if constexpr(NumChars <= SS252)
        {
            return SS252;
        }
        else if constexpr(NumChars <= SS508)
        {
            return SS508;
        }
        else
        {
            return SS0;
        }
    }
}

template<uint64 NumChars>
FString(const char8(&)[NumChars])->FString<StrPri::LeastNeededStackSize<NumChars>()>;

using FString0 = FString<SS0>;
using FString28 = FString<SS28>;
using FString60 = FString<SS60>;
using FString124 = FString<SS124>;
using FString252 = FString<SS252>;
using FString508 = FString<SS508>;

/*
 * a string that fits 32 characters built using SIMD instructions
 * trivially copyable, no need to pass by reference
 * now with constexpr, yay!
 */
class alignas(32) FStaticString final
{
private:

    using TConstIterator = TRangedIterator<const char8>;
    using TMutableIterator = TRangedIterator<char8>;

public:

    inline static constexpr int32 ComparisonMask{Simd::Mask<char8_32>()};

    inline static constexpr int32 MaxNumCharacters{Simd::NumElements<char8_32>()};

    explicit constexpr FStaticString(ENoInit)
    {
    }

    constexpr FStaticString()
        : Characters{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    {
    }

    explicit constexpr FStaticString(char8_32 String)
        : Characters(String)
    {
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"

#define Val(Index) Index >= NumChars ? NULL_CHAR : String[Index]

    template<uint64 NumChars> requires((NumChars < MaxNumCharacters))
    constexpr FStaticString(const char8 (&String)[NumChars])
        : Characters
        {
            __builtin_is_constant_evaluated() ?
            Val(0), Val(1), Val(2), Val(3), Val(4), Val(5), Val(6), Val(7),
            Val(8), Val(9), Val(10), Val(11), Val(12), Val(13), Val(14), Val(15),
            Val(16), Val(17), Val(18), Val(19), Val(20), Val(21), Val(22), Val(23),
            Val(24), Val(25), Val(26), Val(27), Val(28), Val(29), Val(30), Val(31)
            : Simd::LoadUnaligned<char8_32>(String)
        }
    {
    }

#undef Val

#pragma clang diagnostic pop

    INLINE FStaticString(const FStaticString& Other)
        : Characters{Other.Characters}
    {
    }

    INLINE FStaticString(FStaticString&& Other)
        : Characters{Move(Other.Characters)}
    {
    }

    constexpr ~FStaticString() = default;

    INLINE FStaticString& operator=(char8_32 String)
    {
        Characters = String;
        return *this;
    }

    INLINE FStaticString& operator=(const FStaticString& Other)
    {
        Characters = Other.Characters;
        return *this;
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE FStaticString& operator=(const char8 (&String)[NumChars])
    {
        Characters = Simd::LoadUnaligned<char8_32>(String);
        return *this;
    }

    INLINE FStaticString& operator+=(FStaticString Other)
    {
        return Concat_Assign(Other);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE FStaticString& operator+=(const char8 (&String)[NumChars])
    {
        return Concat_Assign(String, NumChars);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE FStaticString& Concat_Assign(const char8 (&String)[NumChars])
    {
        return Concat_Assign(String, NumChars);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE friend FStaticString& operator+=(const char8 (&String)[NumChars], FStaticString& Other)
    {
        return Other.PushBack_Assign(String, NumChars);
    }

    FStaticString& Concat_Assign(FStaticString Other);

private:

    FStaticString& Concat_Assign(const char8* RESTRICT String, uint32 NumChars);

public:

    INLINE FStaticString operator+(FStaticString Other) const
    {
        return Concat_New(Other);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE FStaticString operator+(const char8 (&String)[NumChars]) const
    {
        return Concat_New(String, NumChars);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE FStaticString Concat_New(const char8 (&String)[NumChars]) const
    {
        return Concat_New(String, NumChars);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE friend FStaticString operator+(const char8 (&String)[NumChars], const FStaticString Other)
    {
        return Other.PushBack_New(String, NumChars);
    }

    FStaticString Concat_New(FStaticString Other) const;

private:

    FStaticString Concat_New(const char8* RESTRICT String, uint32 NumChars) const;

public:

    INLINE FStaticString& operator<<=(FStaticString Other)
    {
        return PushBack_Assign(Other);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE FStaticString& operator<<=(const char8 (&String)[NumChars])
    {
        return PushBack_Assign(String, NumChars);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE FStaticString& PushBack_Assign(const char8 (&String)[NumChars])
    {
        return PushBack_Assign(String, NumChars);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE friend FStaticString& operator<<=(const char8 (&String)[NumChars], FStaticString& Other)
    {
        return Other.Concat_Assign(String, NumChars);
    }

    FStaticString& PushBack_Assign(FStaticString Other);

private:

    FStaticString& PushBack_Assign(const char8* RESTRICT String, uint32 NumChars);

public:

    INLINE FStaticString operator<<(FStaticString Other) const
    {
        return PushBack_New(Other);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE FStaticString operator<<(const char8 (&String)[NumChars]) const
    {
        return PushBack_New(String, NumChars);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE FStaticString PushBack_New(const char8 (&String)[NumChars]) const
    {
        return PushBack_New(String, NumChars);
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    INLINE friend FStaticString operator<<(const char8 (&String)[NumChars], const FStaticString Other)
    {
        return Other.Concat_New(String, NumChars);
    }

    FStaticString PushBack_New(FStaticString Other) const;

private:

    FStaticString PushBack_New(const char8* RESTRICT String, uint32 NumChars) const;

public:

    //removes all characters from begin (inclusive) to end (exclusive) and collapses the string
    FStaticString& Erase_Assign(uint32 Begin, uint32 End);

    //removes all characters from begin (inclusive) to end (exclusive) and collapses the string
    FStaticString Erase_New(uint32 Begin, uint32 End) const;

public:

    constexpr bool operator==(const FStaticString Other) const
    {
        return MakeMask(Other) == ComparisonMask;
    }

    constexpr bool operator!=(const FStaticString Other) const
    {
        return MakeMask(Other) != ComparisonMask;
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    constexpr bool operator==(const char8 (&String)[NumChars]) const
    {
        return MakeMask(String) == ComparisonMask;
    }

    template<uint64 NumChars> requires(NumChars <= MaxNumCharacters)
    constexpr bool operator!=(const char8 (&String)[NumChars]) const
    {
        return MakeMask(String) != ComparisonMask;
    }

private:

    INLINE constexpr int32 MakeMask(const FStaticString Other) const
    {
        return Simd::MoveMask(Characters == Other.Characters);
    }

    template<uint64 NumChars>
    constexpr int32 MakeMask(const char8 (&String)[NumChars]) const
    {
        const char8_32 StringVector{Simd::LoadUnaligned<char8_32>(String)};
        const int32 Mask{ComparisonMask >> (32 - NumChars)};
        return Mask & Simd::MoveMask(Characters == StringVector);
    }

    static char8_32 CombineStrings(const char8_32 Lower, const char8_32 Upper, uint32 LowEnd);

public:

    INLINE char8 operator[](const int32 Index) const
    {
        return Characters[Index];
    }

    INLINE char8& operator[](const int32 Index)
    {
        return RawString()[Index];
    }

    INLINE const char8* RawString() const
    {
        return Simd::ToPtr(&Characters);
    }

    INLINE char8* RawString()
    {
        return Simd::ToPtr(&Characters);
    }

    INLINE const char8* Data() const
    {
        return Simd::ToPtr(&Characters);
    }

    INLINE char8* Data()
    {
        return Simd::ToPtr(&Characters);
    }

    INLINE constexpr uint32 Length() const
    {
        const int32 BitMask{Simd::MoveMask(Characters == NULL_CHAR)};
        return Math::FindFirstSet(BitMask);
    }

    INLINE constexpr uint32 Num() const
    {
        const int32 BitMask{Simd::MoveMask(Characters == NULL_CHAR)};
        return Math::FindFirstSet(BitMask);
    }

    INLINE constexpr TConstIterator begin() const
    {
        return TConstIterator{RawString()};
    }

    INLINE constexpr TMutableIterator begin()
    {
        return TMutableIterator{RawString()};
    }

    INLINE constexpr TConstIterator end() const
    {
        return TConstIterator{&RawString()[Num() - 1]};
    }

    INLINE constexpr TMutableIterator end()
    {
        return TMutableIterator{&RawString()[Num() - 1]};
    }

    char8_32 Characters; //public becouse constexpr
};

static_assert(alignof(FStaticString) == 32);

namespace StrUtl
{
    FString<SS124> GetWorkingDirectory();

    template<typename Integer> requires(TypeTrait::IsInteger<Integer>)
    CONST void StepBackDirectories(char8* const Path, Integer& NumChars, uint32 NumDirectoriesToBack);

    /// @returns the number of characters in a string including the null terminator
    CONST int64 Length(const char8* String);

    template<typename ValueType> requires(TypeTrait::IsInteger<ValueType>)
    CONST extern TStaticArray<char8, (sizeof(ValueType) * 2) + 1> IntToHex(ValueType Value);

    CONST char8* ToUpperCase(char8* String LIFETIME_BOUND, int64 Length);
    CONST char8* ToLowerCase(char8* String LIFETIME_BOUND, int64 Length);

    CONST char8* FindSubString(char8* SourceString, uint32 NumSourceChars, const char8* SubString, uint32 NumSubChars);
    CONST const char8* FindSubString(const char8* SourceString, uint32 NumSourceChars, const char8* SubString, uint32 NumSubChars);

    template<typename TargetType> requires(TypeTrait::IsInteger<TargetType>)
    extern TargetType ToValue(const char8* Begin, const char8* End);

    template<typename TargetType> requires(TypeTrait::IsFloatingPoint<TargetType>)
    extern TargetType ToValue(const char8* Begin, const char8* End);

    extern FString<SS124> ToString(FQuaternion);
    extern FString<SS124> ToString(FVector);
    extern FString<SS124> ToString(FColor);
}

namespace fmt
{
    template<EStackSize InStackSize>
    struct formatter<FString<InStackSize>> : formatter<const char8*>
    {
        template<typename FormatContext>
        auto format(const FString<InStackSize>& p, FormatContext& ctx)
        {
            return formatter<const char8*>::format(p.RawString(), ctx);
        }
    };

    template<>
    struct formatter<FStaticString> : formatter<const char8*>
    {
        template<typename FormatContext>
        auto format(const FStaticString& p, FormatContext& ctx)
        {
            return formatter<const char8*>::format(p.RawString(), ctx);
        }
    };
}
