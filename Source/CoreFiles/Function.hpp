#pragma once

#include "Definitions.hpp"
#include "TypeTraits.hpp"
#include "Tuple.hpp"

template<typename LambdaSignature>
class alignas(LambdaSignature) TFunction final
{
public:

    inline explicit TFunction(LambdaSignature&& InLambda)
        : Lambda(Move(InLambda))
    {
    }

    inline TFunction(TFunction&& Other)
        : Lambda(Move(Other.Lambda))
    {
    }

    inline TFunction& operator=(TFunction&& Other)
    {
        Lambda = Move(Other.Lambda);
        return *this;
    }

    inline TFunction& operator=(LambdaSignature&& InLambda)
    {
        Lambda = Move(InLambda);
        return *this;
    }

    template<typename... VarArgs>
    inline decltype(auto) operator()(VarArgs&&... Args) const
    {
        ASSERT(IsValid());

        return Lambda(Move(Args)...);
    }

    inline bool IsValid() const
    {
        return true;
    }

private:

    LambdaSignature Lambda;

};

template<typename LambdaSignature>
class alignas(8) TFunction<LambdaSignature*> final
{
public:

    inline explicit TFunction(LambdaSignature InLambda = nullptr)
        : LambdaPointer(InLambda)
    {
    }

    inline TFunction(const TFunction& Other)
        : LambdaPointer(Other.LambdaPointer)
    {
    }

    inline TFunction& operator=(const TFunction& Other)
    {
        LambdaPointer = Other.LambdaPointer;
        return *this;
    }

    inline TFunction& operator=(LambdaSignature InLambda)
    {
        LambdaPointer = InLambda;
        return *this;
    }

    template<typename... VarArgs>
    inline decltype(auto) operator()(VarArgs&&... Args) const
    {
        ASSERT(IsValid());

        return (*LambdaPointer)(Move(Args)...);
    }

    inline bool IsValid() const
    {
        return LambdaPointer != nullptr;
    }

private:

    LambdaSignature LambdaPointer;

};

template<typename LambdaSignature>
TFunction(LambdaSignature)->TFunction<LambdaSignature>;

template<typename LambdaSignature>
TFunction(const TFunction<LambdaSignature>&)->TFunction<LambdaSignature>;

template<typename LambdaSignature>
TFunction(TFunction<LambdaSignature>&&)->TFunction<LambdaSignature>;

template<typename ReturnType, typename... ParameterTypes>
class alignas(8) TFunction<ReturnType(ParameterTypes...)> final
{
public:

    using Signature = ReturnType(*)(ParameterTypes...);

    inline TFunction(Signature InFunctionPointer = nullptr)
        : FunctionPointer(InFunctionPointer)
    {
    }

    inline TFunction(const TFunction& Other)
        : FunctionPointer(Other.FunctionPointer)
    {
    }

    inline TFunction& operator=(const TFunction& Other)
    {
        FunctionPointer = Other.FunctionPointer;
        return *this;
    }

    inline TFunction& operator=(Signature NewFunction)
    {
        FunctionPointer = NewFunction;
        return *this;
    }

    inline ReturnType operator()(ParameterTypes&&... Args) const
    {
        ASSERT(IsValid());

        return FunctionPointer(Move(Args)...);
    }

    inline bool IsValid() const
    {
        return FunctionPointer != nullptr;
    }

private:

    Signature FunctionPointer;

};

template<typename ReturnType, typename... ParameterTypes>
TFunction(ReturnType(ParameterTypes...))->TFunction<ReturnType(ParameterTypes...)>;

template<typename ReturnType, typename... ParameterTypes>
TFunction(const TFunction<ReturnType(ParameterTypes...)>&)->TFunction<ReturnType(ParameterTypes...)>;

template<typename ReturnType, typename OwnerType, typename... ParameterTypes>
class alignas(16) TFunction<ReturnType(OwnerType::*)(ParameterTypes...)> final
{
public:

    using Signature = ReturnType(OwnerType::*)(ParameterTypes...);

    inline TFunction()
        : Functor(nullptr, nullptr)
    {
    }

    inline TFunction(const TFunction& Other)
        : Functor(Other.Functor)
    {
    }

    inline explicit TFunction(Signature InFunctionPointer, OwnerType* InFunctionOwner)
        : Functor(InFunctionPointer, InFunctionOwner)
    {
    }

