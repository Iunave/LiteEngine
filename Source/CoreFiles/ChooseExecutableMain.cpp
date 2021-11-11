#include "Definitions.hpp"
#include "String.hpp"
#include "Log.hpp"

int32 main(int32 NumArgs, const char8** Arguments)
{
    CPU_INITIALIZE;

    const char8* VersionString{nullptr};
    const char8* FamilyString{nullptr};

    for(int32 ArgumentIndex{0}; ArgumentIndex < NumArgs; ++ArgumentIndex)
    {
        if(Memory::Compare(Arguments[ArgumentIndex], "version=", sizeof("version=") - 1) == 0)
        {
            VersionString = Arguments[ArgumentIndex] + sizeof("version=") - 1;
        }
        else if(Memory::Compare(Arguments[ArgumentIndex], "target=", sizeof("target=") - 1) == 0)
        {
            FamilyString = Arguments[ArgumentIndex] + sizeof("target=") - 1;
        }
        else
        {
            LOGW_ALWAYS(LogProgram, "unsupported argument \"{}\"", Arguments[ArgumentIndex]);
        }
    }

    if(VersionString == nullptr)
    {
        VersionString = "release";
    }

    if(FamilyString == nullptr)
    {
        if(IS_CPU_FAMILY(skylake))
        {
            FamilyString = "skylake";
        }
        else if(IS_CPU_FAMILY(skylake-avx512))
        {
            FamilyString = "skylake-avx512";
        }
        else if(IS_CPU_FAMILY(cannonlake))
        {
            FamilyString = "cannonlake";
        }
        else if(IS_CPU_FAMILY(cascadelake))
        {
            FamilyString = "cascadelake";
        }
        else if(IS_CPU_FAMILY(tigerlake))
        {
            FamilyString = "tigerlake";
        }
        else if(IS_CPU_FAMILY(cooperlake))
        {
            FamilyString = "cooperlake";
        }
        else if(IS_CPU_FAMILY(alderlake))
        {
            FamilyString = "alderlake";
        }
        else if(IS_CPU_FAMILY(znver1))
        {
            FamilyString = "znver1";
        }
        else if(IS_CPU_FAMILY(znver2))
        {
            FamilyString = "znver2";
        }
        else if(IS_CPU_FAMILY(znver3))
        {
            FamilyString = "znver3";
        }
        else
        {
            ASSERT_ALWAYS(false, "cpu is not supported");
        }
    }

    FString<SS124> ExecutableToLaunch{"./"};
    ExecutableToLaunch += PROJECT_NAME;
    ExecutableToLaunch += "_";
    ExecutableToLaunch.Concat_Assign(FamilyString, StrUtl::Length(FamilyString));
    ExecutableToLaunch += "_";
    ExecutableToLaunch.Concat_Assign(VersionString, StrUtl::Length(VersionString));
    ExecutableToLaunch += ".out";

    LOG_ALWAYS(LogProgram, "running {}", ExecutableToLaunch.Data() + 2);

    return system(ExecutableToLaunch.Data());
}

