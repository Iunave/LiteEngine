#include "String.hpp"
#include "Math.hpp"
#include "Simd.hpp"
#include <unistd.h>
#include <charconv>

namespace StrUtl
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
        using CharVectorType = char8_64;
#elif defined(AVX256)
        using CharVectorType = char8_32;
#endif
        constexpr CharVectorType NullCheckRegister{Simd::SetAll<CharVectorType>(NULL_CHAR)};
        constexpr int32 NumCharElements{static_cast<int32>(Simd::NumElements<CharVectorType>())};

        for(int32 Length{NumCharElements}; true; Length += NumCharElements)
        {
            const CharVectorType StringVector{Simd::LoadUnaligned<CharVectorType>((String + Length) - NumCharElements)};

            const Simd::MaskType<CharVectorType> Mask{Simd::MoveMask(StringVector == NullCheckRegister)};

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

    CONST char8* FindSubString(char8* SourceString, const uint32 NumSourceChars, const char8* SubString, const uint32 NumSubChars)
    {
        ASSERT(NumSubChars <= 32, "no implementation is made for larger sub-strings");
#ifdef AVX512
        using VectorType = Vector64<char8>;
#elif defined(AVX256)
        using VectorType = Vector32<char8>;
#endif
        VectorType SubCharVector{Simd::SetAll<VectorType>(SubString[0])};

        for(uint32 SourceIndex{0}; SourceIndex < NumSourceChars; SourceIndex += Simd::NumElements<VectorType>())
        {
            VectorType SourceVector{Simd::LoadUnaligned<VectorType>(SourceString + SourceIndex)};

            Simd::MaskType<VectorType> Mask{Simd::MoveMask(SubCharVector == SourceVector)};

            if(Mask > 0)
            {
                const int32 MatchIndex{Math::FindFirstSet(Mask) - 1};

                SourceVector = Simd::LoadUnaligned<VectorType>(SourceString + SourceIndex + MatchIndex);
                const VectorType SubVector{Simd::LoadUnaligned<VectorType>(SubString)};

                Mask = Simd::MoveMask(SourceVector == SubVector);
                const uint32 NumActiveBits{static_cast<uint32>(Math::NumActiveBits(Mask))};

                if EXPECT(NumActiveBits == (NumSubChars - 1), false)
                {
                    const bool bIsInValidRange{(SourceIndex + NumActiveBits) <= NumSourceChars};
                    const uint64 AddressAsInt{reinterpret_cast<uint64>(SourceString) + SourceIndex + MatchIndex};
                    return reinterpret_cast<char8*>(AddressAsInt * bIsInValidRange);
                }
            }
        }

        return nullptr;
    }

    CONST const char8* FindSubString(const char8* SourceString, const uint32 NumSourceChars, const char8* SubString, const uint32 NumSubChars)
    {
        ASSERT(NumSubChars <= 32, "no implementation is made for larger sub-strings");
#ifdef AVX512
        using VectorType = Vector64<char8>;
#elif defined(AVX256)
        using VectorType = Vector32<char8>;
#endif
        VectorType SubCharVector{Simd::SetAll<VectorType>(SubString[0])};

        for(uint32 SourceIndex{0}; SourceIndex < NumSourceChars; SourceIndex += Simd::NumElements<VectorType>())
        {
            VectorType SourceVector{Simd::LoadUnaligned<VectorType>(SourceString + SourceIndex)};

            Simd::MaskType<VectorType> Mask{Simd::MoveMask(SubCharVector == SourceVector)};

            if(Mask > 0)
            {
                const int32 MatchIndex{Math::FindFirstSet(Mask) - 1};

                SourceVector = Simd::LoadUnaligned<VectorType>(SourceString + SourceIndex + MatchIndex);
                const VectorType SubVector{Simd::LoadUnaligned<VectorType>(SubString)};

                Mask = Simd::MoveMask(SourceVector == SubVector);
                const uint32 NumActiveBits{static_cast<uint32>(Math::NumActiveBits(Mask))};

                if EXPECT(NumActiveBits == (NumSubChars - 1), false)
                {
                    const bool bIsInValidRange{(SourceIndex + NumActiveBits) <= NumSourceChars};
                    const uint64 AddressAsInt{reinterpret_cast<uint64>(SourceString) + SourceIndex + MatchIndex};
                    return reinterpret_cast<char8*>(AddressAsInt * bIsInValidRange);
                }
            }
        }

        return nullptr;
    }

#ifdef AVX512
    template<typename T>
    using StackStringVector = Vector64<T>;
#elif defined(AVX256)
    template<typename T>
    using StackStringVector = Vector32<T>;
#endif

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

    template<EStackSize InStackSize>
    FString<InStackSize>& ToUpperCase(FString<InStackSize>& String LIFETIME_BOUND)
    {
        if(String.ActiveArray() == FString<InStackSize>::EActiveArray::Stack)
        {
#if defined(AVX512)
            char8_64 CharacterVector{Simd::LoadUnaligned<char8_64>(String.CharacterArray.Stack)};
            const char8_64 Mask{LowerCaseMask(CharacterVector) & 32};

            CharacterVector -= Mask;

            int32* StackPointer{reinterpret_cast<int32*>(String.CharacterArray.Stack)};

            Simd::MaskStoreUnaligned(StackPointer, FString<InStackSize>::MaskStoreMask, static_cast<const int32_16>(CharacterVector));
#elif defined(AVX256)
            char8_32 CharacterVector{Simd::LoadUnaligned<char8_32>(String.CharacterArray.Stack)};
            char8_32 Mask{LowerCaseMask(CharacterVector) & 32};

            CharacterVector -= Mask;

            Simd::StoreUnaligned(String.CharacterArray.Stack, CharacterVector);

            CharacterVector = Simd::LoadUnaligned<char8_32>(&String.CharacterArray.Stack[32]);
            Mask = LowerCaseMask(CharacterVector) & 32;

            CharacterVector -= Mask;

            int32_8* StackPointer{reinterpret_cast<int32_8*>(&String.CharacterArray.Stack[32])};

            Simd::MaskStore(StackPointer, FString<InStackSize>::MaskStoreMask, static_cast<const int32_8>(CharacterVector));
#endif
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
        String.Characters -= (LowerCaseMask(String.Characters) & 32);
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

    template<EStackSize InStackSize>
    FString<InStackSize>& ToLowerCase(FString<InStackSize>& String LIFETIME_BOUND)
    {
        if(String.ActiveArray() == FString<InStackSize>::EActiveArray::Stack)
        {
#if defined(AVX512)
            char8_64 CharacterVector{Simd::LoadUnaligned<char8_64>(String.CharacterArray.Stack)};
            const char8_64 Mask{UpperCaseMask(CharacterVector) & 32};

            CharacterVector += Mask;

            int32* StackPointer{reinterpret_cast<int32*>(String.CharacterArray.Stack)};

            Simd::MaskStoreUnaligned(StackPointer, FString<InStackSize>::MaskStoreMask, static_cast<const int32_16>(CharacterVector));
#elif defined(AVX256)
            char8_32 CharacterVector{Simd::LoadUnaligned<char8_32>(String.CharacterArray.Stack)};
            char8_32 Mask{UpperCaseMask(CharacterVector) & 32};

            CharacterVector += Mask;

            Simd::StoreUnaligned(String.CharacterArray.Stack, CharacterVector);

            CharacterVector = Simd::LoadUnaligned<char8_32>(&String.CharacterArray.Stack[32]);
            Mask = UpperCaseMask(CharacterVector) & 32;

            CharacterVector += Mask;

            int32_8* StackPointer{reinterpret_cast<int32_8*>(&String.CharacterArray.Stack[32])};

            Simd::MaskStore(StackPointer, FString<InStackSize>::MaskStoreMask, static_cast<const int32_8>(CharacterVector));
#endif
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

    template<typename TargetType> requires(TypeTrait::IsInteger<TargetType>)
    TargetType ToValue(const char8* Begin, const char8* End)
    {
        TargetType ResultValue{0};
        TargetType PositionValue{1};

        while(End >= Begin)
        {
            const char8 Character{*End};

            if constexpr(TypeTrait::IsSigned<TargetType>)
            {
                if EXPECT(End == Begin, false)
                {
                    if(Character == '-')
                    {
                        ResultValue = -ResultValue;
                        break;
                    }
                    else if(Character == '+')
                    {
                        break;
                    }
                }
            }

            ASSERT_ALWAYS(Character >= '0' && Character <= '9', "invalid character {}", Character);

            TargetType CharacterValue{static_cast<TargetType>(Character - '0')};

            const bool HasOverflowed{Math::MulCheckOverflow(CharacterValue, PositionValue, &CharacterValue) || Math::AddCheckOverflow(ResultValue, CharacterValue, &ResultValue)};
            ASSERT_ALWAYS(!HasOverflowed, "overflow detected, value did not fit");

            PositionValue *= 10;
            --End;
        }

        return ResultValue;
    }

    template<typename TargetType> requires(TypeTrait::IsFloatingPoint<TargetType>)
    TargetType ToValue(const char8* Begin, const char8* End)
    {
        TargetType ResultValue{0};

        auto ReadDecimalValues = [&ResultValue, Begin, &End]() mutable -> void
        {
            while(End >= Begin)
            {
                if(*Begin == '.')
                {
                    break;
                }
                ++Begin;
            }

            for(int32 NumDecimals{static_cast<int32>(End - Begin)}; NumDecimals > 0; --NumDecimals, --End)
            {
                const char8 Character{*End};

                ASSERT_ALWAYS(Character >= '0' && Character <= '9', "invalid character {}", Character);

                TargetType CharacterValue{static_cast<TargetType>(Character - '0')};
                const TargetType PositionValue{Math::ToPower(static_cast<TargetType>(10), static_cast<TargetType>(NumDecimals))};

                CharacterValue /= PositionValue;
                ResultValue += CharacterValue;
            }

            End = Begin - 1;
        };

        auto ReadWholeValues = [&ResultValue, Begin, &End]() -> void
        {
            TargetType PositionValue{1};

            while(End >= Begin)
            {
                const char8 Character{*End};

                if EXPECT(End == Begin, false)
                {
                    if(Character == '-')
                    {
                        ResultValue = -ResultValue;
                        break;
                    }
                    else if(Character == '+')
                    {
                        break;
                    }
                }

                ASSERT_ALWAYS(Character >= '0' && Character <= '9', "invalid character {}", Character);

                TargetType CharacterValue{static_cast<TargetType>(Character - '0')};

                const bool HasOverflowed{Math::MulCheckOverflow(CharacterValue, PositionValue, &CharacterValue) || Math::AddCheckOverflow(ResultValue, CharacterValue, &ResultValue)};
                ASSERT_ALWAYS(!HasOverflowed, "overflow detected, value did not fit");

                PositionValue *= 10;
                --End;
            }
        };

        ReadDecimalValues();
        ReadWholeValues();

        return ResultValue;
    }

}

template char8 StrUtl::ToValue(const char8*, const char8*);
template int8 StrUtl::ToValue(const char8*, const char8*);
template uint8 StrUtl::ToValue(const char8*, const char8*);
template int16 StrUtl::ToValue(const char8*, const char8*);
template uint16 StrUtl::ToValue(const char8*, const char8*);
template int32 StrUtl::ToValue(const char8*, const char8*);
template uint32 StrUtl::ToValue(const char8*, const char8*);
template int64 StrUtl::ToValue(const char8*, const char8*);
template uint64 StrUtl::ToValue(const char8*, const char8*);
template float32 StrUtl::ToValue(const char8*, const char8*);
template float64 StrUtl::ToValue(const char8*, const char8*);

template TStaticArray<char8, 3> StrUtl::IntToHex(uint8);
template TStaticArray<char8, 3> StrUtl::IntToHex(int8);
template TStaticArray<char8, 5> StrUtl::IntToHex(uint16);
template TStaticArray<char8, 5> StrUtl::IntToHex(int16);
template TStaticArray<char8, 9> StrUtl::IntToHex(uint32);
template TStaticArray<char8, 9> StrUtl::IntToHex(int32);
template TStaticArray<char8, 17> StrUtl::IntToHex(unsigned long);
template TStaticArray<char8, 17> StrUtl::IntToHex(signed long);
template TStaticArray<char8, 17> StrUtl::IntToHex(uint64);
template TStaticArray<char8, 17> StrUtl::IntToHex(int64);
template TStaticArray<char8, 33> StrUtl::IntToHex(uint128);
template TStaticArray<char8, 33> StrUtl::IntToHex(int128);

template<uint64 VectorSize, EStackSize StackSize, uint64 OriginalVectorSize = VectorSize, uint64 Iterations = 1>
consteval uint64 CalculateNumIterations()
{
    if constexpr(VectorSize >= StackSize)
    {
        return Iterations;
    }
    else
    {
        return CalculateNumIterations<VectorSize + OriginalVectorSize, StackSize, OriginalVectorSize, Iterations + 1>();
    }
}

template<EStackSize InStackSize>
void FString<InStackSize>::CopyToStack(const char8* Source)
{
    ASSERT(ActiveArray() == EActiveArray::Stack);

#if defined(AVX512)
    using VectorType = Vector64<int32>;
    constexpr uint64 NumCharElements = 64;
#elif defined(AVX256)
    using VectorType = Vector32<int32>;
    constexpr uint64 NumCharElements = 32;
#endif

    int32* ThisCharacterArrayData{reinterpret_cast<int32*>(this->CharacterArray.Stack)};
    const int32* OtherCharacterArrayData{reinterpret_cast<const int32*>(Source)};

    #pragma unroll
    for(uint64 Iteration{0}; Iteration < CalculateNumIterations<NumCharElements, InStackSize>(); ++Iteration)
    {
        VectorType OtherCharacterRegister{Simd::LoadUnaligned<VectorType>(OtherCharacterArrayData)};

        if(Iteration == (CalculateNumIterations<NumCharElements, InStackSize>() - 1)) //should be optimized out
        {
#if defined(AVX512)
            Simd::MaskStoreUnaligned(ThisCharacterArrayData, MaskStoreMask, OtherCharacterRegister);
#elif defined(AVX256)
            Simd::MaskStore(reinterpret_cast<int32_8*>(ThisCharacterArrayData), MaskStoreMask, OtherCharacterRegister);
#endif
        }
        else
        {
            Simd::StoreUnaligned(ThisCharacterArrayData, OtherCharacterRegister);
        }

        ThisCharacterArrayData += (NumCharElements / sizeof(int32));
        OtherCharacterArrayData += (NumCharElements / sizeof(int32));
    }
}

template<EStackSize InStackSize>
FString<InStackSize>::FString(const FString& Other)
    : TerminatorIndex{Other.TerminatorIndex}
{
    if EXPECT(Other.ActiveArray() == EActiveArray::Heap, StackSize == SS0)
    {
        CharacterArray.Heap = Memory::Allocate<char8>(Num());
        Memory::Copy(CharacterArray.Heap, Other.CharacterArray.Heap, Num());
    }
    else
    {
        CopyToStack(Other.CharacterArray.Stack);
    }
}

template<EStackSize InStackSize>
FString<InStackSize>::FString(FString&& Other)
    : TerminatorIndex{Other.TerminatorIndex}
{
    if EXPECT(Other.ActiveArray() == EActiveArray::Heap, StackSize == SS0)
    {
        CharacterArray.Heap = Other.CharacterArray.Heap;
        Other.TerminatorIndex = 0;
    }
    else
    {
        CopyToStack(Other.CharacterArray.Stack);
    }
}

template<EStackSize InStackSize>
FString<InStackSize>& FString<InStackSize>::operator=(const FString& Other)
{
    if EXPECT(Other.ActiveArray() == EActiveArray::Heap, StackSize == SS0)
    {
        if EXPECT(this->ActiveArray() == EActiveArray::Heap, StackSize == SS0)
        {
            if(Other.Num() > Memory::AllocatedSize(CharacterArray.Heap))
            {
                CharacterArray.Heap = Memory::Reallocate(CharacterArray.Heap, Other.Num());
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

template<EStackSize InStackSize>
FString<InStackSize>& FString<InStackSize>::operator=(FString&& Other)
{
    if EXPECT(Other.ActiveArray() == EActiveArray::Heap, StackSize == SS0)
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

template<EStackSize InStackSize>
bool FString<InStackSize>::CompareEqual(const char8* String, const uint32 NumChars) const
{
#if defined(AVX512)
    using VectorType = Vector64<int32>;
    constexpr uint64 NumCharElements = 64;
#elif defined(AVX256)
    using VectorType = Vector32<int32>;
    constexpr uint64 NumCharElements = 32;
#endif

    if EXPECT(this->Num() == NumChars, false)
    {
        if EXPECT(ActiveArray() == EActiveArray::Heap, StackSize == SS0)
        {
            return Memory::Compare(this->RawString(), String, Num()) == 0;
        }
        else //todo is it important that all the stack not in use is zeroed, but dont do it here
        {
            VectorType ComparisonVector{Simd::SetAll<VectorType>(Math::Largest<int32>())};

            const int32* ThisCharacterArrayData{reinterpret_cast<const int32*>(this->CharacterArray.Stack)};
            const int32* OtherCharacterArrayData{reinterpret_cast<const int32*>(String)};

            VectorType ThisCharacterVector;
            VectorType OtherCharacterVector;

            #pragma unroll
            for(uint64 Iteration{0}; Iteration < CalculateNumIterations<NumCharElements, InStackSize>(); ++Iteration)
            {
                if(Iteration == (CalculateNumIterations<NumCharElements, InStackSize>() - 1)) //should be optimized out
                {
#if defined(AVX512)
                    ThisCharacterVector = Simd::MaskLoadUnaligned(ThisCharacterArrayData, MaskStoreMask);
                    OtherCharacterVector = Simd::MaskLoadUnaligned(OtherCharacterArrayData, MaskStoreMask);
#elif defined(AVX256)
                    ThisCharacterVector = Simd::MaskLoad(reinterpret_cast<const int32_8*>(ThisCharacterArrayData), MaskStoreMask);
                    OtherCharacterVector = Simd::MaskLoad(reinterpret_cast<const int32_8*>(OtherCharacterArrayData), MaskStoreMask);
#endif
                }
                else
                {
                    ThisCharacterVector = Simd::LoadUnaligned<VectorType>(ThisCharacterArrayData);
                    OtherCharacterVector = Simd::LoadUnaligned<VectorType>(OtherCharacterArrayData);
                }

                ComparisonVector &= (ThisCharacterVector == OtherCharacterVector);

                ThisCharacterArrayData += (NumCharElements / sizeof(int32));
                OtherCharacterArrayData += (NumCharElements / sizeof(int32));
            }

            return Simd::MoveMask(ComparisonVector) == Simd::Mask<VectorType>();
        }
    }
    return false;
}

template<EStackSize InStackSize>
FString<InStackSize>& FString<InStackSize>::Assign_Stack(const char8* String, const uint32 NumChars)
{
    if EXPECT(ActiveArray() == EActiveArray::Heap, false)
    {
        Memory::Free(CharacterArray.Heap);
    }

    CopyToStack(String);

    TerminatorIndex = NumChars - 1;

    return *this;
}

template<EStackSize InStackSize>
FString<InStackSize>& FString<InStackSize>::Assign_Heap(const char8* String, const uint32 NumChars)
{
    if EXPECT(ActiveArray() == EActiveArray::Heap, StackSize == SS0)
    {
        if(NumChars > Memory::AllocatedSize(CharacterArray.Heap))
        {
            CharacterArray.Heap = Memory::Reallocate(CharacterArray.Heap, NumChars + ReserveSize);
        }
    }
    else
    {
        CharacterArray.Heap = Memory::Allocate<char8>(NumChars + ReserveSize);
    }

    Memory::Copy(CharacterArray.Heap, String, NumChars);

    TerminatorIndex = NumChars - 1;

    return *this;
}

template<EStackSize InStackSize>
FString<InStackSize>& FString<InStackSize>::Concat_Assign(const char8* String, const uint32 NumChars)
{
    const uint32 NewSize{(Num() - 1) + NumChars};

    if EXPECT(ActiveArray() == EActiveArray::Heap, StackSize == SS0)
    {
        if(NewSize > Memory::AllocatedSize(CharacterArray.Heap))
        {
            CharacterArray.Heap = Memory::Reallocate(CharacterArray.Heap, NewSize + ReserveSize);
        }

        Memory::Copy(&CharacterArray.Heap[TerminatorIndex], String, NumChars);
    }
    else if EXPECT(NewSize > StackSize, false)
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

    TerminatorIndex += NumChars - 1;

    return *this;
}

template<EStackSize InStackSize>
FString<InStackSize> FString<InStackSize>::Concat_New(const char8* String, const uint32 NumChars) const
{
    const uint32 NewSize{TerminatorIndex + NumChars};

    FString ResultString{NewSize};

    char8* ResultMemory{ResultString.RawString()};

    Memory::Copy(ResultMemory, this->RawString(), Num() - 1);
    Memory::Copy(&ResultMemory[TerminatorIndex], String, NumChars);

    return ResultString;
}

template<EStackSize InStackSize>
FString<InStackSize>& FString<InStackSize>::Concat_Assign(const FStaticString& String)
{
    const uint32 NumChars{String.Num()};
    const uint32 NewSize{(Num() - 1) + NumChars};

    if EXPECT(ActiveArray() == EActiveArray::Heap, StackSize == SS0)
    {
        if(NewSize > Memory::AllocatedSize(CharacterArray.Heap))
        {
            CharacterArray.Heap = Memory::Reallocate(CharacterArray.Heap, NewSize + ReserveSize);
        }

        Memory::Copy(&CharacterArray.Heap[TerminatorIndex], String.RawString(), NumChars);
    }
    else if EXPECT(NewSize > StackSize, false)
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

    TerminatorIndex += NumChars - 1;

    return *this;
}

template<EStackSize InStackSize>
FString<InStackSize> FString<InStackSize>::Concat_New(const FStaticString& String) const
{
    const uint32 NumChars{String.Num()};
    const uint32 NewSize{TerminatorIndex + NumChars};

    FString ResultString{NewSize};

    char8* ResultMemory{ResultString.RawString()};

    Memory::Copy(ResultMemory, this->RawString(), this->Num() - 1);
    Memory::Copy(&ResultMemory[TerminatorIndex], String.RawString(), NumChars);

    return ResultString;
}

template<EStackSize InStackSize>
FString<InStackSize>& FString<InStackSize>::PushBack_Assign(const char8* String, const uint32 NumChars)
{
    const uint32 NewSize{TerminatorIndex + NumChars};

    if EXPECT(ActiveArray() == EActiveArray::Heap, StackSize == SS0)
    {
        if(NewSize > Memory::AllocatedSize(CharacterArray.Heap))
        {
            CharacterArray.Heap = Memory::Reallocate(CharacterArray.Heap, NewSize + ReserveSize);
        }

        Memory::Move(&CharacterArray.Heap[NumChars - 1], CharacterArray.Heap, Num());
        Memory::Copy(CharacterArray.Heap, String, NumChars - 1);
    }
    else if EXPECT(NewSize > StackSize, false)
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

    TerminatorIndex += NumChars - 1;

    return *this;
}

template<EStackSize InStackSize>
FString<InStackSize> FString<InStackSize>::PushBack_New(const char8* String, const uint32 NumChars) const
{
    const uint32 NewSize{(Num() - 1) + NumChars};

    FString ResultString{NewSize};
    char8* ResultMemory{ResultString.RawString()};

    Memory::Copy(ResultMemory, String, NumChars - 1);
    Memory::Copy(&ResultMemory[NumChars - 1], this->RawString(), Num());

    return ResultString;
}

template<EStackSize InStackSize>
FString<InStackSize>& FString<InStackSize>::Erase_Assign(const uint32 Begin, const uint32 End)
{
    ASSERT((End < Num()) && (Begin < End));

    const uint32 NumCharsLeftAtEnd{Num() - End};
    const uint32 NewSize{Begin + NumCharsLeftAtEnd};

    if EXPECT(NewSize > StackSize, StackSize == SS0)
    {
        Memory::Copy(&CharacterArray.Heap[Begin], &CharacterArray.Heap[End], NumCharsLeftAtEnd);
    }
    else if EXPECT(ActiveArray() == EActiveArray::Heap, false)
    {
        char8* Buffer{static_cast<char8*>(STACK_ALLOCATE(NewSize))};

        Memory::Copy(Buffer, CharacterArray.Heap, Begin);
        Memory::Copy(&Buffer[Begin], &CharacterArray.Heap[End], NumCharsLeftAtEnd);

        Memory::Copy(CharacterArray.Stack, Buffer, NewSize);
    }
    else
    {
        Memory::Copy(&CharacterArray.Stack[Begin], &CharacterArray.Stack[End], NumCharsLeftAtEnd);
    }

    TerminatorIndex = NewSize - 1;

    return *this;
}

template<EStackSize InStackSize>
FString<InStackSize> FString<InStackSize>::Erase_New(const uint32 Begin, const uint32 End) const
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

template<EStackSize InStackSize>
FString<InStackSize>& FString<InStackSize>::Replace_Assign(const uint32 Begin, const char8* String, const uint32 NumChars)
{
    const uint32 NewSize{Math::Max(Begin + NumChars, Num())};
    const uint32 ReplacementSize{NumChars - (NewSize < Num())};

    if EXPECT(ActiveArray() == EActiveArray::Heap, StackSize == SS0)
    {
        if EXPECT(NewSize > Memory::AllocatedSize(CharacterArray.Heap), false)
        {
            CharacterArray.Heap = Memory::Reallocate(CharacterArray.Heap, NewSize + ReserveSize);
        }

        Memory::Copy(&CharacterArray.Heap[Begin], String, ReplacementSize);
    }
    else
    {
        Memory::Copy(&CharacterArray.Stack[Begin], String, ReplacementSize);
    }

    TerminatorIndex = NewSize - 1;

    return *this;
}

template<EStackSize InStackSize>
FString<InStackSize> FString<InStackSize>::Replace_New(const uint32 Begin, const char8* String, const uint32 NumChars) const
{
    const uint32 NewSize{Math::Max(Begin + NumChars, Num())};
    const uint32 ReplacementSize{NumChars - (NewSize < Num())};

    FString ResultString{NewSize};
    char8* ResultMemory{ResultString.RawString()};

    Memory::Copy(ResultMemory, this->RawString(), Num());
    Memory::Copy(&ResultMemory[Begin], String, ReplacementSize);

    return ResultString;
}

template<EStackSize InStackSize>
FString<InStackSize>& FString<InStackSize>::Insert_Assign(const uint32 Begin, const char8* String, const uint32 NumChars)
{
    ASSERT(Begin < TerminatorIndex);

    const uint32 NewSize{Math::Max(Begin + NumChars + 1, Num())};
    const uint32 InsertionSize{NumChars - (NewSize < Num())}; //lewd

    if EXPECT(ActiveArray() == EActiveArray::Heap, StackSize == SS0)
    {
        if EXPECT(NewSize > Memory::AllocatedSize(CharacterArray.Heap), false)
        {
            CharacterArray.Heap = Memory::Reallocate(CharacterArray.Heap, NewSize + ReserveSize);
        }

        Memory::Copy(&CharacterArray.Heap[Begin + InsertionSize], &CharacterArray.Heap[Begin], InsertionSize);
        Memory::Copy(&CharacterArray.Heap[Begin + 1], String, InsertionSize);
    }
    else if(NewSize > StackSize)
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

    TerminatorIndex = NewSize - 1;

    return *this;
}

template<EStackSize InStackSize>
FString<InStackSize> FString<InStackSize>::Insert_New(const uint32 Begin, const char8* String, const uint32 NumChars) const
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

template<EStackSize InStackSize>
char8* FString<InStackSize>::FindSubstring(const char8* String, const uint32 NumChars)
{
    return StrUtl::FindSubString(Data(), Num(), String, NumChars);
}

template<EStackSize InStackSize>
const char8* FString<InStackSize>::FindSubstring(const char8* String, const uint32 NumChars) const
{
    return StrUtl::FindSubString(Data(), Num(), String, NumChars);
}

template<EStackSize InStackSize>
void FString<InStackSize>::Empty()
{
    if EXPECT(ActiveArray() == EActiveArray::Heap, StackSize == SS0)
    {
        Memory::Free(CharacterArray.Heap);
    }

    TerminatorIndex = 0;

    if constexpr(StackSize > SS0)
    {
        CharacterArray.Stack[TerminatorIndex] = NULL_CHAR;
    }
}

template class FString<SS0>;
template class FString<SS60>;
template class FString<SS124>;
template class FString<SS256>;
template class FString<SS508>;

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

FStaticString& FStaticString::Concat_Assign(const char8* String, const uint32 NumChars)
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

FStaticString FStaticString::Concat_New(const char8* String, const uint32 NumChars attr(unused)) const
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

FStaticString& FStaticString::PushBack_Assign(const char8* String, const uint32 NumChars)
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

FStaticString FStaticString::PushBack_New(const char8* String, const uint32 NumChars) const
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