    inline ReturnType operator()(ParameterTypes&&... Args) const
    {
        ASSERT(IsValid());

        return (Functor.FunctionOwner->*Functor.FunctionPointer)(Move(Args)...);
    }

    inline TFunction& operator=(const TFunction& Other)
    {
        return Assign(Other.Functor.FunctionPointer, Other.Functor.FunctionOwner);
    }

    inline TFunction& Set(Signature InFunctionPointer)
    {
        return Assign(InFunctionPointer, Functor.FunctionOwner);
    }

    inline TFunction& Set(Signature InFunctionPointer, OwnerType* InFunctionOwner)
    {
        return Assign(InFunctionPointer, InFunctionOwner);
    }

    inline bool IsValid() const
    {
        return Functor.FunctionOwner != nullptr && Functor.FunctionPointer != nullptr;
    }

private:

    inline TFunction& Assign(Signature InFunctionPointer, OwnerType* InFunctionOwner)
    {
        Functor.FunctionPointer = InFunctionPointer;
        Functor.FunctionOwner = InFunctionOwner;
        return *this;
    }

    struct FLambda final
    {
        inline FLambda(Signature InFunctionPointer, OwnerType* InFunctionOwner)
            : FunctionPointer(InFunctionPointer)
            , FunctionOwner(InFunctionOwner)
        {
        }

        Signature FunctionPointer;

        OwnerType* FunctionOwner;
    };

    FLambda Functor;

};

template<typename ReturnType, typename... ParameterTypes>
class TStorableFunction;

template<typename ReturnType, typename... ParameterTypes>
class TStorableFunction<ReturnType(ParameterTypes...)> final
{
public:

    using Signature = ReturnType(*)(ParameterTypes...);

    template<typename... VarArgs>
    inline TStorableFunction(Signature InFunctionPointer, VarArgs&&... Args)
        : FunctionPointer(InFunctionPointer)
        , Arguments(MoveIfPossible(Args)...)
    {
    }

    inline TStorableFunction(Signature InFunctionPointer, TTuple<ParameterTypes...>&& Args)
        : FunctionPointer(InFunctionPointer)
        , Arguments(Move(Args))
    {
    }

    inline TStorableFunction(const TStorableFunction& Other)
        : FunctionPointer(Other.FunctionPointer)
        , Arguments(Other.Arguments)
    {
    }

    inline TStorableFunction& operator=(const TStorableFunction& Other)
    {
        FunctionPointer = Other.FunctionPointer;
        Arguments = Other.Arguments;
        return *this;
    }

    inline TStorableFunction(TStorableFunction&& Other)
        : FunctionPointer(Move(Other.FunctionPointer))
        , Arguments(Move(Other.Arguments))
    {
    }

    inline TStorableFunction& operator=(TStorableFunction&& Other)
    {
        FunctionPointer = Move(Other.FunctionPointer);
        Arguments = Move(Other.Arguments);
        return *this;
    }

    template<uint64... I>
    inline ReturnType Invoke() const
    {
        return FunctionPointer(Arguments.template Get<I>()...);
    }

    inline ReturnType operator()() const
    {
        ASSERT(IsValid());

        auto Invoke = [this]<uint64... I>(TypeTrait::IndexSequence<I...>) -> ReturnType
        {
            return (*FunctionPointer)((Arguments.template Get<I>())...);
        };

        return Invoke(TypeTrait::MakeIndexSequence<sizeof...(ParameterTypes)>{});
    }

    inline bool IsValid() const
    {
        return FunctionPointer != nullptr;
    }

private:

    Signature FunctionPointer;

    TTuple<ParameterTypes...> Arguments;

};

template<typename ReturnType, typename OwnerType, typename... ParameterTypes>
TFunction(ReturnType(OwnerType::*)(ParameterTypes...), OwnerType*)->TFunction<ReturnType(OwnerType::*)(ParameterTypes...)>;

template<typename ReturnType, typename... ParameterTypes>
TFunction(ReturnType(*)(ParameterTypes...))->TFunction<ReturnType(*)(ParameterTypes...)>;

template<typename ReturnType, typename OwnerType, typename... ParameterTypes>
TFunction(const TFunction<ReturnType(OwnerType::*)(ParameterTypes...)>&)->TFunction<ReturnType(OwnerType::*)(ParameterTypes...)>;

template<typename ReturnType, typename... ParameterTypes>
TStorableFunction(ReturnType(*)(ParameterTypes...), ParameterTypes...)->TStorableFunction<ReturnType(ParameterTypes...)>;
