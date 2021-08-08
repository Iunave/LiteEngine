#pragma once

#include "Array.hpp"
#include "Function.hpp"

template<typename ReturnType, typename... Parameters>
class TDelegate final
{
public:

    TDelegate() = default;
    ~TDelegate() = default;

    void Bind(TFunction<ReturnType(Parameters...)> InFunction)
    {
        FunctionArray.Append(InFunction);
    }

    void UnBind(TFunction<ReturnType(Parameters...)> InFunction)
    {
        const int64 FoundIndex{FunctionArray.FindIndex(InFunction)};

        if(FoundIndex != INDEX_NONE)
        {
            FunctionArray.RemoveAtSwap(FoundIndex);
        }
    }

    void Broadcast(Parameters&&... BroadcastParameters)
    {
        for(TFunction<ReturnType(Parameters...)>& Function : FunctionArray)
        {
            Function(BroadcastParameters...);
        }
    }

private:

    TDynamicArray<TFunction<ReturnType(Parameters...)>> FunctionArray;

};
