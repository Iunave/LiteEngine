#include "IniConfig.hpp"
#include "Log.hpp"

IIniConfig::IIniConfig()
{

}

IIniConfig::~IIniConfig()
{

}

TDynamicArray<char8> IIniConfig::LoadFileForRead() const
{
    fmt::file ConfigFile{ConfigFilePath.Data(), fmt::file::RDONLY};

    TDynamicArray<char8> FileContents{};
    FileContents.ResizeTo(ConfigFile.size());

    ConfigFile.read(FileContents.Data(), FileContents.Num());

    return FileContents;
}

TDynamicArray<char8> IIniConfig::LoadFileForWrite() const
{
    fmt::file ConfigFile{ConfigFilePath.Data(), fmt::file::RDWR};

    TDynamicArray<char8> FileContents{};
    FileContents.ResizeTo(ConfigFile.size());

    ConfigFile.read(FileContents.Data(), FileContents.Num());

    return FileContents;
}

char8* IIniConfig::FindValueStart(TDynamicArray<char8>& FileContents, const FStaticString VariableName)
{
    char8_32 VarNameVector{VariableName.Characters};
    const uint32 VarNameLength{VariableName.Length()};

    const int32 ValidMask{static_cast<int32>(0xFFFFFFFF >> (32 - (VarNameLength - 1)))};

    auto AdvanceNextRow = [&FileContents](int64& Index) -> void
    {
        #ifdef AVX512
            using VectorType = char8_64;
        #else
            using VectorType = char8_32;
        #endif

        #pragma unroll 128 / Simd::NumElements<VectorType>()
        while(true)
        {
            VectorType LoadedChars{Simd::LoadUnaligned<VectorType>(FileContents + Index)};

            Simd::MaskType<VectorType> Mask{Simd::MoveMask(LoadedChars == Simd::SetAll<VectorType>('\n'))};
            Mask |= Simd::MoveMask(LoadedChars == Simd::SetAll<VectorType>('\0'));

            if(Mask != 0)
            {
                Index += Math::FindFirstSet(Mask);
                return;
            }

            Index += Simd::NumElements<VectorType>();
        }
    };

    for(int64 Index{0}; Index < FileContents.Num(); AdvanceNextRow(Index))
    {
        char8_32 LoadedChars{Simd::LoadUnaligned<char8_32>(FileContents + Index)};

        int32 Mask{Simd::MoveMask(VarNameVector == LoadedChars)};
        Mask &= ValidMask;

        if EXPECT(Mask == ValidMask, false)
        {
            return FileContents + (Index + VarNameLength + 2);
        }
    }
    return nullptr;
}

