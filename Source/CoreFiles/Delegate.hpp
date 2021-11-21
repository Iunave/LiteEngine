#pragma once

#include "Array.hpp"
#include "Simd.hpp"

template<typename ReturnType, typename... Parameters>
class TDelegate final
{
public:

    using FunctionType = ReturnType(*)(Parameters...);

    TDelegate() = default;
    ~TDelegate() = default;

    void Bind(FunctionType InFunction)
    {
        FunctionArray.Append(InFunction);
    }

    void UnBind(FunctionType InFunction)
    {
    #if defined(AVX512)
        using VectorType = uint64_8;
    #elif defined(AVX256)
        using VectorType = uint64_4;
    #endif

        const VectorType ComparisonPointers{Simd::SetAll<VectorType>(reinterpret_cast<uint64>(InFunction))};

        for(int64 Index{0}; Index < FunctionArray.Num(); Index += Simd::NumElements<VectorType>())
        {
            const VectorType FunctionPointers{Simd::LoadUnaligned<VectorType>(FunctionArray + Index)};
            const Simd::MaskType<VectorType> Mask{Simd::MoveMask(FunctionPointers == ComparisonPointers)};

            if(Mask != 0)
            {
                const int32 FoundIndex{Math::FindFirstSet(Mask) - 1};

                if(FunctionArray.IsIndexValid(FoundIndex))
                {
                    FunctionArray.RemoveAtSwap(FoundIndex);
                    return;
                }
            }
        }
    }

    template<typename... BroadcastParameters>
    void Broadcast(BroadcastParameters&&... Params)
    {
        for(FunctionType Function : FunctionArray)
        {
            Function(Params...);
        }
    }

private:

    TDynamicArray<FunctionType> FunctionArray;
};
