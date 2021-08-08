#pragma once

#include "Simd.hpp"
#include "Array.hpp"
#include "SmartPointer.hpp"

#ifndef NULL_CHAR
#define NULL_CHAR static_cast<const char8>('\0')
#endif

class FString;
class FStaticString;

namespace StrUtil
{
    const char8* GetDirectoryName();

    //returns the number of characters in a string including the null terminator
    CONST int64 Length(const char8* NONNULL String);

    template<typename ValueType> requires(TypeTrait::IsInteger<ValueType>)
    CONST extern TStaticArray<char8, (sizeof(ValueType) * 2) + 1> IntToHex(ValueType Value);

    char8* ToUpperCase(char8* String LIFETIME_BOUND);
    FString& ToUpperCase(FString& String LIFETIME_BOUND);
    FStaticString& ToUpperCase(FStaticString& String LIFETIME_BOUND);

    char8* ToLowerCase(char8* String LIFETIME_BOUND);
    FString& ToLowerCase(FString& String LIFETIME_BOUND);
    FStaticString& ToLowerCase(FStaticString& String LIFETIME_BOUND);
}

template<uint64 Num>
struct TTemplateString
{
    constexpr implicit TTemplateString(const char8 (&Literal)[Num])
        : Array{Literal, Num}
    {
    }

    TStaticArray<char8, Num> Array;
};

/*
 * this string will store up to 60 characters on the stack
 * if the characters exceed 60 in size, it will switch over to heap allocation
 * not trivially copyable
 * now with constexpr, yay!
 */
class PACKED alignas(64) FString final
{
private:

    friend FString& StrUtil::ToUpperCase(FString&);
    friend FString& StrUtil::ToLowerCase(FString&);

    using TConstIterator = TRangedIterator<const char8>;
    using TMutableIterator = TRangedIterator<char8>;

public:

    enum class EActiveArray : bool
    {
        Stack = 0,
        Heap = 1,
    };

    //the mask we use to assign to the upper half of the stack
    inline static constexpr int32_8 MaskStoreMask{-1, -1, -1, -1, -1, -1, -1, 0};

    //how much to reserve extra if we need to reallocate
    inline static constexpr uint32 ReserveSize{32};

    //the number of characters that can fit in the stack
    inline static constexpr uint32 StackSize{60};

    constexpr explicit FString(ENoInit)
    {
    }

    constexpr FString()
        : TerminatorIndex{0}
    {
        CharacterArray.Stack[TerminatorIndex] = NULL_CHAR;
    }

    explicit constexpr FString(const uint32 NumChars)
        : CharacterArray{NumChars}
        , TerminatorIndex{NumChars - 1}
    {
    }

    template<uint64 NumChars>
    implicit constexpr FString(const char8 (&String)[NumChars])
        : CharacterArray{String}
        , TerminatorIndex{NumChars - 1}
    {
    }

    explicit constexpr FString(const char8* String, const uint32 NumChars)
        : CharacterArray{String, NumChars}
        , TerminatorIndex{NumChars - 1}
    {
    }

    FString(const FString& Other);

    FString(FString&& Other);

