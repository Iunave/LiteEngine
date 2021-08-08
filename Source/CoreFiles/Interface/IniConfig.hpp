#pragma once

#include "Definitions.hpp"
#include "String.hpp"

#ifndef MAKE_INI_VARIABLE
#define MAKE_INI_VARIABLE(Type, Name, InitValue) ::TIniVariable<Type> Name{::FStaticString{#Name}, InitValue}
#endif

template<typename Type>
struct TIniVariable final
{
    TIniVariable() = default;
    ~TIniVariable() = default;

    inline explicit TIniVariable(FStaticString&& InVarName, const Type&& InValue)
        : Variable(Move(InValue))
        , VarName(InVarName)
    {
    }

    Type Variable;

    const FStaticString VarName;
};

class FIniConfig
{
public:

    FIniConfig();

    virtual ~FIniConfig();

protected:

};
