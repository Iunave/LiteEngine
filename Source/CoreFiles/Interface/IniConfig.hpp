#pragma once

#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/String.hpp"
#include "Object/Object.hpp"
#include <fmt/core.h>
#include <fmt/os.h>

template<typename Type>
struct TIniVariable final
{
    TIniVariable() = default;
    ~TIniVariable() = default;

    inline explicit TIniVariable(FStaticString InVarName, const Type&& InValue)
        : Variable{Move(InValue)}
        , VarName{InVarName}
    {
    }

    Type Variable;

    const FStaticString VarName;
};

OBJECT_CLASS(IIniConfig)
class IIniConfig : public OObject
{
    OBJECT_BASES(OObject)
public:

    IIniConfig();
    virtual ~IIniConfig();

    virtual void ReadConfig() = 0;
    virtual void WriteConfig() = 0;

protected:

    template<typename... Ts>
    void SaveConfigVariables(const TIniVariable<Ts>... Variables)
    {
        TDynamicArray<char8> FileContents{LoadFileForWrite()};

        (FindValueStart(FileContents, Variables.VarName), ...);
    }

    template<typename... Ts>
    void ReadConfigVariables(const TIniVariable<Ts>... Variables)
    {
        TDynamicArray<char8> FileContents{LoadFileForRead()};

        (FindValueStart(FileContents, Variables.VarName), ...);
    }

    static char8* FindValueStart(TDynamicArray<char8>& FileContents, const FStaticString VariableName);

    TDynamicArray<char8> LoadFileForRead() const;
    TDynamicArray<char8> LoadFileForWrite() const;

    FString<SS124> ConfigFilePath;
};
