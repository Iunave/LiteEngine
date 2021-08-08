#include "String.hpp"
#include "Math.hpp"
#include "Simd.hpp"
#include <unistd.h>

namespace StrUtil
{
    const char8* GetDirectoryName()
    {
        struct FAutoFreeString
        {
            ~FAutoFreeString()
            {
                Memory::Free(String);
            }

            char8* String;
        };

        static const FAutoFreeString DirName{::get_current_dir_name()};
        return DirName.String;
    }

    CONST int64 Length(const char8* NONNULL String)
    {
        #if defined(AVX512)
            using CharRegisterType = char8_64;
        #elif defined(AVX256)
            using CharRegisterType = char8_32;
        #elif defined(AVX128)
            using CharRegisterType = char8_16;
        #endif

        static constexpr CharRegisterType NullCheckRegister{Simd::SetAll<CharRegisterType>(NULL_CHAR)};
        static constexpr int32 NumCharElements{static_cast<int64>(Simd::NumElements<CharRegisterType>())};

        for(int32 Length{NumCharElements}; true; Length += NumCharElements)
        {
            const CharRegisterType StringRegister{Simd::LoadUnaligned<CharRegisterType>((String + Length) - NumCharElements)};

            const int32 Mask{Simd::MoveMask(StringRegister == NullCheckRegister)};

            if(Mask > 0)
            {
                const int32 NumInvalidCharacters{NumCharElements - Math::FindFirstSet(Mask)};
                return Length - NumInvalidCharacters;
            }
        }
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshift-count-overflow"

    template<typename ValueType> requires(TypeTrait::IsInteger<ValueType>)
    CONST TStaticArray<char8, (sizeof(ValueType) * 2) + 1> IntToHex(ValueType Value)
    {
        static constexpr TStaticArray<char8, 16> HexTable{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        TStaticArray<char8, (sizeof(ValueType) * 2) + 1> HexString{};
        HexString[sizeof(ValueType) * 2] = NULL_CHAR;

        #pragma unroll
        for(uint64 Index{0}; Index < ((sizeof(ValueType) * 2) - 1); Value >>= 8)
        {
            HexString[Index] = HexTable[static_cast<const uint8>(Value) >> 4];
            ++Index;
            HexString[Index] = HexTable[static_cast<const uint8>(Value) & 0b00001111];
        }

        return HexString;
    }

#pragma clang diagnostic pop

    template<typename T>
    INLINE T LowerCaseMask(const T Character)
    {
        return Character >= 'a' && Character <= 'z';
    }

    template<typename T>
    INLINE T UpperCaseMask(const T Character)
    {
        return Character >= 'A' && Character <= 'Z';
    }

    char8* ToUpperCase(char8* String LIFETIME_BOUND)
    {
        //index to the last valid character
        for(int64 StringEnd{Length(String) - 2}; StringEnd >= 0; --StringEnd)
        {
            String[StringEnd] -= (32 * LowerCaseMask(String[StringEnd]));
        }

        return String;
    }

    FString& ToUpperCase(FString& String LIFETIME_BOUND)
    {
        if(String.ActiveArray() == FString::EActiveArray::Stack)
        {
            char8_32 CharacterRegister{Simd::LoadUnaligned<char8_32>(String.CharacterArray.Stack)};
            char8_32 Mask{LowerCaseMask(CharacterRegister) & 32};

            CharacterRegister -= Mask;

            Simd::StoreUnaligned(String.CharacterArray.Stack, CharacterRegister);

            CharacterRegister = Simd::LoadUnaligned<char8_32>(&String.CharacterArray.Stack[32]);
            Mask = LowerCaseMask(CharacterRegister) & 32;

            CharacterRegister -= Mask;

            int32_8* StackPointer{reinterpret_cast<int32_8*>(&String.CharacterArray.Stack[32])};

            Simd::MaskStore(StackPointer, FString::MaskStoreMask, static_cast<int32_8>(CharacterRegister));
        }
        else
        {
            for(char8& Character : String)
            {
                Character -= (32 * LowerCaseMask(Character));
            }
        }

        return String;
    }

    FStaticString& ToUpperCase(FStaticString& String LIFETIME_BOUND)
    {
        String.Characters -= LowerCaseMask(String.Characters) & 32;
        return String;
    }

    char8* ToLowerCase(char8* String LIFETIME_BOUND)
    {
        //index to the last valid character
        for(int64 StringEnd{Length(String) - 2}; StringEnd >= 0; --StringEnd)
        {
            String[StringEnd] += (32 * UpperCaseMask(String[StringEnd]));
        }

        return String;
    }

    FString& ToLowerCase(FString& String LIFETIME_BOUND)
    {
        if(String.ActiveArray() == FString::EActiveArray::Stack)
        {
            char8_32 CharacterRegister{Simd::LoadUnaligned<char8_32>(String.CharacterArray.Stack)};
            char8_32 Mask{UpperCaseMask(CharacterRegister) & 32};

            CharacterRegister += Mask;

            Simd::StoreUnaligned(String.CharacterArray.Stack, CharacterRegister);

            CharacterRegister = Simd::LoadUnaligned<char8_32>(&String.CharacterArray.Stack[32]);
            Mask = UpperCaseMask(CharacterRegister) & 32;

            CharacterRegister += Mask;

            int32_8* StackPointer{reinterpret_cast<int32_8*>(&String.CharacterArray.Stack[32])};

            Simd::MaskStore(StackPointer, FString::MaskStoreMask, static_cast<int32_8>(CharacterRegister));
        }
        else
        {
            for(char8& Character : String)
            {
                Character += (32 * UpperCaseMask(Character));
            }
        }

        return String;
    }

    FStaticString& ToLowerCase(FStaticString& String LIFETIME_BOUND)
    {
        String.Characters += UpperCaseMask(String.Characters) & 32;
        return String;
    }

}

template TStaticArray<char8, 3> StrUtil::IntToHex(uint8);
template TStaticArray<char8, 3> StrUtil::IntToHex(int8);
template TStaticArray<char8, 5> StrUtil::IntToHex(uint16);
template TStaticArray<char8, 5> StrUtil::IntToHex(int16);
template TStaticArray<char8, 9> StrUtil::IntToHex(uint32);
template TStaticArray<char8, 9> StrUtil::IntToHex(int32);
template TStaticArray<char8, 17> StrUtil::IntToHex(unsigned long);
template TStaticArray<char8, 17> StrUtil::IntToHex(signed long);
template TStaticArray<char8, 17> StrUtil::IntToHex(uint64);
template TStaticArray<char8, 17> StrUtil::IntToHex(int64);
template TStaticArray<char8, 33> StrUtil::IntToHex(uint128);
template TStaticArray<char8, 33> StrUtil::IntToHex(int128);

void FString::CopyToStack(const char8* Source)
{
    ASSERT(ActiveArray() == EActiveArray::Stack);

    int32* ThisCharacterArrayData{reinterpret_cast<int32*>(this->CharacterArray.Stack)};
    const int32* OtherCharacterArrayData{reinterpret_cast<const int32*>(Source)};

    int32_8 OtherCharacterRegister{Simd::LoadUnaligned<int32_8>(OtherCharacterArrayData)};

    Simd::StoreUnaligned(ThisCharacterArrayData, OtherCharacterRegister);

    ThisCharacterArrayData += (32 / sizeof(int32));
    OtherCharacterArrayData += (32 / sizeof(int32));

    OtherCharacterRegister = Simd::LoadUnaligned<int32_8>(OtherCharacterArrayData);

    Simd::MaskStore(reinterpret_cast<int32_8*>(ThisCharacterArrayData), MaskStoreMask, OtherCharacterRegister);
}

FString::FString(const FString& Other)
    : TerminatorIndex{Other.TerminatorIndex}
{
    if EXPECT(Other.ActiveArray() == EActiveArray::Heap, false)
    {
        CharacterArray.Heap = Memory::Allocate<char8>(Num());
        Memory::Copy(CharacterArray.Heap, Other.CharacterArray.Heap, Num());
    }
    else
    {
        CopyToStack(Other.CharacterArray.Stack);
    }
}

FString::FString(FString&& Other)
    : TerminatorIndex{Other.TerminatorIndex}
{
    if EXPECT(Other.ActiveArray() == EActiveArray::Heap, false)
    {
        CharacterArray.Heap = Other.CharacterArray.Heap;
        Other.TerminatorIndex = 0;
    }
    else
    {
        CopyToStack(Other.CharacterArray.Stack);
    }
}

FString& FString::operator=(const FString& Other)
{
    if EXPECT(Other.ActiveArray() == EActiveArray::Heap, false)
    {
        if EXPECT(this->ActiveArray() == EActiveArray::Heap,false)
        {
            if(Other.Num() > Memory::AllocatedSize(CharacterArray.Heap))
            {
                CharacterArray.Heap = Memory::ReAllocate(CharacterArray.Heap, Other.Num());
            }
        }
        else
        {
            CharacterArray.Heap = Memory::Allocate<char8>(Other.Num());
        }

        Memory::Copy(CharacterArray.Heap, Other.CharacterArray.Heap, Other.Num());
    }
    else
    {
        if EXPECT(this->ActiveArray() == EActiveArray::Heap,false)
        {
            Memory::Free(CharacterArray.Heap);
        }

        CopyToStack(Other.CharacterArray.Stack);
    }

    TerminatorIndex = Other.TerminatorIndex;
    return *this;
}

FString& FString::operator=(FString&& Other)
{
    if EXPECT(Other.ActiveArray() == EActiveArray::Heap, false)
    {
        TerminatorIndex = Other.TerminatorIndex;
        CharacterArray.Heap = Other.CharacterArray.Heap;
        Other.TerminatorIndex = 0;
    }
    else
    {
        if EXPECT(this->ActiveArray() == EActiveArray::Heap, false)
        {
            Memory::Free(CharacterArray.Heap);
        }

        CopyToStack(Other.CharacterArray.Stack);

        TerminatorIndex = Other.TerminatorIndex;
    }

    return *this;
}

FString& FString::Assign_Stack(const char8* RESTRICT String, const uint32 NumChars)
{
    if EXPECT(ActiveArray() == EActiveArray::Heap, false)
    {
        Memory::Free(CharacterArray.Heap);
    }

    CopyToStack(String);

    TerminatorIndex = NumChars - 1;

    return *this;
}

FString& FString::Assign_Heap(const char8* RESTRICT String, const uint32 NumChars)
{
    if EXPECT(ActiveArray() == EActiveArray::Heap, false)
    {
        if(NumChars > Memory::AllocatedSize(CharacterArray.Heap))
        {
            CharacterArray.Heap = Memory::ReAllocate(CharacterArray.Heap, NumChars + ReserveSize);
        }
    }
    else
    {
        CharacterArray.Heap = Memory::Allocate<char8>(NumChars + ReserveSize);
    }

    Memory::Copy(CharacterArray.Heap, String, NumChars);

    TerminatorIndex = (NumChars - 1);

    return *this;
}

FString& FString::Concat_Assign(const char8* RESTRICT String, const uint32 NumChars)
{
    const uint32 NewSize{(Num() - 1) + NumChars};

    if EXPECT(ActiveArray() == EActiveArray::Heap, false)
    {
        if(NewSize > Memory::AllocatedSize(CharacterArray.Heap))
        {
            CharacterArray.Heap = Memory::ReAllocate(CharacterArray.Heap, NewSize + ReserveSize);
        }

        Memory::Copy(&CharacterArray.Heap[TerminatorIndex], String, NumChars);
    }
    else if EXPECT(NewSize > 60, false)
    {
        char8* Buffer{Memory::Allocate<char8>(NewSize + ReserveSize)};

        Memory::Copy(Buffer, CharacterArray.Stack, Num() - 1);
        Memory::Copy(&Buffer[TerminatorIndex], String, NumChars);

        CharacterArray.Heap = Buffer;
    }
    else
    {
        Memory::Copy(&CharacterArray.Stack[TerminatorIndex], String, NumChars);
    }

    TerminatorIndex += (NumChars - 1);

    return *this;
}

FString FString::Concat_New(const char8* RESTRICT String, const uint32 NumChars) const
{
    const uint32 NewSize{TerminatorIndex + NumChars};

    FString ResultString{NewSize};

    char8* ResultMemory{ResultString.RawString()};

    Memory::Copy(ResultMemory, this->RawString(), Num() - 1);
    Memory::Copy(&ResultMemory[TerminatorIndex], String, NumChars);

    return ResultString;
}

FString& FString::Concat_Assign(const FStaticString& String)
{
    const uint32 NumChars{String.Num()};
    const uint32 NewSize{(Num() - 1) + NumChars};

    if EXPECT(ActiveArray() == EActiveArray::Heap, false)
    {
        if(NewSize > Memory::AllocatedSize(CharacterArray.Heap))
        {
            CharacterArray.Heap = Memory::ReAllocate(CharacterArray.Heap, NewSize + ReserveSize);
        }

        Memory::Copy(&CharacterArray.Heap[TerminatorIndex], String.RawString(), NumChars);
    }
    else if EXPECT(NewSize > 60, false)
    {
        char8* Buffer{Memory::Allocate<char8>(NewSize + ReserveSize)};

        Memory::Copy(Buffer, CharacterArray.Stack, Num() - 1);
        Memory::Copy(&Buffer[TerminatorIndex], String.RawString(), NumChars);

        CharacterArray.Heap = Buffer;
    }
    else
    {
        Memory::Copy(&CharacterArray.Stack[TerminatorIndex], String.RawString(), NumChars);
    }

    TerminatorIndex += (NumChars - 1);

    return *this;
}

FString FString::Concat_New(const FStaticString& String) const
{
    const uint32 NumChars{String.Num()};
    const uint32 NewSize{TerminatorIndex + NumChars};

    FString ResultString{NewSize};

    char8* ResultMemory{ResultString.RawString()};

    Memory::Copy(ResultMemory, this->RawString(), this->Num() - 1);
    Memory::Copy(&ResultMemory[TerminatorIndex], String.RawString(), NumChars);

    return ResultString;
}

FString& FString::PushBack_Assign(const char8* RESTRICT String, const uint32 NumChars)
{
    const uint32 NewSize{TerminatorIndex + NumChars};

    if EXPECT(ActiveArray() == EActiveArray::Heap, false)
    {
        if(NewSize > Memory::AllocatedSize(CharacterArray.Heap))
        {
            CharacterArray.Heap = Memory::ReAllocate(CharacterArray.Heap, NewSize + ReserveSize);
        }

        Memory::Move(&CharacterArray.Heap[NumChars - 1], CharacterArray.Heap, Num());
        Memory::Copy(CharacterArray.Heap, String, NumChars - 1);
    }
    else if EXPECT(NewSize > 60, false)
    {
        char8* Buffer{Memory::Allocate<char8>(NewSize + ReserveSize)};

        Memory::Copy(&Buffer[NumChars - 1], CharacterArray.Stack, Num());
        Memory::Copy(Buffer, String, NumChars - 1);

        CharacterArray.Heap = Buffer;
    }
    else
    {
        Memory::Move(&CharacterArray.Stack[NumChars - 1], CharacterArray.Stack, Num());
        Memory::Copy(CharacterArray.Stack, String, NumChars - 1);
    }

    TerminatorIndex += (NumChars - 1);

    return *this;
}

FString FString::PushBack_New(const char8* RESTRICT String, const uint32 NumChars) const
{
    const uint32 NewSize{(Num() - 1) + NumChars};

    FString ResultString{NewSize};
    char8* ResultMemory{ResultString.RawString()};

    Memory::Copy(ResultMemory, String, NumChars - 1);
    Memory::Copy(&ResultMemory[NumChars - 1], this->RawString(), Num());

    return ResultString;
}

FString& FString::Erase_Assign(const uint32 Begin, const uint32 End)
{
    ASSERT((End < Num()) && (Begin < End));

    const uint32 NumCharsLeftAtEnd{Num() - End};
    const uint32 NewSize{Begin + NumCharsLeftAtEnd};

    if(NewSize > 60)
    {
        Memory::Move(&CharacterArray.Heap[Begin], &CharacterArray.Heap[End], NumCharsLeftAtEnd);
    }
    else if(ActiveArray() == EActiveArray::Heap)
    {
        char8* Buffer{static_cast<char8*>(STACK_ALLOCATE(NewSize))};

        Memory::Copy(Buffer, CharacterArray.Heap, Begin);
        Memory::Copy(&Buffer[Begin], &CharacterArray.Heap[End], NumCharsLeftAtEnd);

        Memory::Copy(CharacterArray.Stack, Buffer, NewSize);
    }
    else
    {
        Memory::Move(&CharacterArray.Stack[Begin], &CharacterArray.Stack[End], NumCharsLeftAtEnd);
    }

    TerminatorIndex = (NewSize - 1);

    return *this;
}

FString FString::Erase_New(const uint32 Begin, const uint32 End) const
{
    ASSERT((End < Num()) && (Begin < End));

    const uint32 NumCharsLeftAtEnd{Num() - End};
    const uint32 NewSize{Begin + NumCharsLeftAtEnd};

    FString ResultString{NewSize};

    char8* ResultMemory{ResultString.RawString()};
    const char8* SourceMemory{this->RawString()};

    Memory::Copy(ResultMemory, SourceMemory, Begin);
    Memory::Copy(&ResultMemory[Begin], &SourceMemory[End], NumCharsLeftAtEnd);

    return ResultString;
}

FString& FString::Replace_Assign(const uint32 Begin, const char8* RESTRICT String, const uint32 NumChars)
{
    const uint32 NewSize{Math::Max(Begin + NumChars, Num())};
    const uint32 ReplacementSize{NumChars - (NewSize < Num())};

    if(ActiveArray() == EActiveArray::Heap)
    {
        if EXPECT(NewSize > Memory::AllocatedSize(CharacterArray.Heap), false)
        {
            CharacterArray.Heap = Memory::ReAllocate(CharacterArray.Heap, NewSize + ReserveSize);
        }

        Memory::Copy(&CharacterArray.Heap[Begin], String, ReplacementSize);
    }
    else
    {
        Memory::Copy(&CharacterArray.Stack[Begin], String, ReplacementSize);
    }

    TerminatorIndex = (NewSize - 1);

    return *this;
}

FString FString::Replace_New(const uint32 Begin, const char8* RESTRICT String, const uint32 NumChars) const
{
    const uint32 NewSize{Math::Max(Begin + NumChars, Num())};
    const uint32 ReplacementSize{NumChars - (NewSize < Num())};

    FString ResultString{NewSize};
    char8* ResultMemory{ResultString.RawString()};

    Memory::Copy(ResultMemory, this->RawString(), Num());
    Memory::Copy(&ResultMemory[Begin], String, ReplacementSize);

    return ResultString;
}

FString& FString::Insert_Assign(const uint32 Begin, const char8* RESTRICT String, const uint32 NumChars)
{
    ASSERT(Begin < TerminatorIndex);

    const uint32 NewSize{Math::Max(Begin + NumChars + 1, Num())};
    const uint32 InsertionSize{NumChars - (NewSize < Num())}; //lewd

    if(ActiveArray() == EActiveArray::Heap)
    {
        if EXPECT(NewSize > Memory::AllocatedSize(CharacterArray.Heap), false)
        {
            CharacterArray.Heap = Memory::ReAllocate(CharacterArray.Heap, NewSize + ReserveSize);
        }

        Memory::Copy(&CharacterArray.Heap[Begin + InsertionSize], &CharacterArray.Heap[Begin], InsertionSize);
        Memory::Copy(&CharacterArray.Heap[Begin + 1], String, InsertionSize);
    }
    else if(NewSize > 60)
    {
        char8* Buffer{Memory::Allocate<char8>(NewSize + ReserveSize)};

        Memory::Copy(Buffer, CharacterArray.Stack, Begin + 1);
        Memory::Copy(&Buffer[Begin + InsertionSize], &CharacterArray.Stack[Begin], Num() - Begin);
        Memory::Copy(&Buffer[Begin + 1], String, InsertionSize);

        CharacterArray.Heap = Buffer;
    }
    else
    {
        Memory::Copy(&CharacterArray.Stack[Begin + InsertionSize], &CharacterArray.Stack[Begin], InsertionSize);
        Memory::Copy(&CharacterArray.Stack[Begin + 1], String, InsertionSize);
    }

    TerminatorIndex = (NewSize - 1);

    return *this;
}

FString FString::Insert_New(const uint32 Begin, const char8* RESTRICT String, const uint32 NumChars) const
{
    ASSERT(Begin < TerminatorIndex);

    const uint32 NewSize{Math::Max(Begin + NumChars + 1, Num())};
    const uint32 InsertionSize{NumChars - (NewSize < Num())};

    FString ResultString{NewSize};

    char8* ResultMemory{ResultString.RawString()};
    const char8* SourceMemory{this->RawString()};

    Memory::Copy(ResultMemory, SourceMemory, Begin + 1);
    Memory::Copy(&ResultMemory[Begin + InsertionSize], &SourceMemory[Begin], Num() - Begin);
    Memory::Copy(&ResultMemory[Begin + 1], String, InsertionSize);

    return ResultString;
}

void FString::Empty()
{
    if EXPECT(ActiveArray() == EActiveArray::Heap, false)
    {
        Memory::Free(CharacterArray.Heap);
    }

    TerminatorIndex = 0;
    CharacterArray.Stack[TerminatorIndex] = NULL_CHAR;
}

char8_32 FStaticString::CombineStrings(const char8_32 Lower, const char8_32 Upper, const uint32 LowEnd)
{
    char8* StringBuffer{static_cast<char8*>(STACK_ALLOCATE(MaxNumCharacters * 2))};
    Simd::StoreUnaligned(StringBuffer, Lower);
    Simd::StoreUnaligned(&StringBuffer[LowEnd], Upper);

    return Simd::LoadUnaligned<char8_32>(StringBuffer);
}

FStaticString& FStaticString::Concat_Assign(FStaticString Other)
{
    ASSERT(((Num() - 1) + Other.Num()) <= MaxNumCharacters);

    Other.Characters = Simd::ShuffleLeft(Other.Characters, Other.Num() - 1);
    this->Characters |= Other.Characters;

    return *this;
}

FStaticString& FStaticString::Concat_Assign(const char8* RESTRICT String, const uint32 NumChars)
{
    ASSERT(((Num() - 1) + NumChars) <= MaxNumCharacters);

    char8_32 StringRegister{Simd::LoadUnaligned<char8_32>(String - (Num() - 1))};
    CombineStrings(Characters, StringRegister, Num() - 1);

    return *this;
}

FStaticString FStaticString::Concat_New(FStaticString Other) const
{
    ASSERT(((Num() - 1) + Other.Num()) <= MaxNumCharacters);

    Other.Characters = Simd::ShuffleLeft(Other.Characters, Other.Num() - 1);
    Other.Characters |= this->Characters;

    return Other;
}

FStaticString FStaticString::Concat_New(const char8* RESTRICT String, const uint32 NumChars) const
{
    ASSERT(((Num() - 1) + NumChars) <= MaxNumCharacters);

    const char8_32 StringRegister{Simd::LoadUnaligned<char8_32>(String)};

    return FStaticString{CombineStrings(Characters, StringRegister, Num() - 1)};
}

FStaticString& FStaticString::PushBack_Assign(FStaticString Other)
{
    ASSERT(((Num() - 1) + Other.Num()) <= MaxNumCharacters);

    Characters = Simd::ShuffleLeft(Characters, Other.Num() - 1);
    Characters |= Other.Characters;

    return *this;
}

FStaticString& FStaticString::PushBack_Assign(const char8* RESTRICT String, const uint32 NumChars)
{
    ASSERT(((Num() - 1) + NumChars) <= MaxNumCharacters);

    const char8_32 StringRegister{Simd::LoadUnaligned<char8_32>(String)};

    Characters = CombineStrings(StringRegister, Characters, NumChars - 1);

    return *this;
}

FStaticString FStaticString::PushBack_New(FStaticString Other) const
{
    ASSERT(((Num() - 1) + Other.Num()) <= MaxNumCharacters);

    const char8_32 ShuffledRegister{Simd::ShuffleLeft(this->Characters, Other.Num() - 1)};
    Other.Characters |= ShuffledRegister;

    return Other;
}

FStaticString FStaticString::PushBack_New(const char8* RESTRICT String, const uint32 NumChars) const
{
    ASSERT(((Num() - 1) + NumChars) <= MaxNumCharacters);

    const char8_32 StringRegister{Simd::LoadUnaligned<char8_32>(String)};

    return FStaticString{CombineStrings(StringRegister, Characters, NumChars - 1)};
}

FStaticString& FStaticString::Erase_Assign(const uint32 Begin, const uint32 End) //todo
{
    ASSERT((End < Num()) && (Begin < End));

    const uint32 NumCharsLeftAtEnd{Num() - End};

    Memory::Move(&RawString()[Begin], &RawString()[End + 1], NumCharsLeftAtEnd);

    return *this;
}

FStaticString FStaticString::Erase_New(const uint32 Begin, const uint32 End) const //todo
{
    ASSERT((End < Num()) && (Begin < End));

    const uint32 NumCharsLeftAtEnd{Num() - End};

    FStaticString NewString{*this};
    Memory::Copy(&NewString.RawString()[Begin], &this->RawString()[End + 1], NumCharsLeftAtEnd);

    return NewString;
}