    constexpr ~FString()
    {
        if EXPECT(ActiveArray() == EActiveArray::Heap, false)
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

private:

    FString& Assign_Stack(const char8* RESTRICT String, const uint32 NumChars);

    FString& Assign_Heap(const char8* RESTRICT String, const uint32 NumChars);

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

    INLINE FString& operator+=(const FString& Other)
    {
        return Concat_Assign(Other.RawString(), Other.Num());
    }

    INLINE FString& Concat_Assign(const FString& Other)
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

    INLINE FString operator+(const FString& Other) const
    {
        return Concat_New(Other.RawString(), Other.Num());
    }

    INLINE FString Concat_New(const FString& Other) const
    {
        return Concat_New(Other.RawString(), Other.Num());
    }

    INLINE FString operator+(const FStaticString& Other) const
    {
        return Concat_New(Other);
    }

    FString Concat_New(const FStaticString& String) const;

private:

    FString& Concat_Assign(const char8* RESTRICT String, const uint32 NumChars);

    FString Concat_New(const char8* RESTRICT String, const uint32 NumChars) const;

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

    INLINE FString& operator<<=(const FString& Other)
    {
        return PushBack_Assign(Other.RawString(), Other.Num());
    }

    INLINE FString& PushBack_Assign(const FString& Other)
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

    INLINE FString operator<<(const FString& Other) const
    {
        return PushBack_New(Other.RawString(), Other.Num());
    }

    INLINE FString PushBack_New(const FString& Other) const
    {
        return PushBack_New(Other.RawString(), Other.Num());
    }

private:

    FString& PushBack_Assign(const char8* RESTRICT String, const uint32 NumChars);

    FString PushBack_New(const char8* RESTRICT String, const uint32 NumChars) const;

public:

    //removes all characters from begin (inclusive) to end (exclusive) and collapses the string
    FString& Erase_Assign(const uint32 Begin, const uint32 End);

    //removes all characters from begin (inclusive) to end (exclusive) and collapses the string
    FString Erase_New(const uint32 Begin, const uint32 End) const;

public:

    template<uint64 NumChars>
    INLINE FString& Replace_Assign(const uint32 Begin, const char8 (&String)[NumChars])
    {
        return Replace_Assign(Begin, String, NumChars);
    }

    INLINE FString& Replace_Assign(const uint32 Begin, const FString& Other)
    {
        return Replace_Assign(Begin, Other.RawString(), Other.Num());
    }

    template<uint64 NumChars>
    INLINE FString Replace_New(const uint32 Begin, const char8 (&String)[NumChars]) const
    {
        return Replace_New(Begin, String, NumChars);
    }

    INLINE FString Replace_New(const uint32 Begin, const FString& Other) const
    {
        return Replace_New(Begin, Other.RawString(), Other.Num());
    }

private:

    FString& Replace_Assign(const uint32 Begin, const char8* RESTRICT String, const uint32 NumChars);

    FString Replace_New(const uint32 Begin, const char8* RESTRICT String, const uint32 NumChars) const;

public:

    template<uint64 NumChars>
    INLINE FString& Insert_Assign(const uint32 Begin, const char8 (&String)[NumChars])
    {
        return Insert_Assign(Begin, String, NumChars);
    }

    INLINE FString& Insert_Assign(const uint32 Begin, const FString& Other)
    {
        return Insert_Assign(Begin, Other.RawString(), Other.Num());
    }

    template<uint64 NumChars>
    INLINE FString Insert_New(const uint32 Begin, const char8 (&String)[NumChars]) const
    {
        return Insert_New(Begin, String, NumChars);
    }

    INLINE FString Insert_New(const uint32 Begin, const FString& Other) const
    {
        return Insert_New(Begin, Other.RawString(), Other.Num());
    }

private:

    FString& Insert_Assign(const uint32 Begin, const char8* RESTRICT String, const uint32 NumChars);

    FString Insert_New(const uint32 Begin, const char8* RESTRICT String, const uint32 NumChars) const;

public:

    void Empty();

    template<uint64 NumChars>
    constexpr bool operator==(const char8 (&String)[NumChars]) const
    {
        if(Num() == NumChars)
        {
            return Memory::Compare(RawString(), String, NumChars) == 0;
        }
        return false;
    }

    constexpr bool operator==(const FString& Other) const
    {
        if(this->Num() == Other.Num())
        {
            return Memory::Compare(this->RawString(), Other.RawString(), Num()) == 0;
        }
        return false;
    }

    template<uint64 NumChars>
    constexpr bool operator!=(const char8 (&String)[NumChars]) const
    {
        if(Num() == NumChars)
        {
            return Memory::Compare(RawString(), String, NumChars) != 0;
        }
        return true;
    }

    constexpr bool operator!=(const FString& Other) const
    {
        if(this->Num() == Other.Num())
        {
            return Memory::Compare(this->RawString(), Other.RawString(), Num()) != 0;
        }
        return true;
    }

    INLINE constexpr EActiveArray ActiveArray() const
    {
        return static_cast<EActiveArray>(Num() > StackSize);
    }

    INLINE constexpr char8 operator[](const uint32 Index) const
    {
        ASSERT(Index < Num());
        return RawString()[Index];
    }

    INLINE constexpr char8& operator[](const uint32 Index)
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

    //used internally to copy characters from source to the stack array
    void CopyToStack(const char8* Source);

    union PACKED UCharacterArray final
    {
    public:

        constexpr UCharacterArray() = default;
        constexpr ~UCharacterArray() = default;

        template<uint64 NumChars> requires(NumChars <= StackSize)
        implicit constexpr UCharacterArray(const char8 (&String)[NumChars])
            : Stack{}
        {
            Memory::Copy(Stack, String, NumChars);
        }

        template<uint64 NumChars> requires(NumChars > StackSize)
        implicit UCharacterArray(const char8 (&String)[NumChars])
            : Heap{Memory::Allocate<char8>(NumChars)}
        {
            Memory::Copy(Heap, String, NumChars);
        }

        explicit constexpr UCharacterArray(const uint32 NumChars)
        {
            if EXPECT(NumChars <= StackSize, true)
            {
                (void)Stack;
            }
            else
            {
                Heap = Memory::Allocate<char8>(NumChars + ReserveSize);
            }
        }

        explicit constexpr UCharacterArray(const char8* String, const uint32 NumChars)
        {
            if EXPECT(NumChars <= StackSize, true)
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

    UCharacterArray CharacterArray;

    uint32 TerminatorIndex;
};

static_assert(alignof(FString) == 64);

/*
 * a string that fits 32 characters built using SIMD instructions
 * trivially copyable, no need to pass by reference
 * now with constexpr, yay!
 */
class alignas(32) FStaticString final
{
private:

    friend FStaticString& StrUtil::ToUpperCase(FStaticString&);
    friend FStaticString& StrUtil::ToLowerCase(FStaticString&);

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

    explicit FStaticString(char8_32 String)
        : Characters{Move(String)}
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

    FStaticString& Concat_Assign(const char8* RESTRICT String, const uint32 NumChars);

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

    FStaticString Concat_New(const char8* RESTRICT String, const uint32 NumChars) const;

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

    FStaticString& PushBack_Assign(const char8* RESTRICT String, const uint32 NumChars);

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

    FStaticString PushBack_New(const char8* RESTRICT String, const uint32 NumChars) const;

public:

    //removes all characters from begin (inclusive) to end (exclusive) and collapses the string
    FStaticString& Erase_Assign(const uint32 Begin, const uint32 End);

    //removes all characters from begin (inclusive) to end (exclusive) and collapses the string
    FStaticString Erase_New(const uint32 Begin, const uint32 End) const;

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
    consteval int32 MakeMask(const char8 (&String)[NumChars]) const
    {
        const char8_32 StringRegister{Simd::LoadUnaligned<char8_32>(String)};
        const int32 Mask{ComparisonMask >> (32 - NumChars)};
        return Mask & Simd::MoveMask(Characters == StringRegister);
    }

    static char8_32 CombineStrings(const char8_32 Lower, const char8_32 Upper, const uint32 LowEnd);

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

namespace fmt
{
    namespace detail
    {
        /*
        template<>
        struct char_t_impl<FString>
        {
            using type = char8;
        };

        template<>
        struct char_t_impl<FStaticString>
        {
            using type = char8;
        };

        template<>
        struct is_string<FString>
        {
            inline static constexpr bool value{true};
        };

        template<>
        struct is_string<FStaticString>
        {
            inline static constexpr bool value{true};
        };
        */
    }
    template<>
    struct formatter<FString> : formatter<const char8*>
    {
        template<typename FormatContext>
        auto format(const FString& p, FormatContext& ctx)
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
