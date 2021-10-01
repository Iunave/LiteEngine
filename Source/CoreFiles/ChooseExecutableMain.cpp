#include <cstdlib>
#include <stdio.h>
#include <chrono>
#include <string>

using char8 = char;
using char16 = char16_t;
using char32 = char32_t;
using uint8 = unsigned char;
using int8 = signed char;
using uint16 = unsigned short;
using int16 = signed short;
using uint32 = unsigned int;
using int32 = signed int;
using uint64 = unsigned long long;
using int64 = signed long long;
using int128 = __int128_t;
using uint128 = __uint128_t;
using float32 = float;
using float64 = double;
using float128 = long double;
using byte = unsigned char;

int32 main(int NumArgs, const char** Arguments)
{
    __builtin_cpu_init();

    std::string ExecutableName{"./"};
    ExecutableName += PROJECT_NAME;

    int32 NumProgramRuns{1};

    bool bAvxExplicitlySpecified{false};
    const char8* AvxVersionString{nullptr};

    bool bDebugProgram{false};

    for(int ArgumentIndex{0}; ArgumentIndex < NumArgs; ++ArgumentIndex)
    {
        if(__builtin_memcmp(Arguments[ArgumentIndex], "benchmark=", 10) == 0)
        {
            sscanf(&Arguments[ArgumentIndex][10], "%i", &NumProgramRuns);
        }
        else if(__builtin_memcmp(Arguments[ArgumentIndex], "debug", 5) == 0)
        {
            bDebugProgram = true;
        }
        else if(__builtin_memcmp(Arguments[ArgumentIndex], "avx", 3) == 0)
        {
            bAvxExplicitlySpecified = true;
            AvxVersionString = Arguments[ArgumentIndex];
        }
    }

    if(!bAvxExplicitlySpecified)
    {
        if(__builtin_cpu_supports("avx512f"))
        {
            ExecutableName += "_avx512";
        }
        else if(__builtin_cpu_supports("avx2"))
        {
            ExecutableName += "_avx256";
        }
    }
    else
    {
        ExecutableName += "_";
        ExecutableName += AvxVersionString;
    }

    if(bDebugProgram)
    {
        ExecutableName += "_debug";
    }

    auto StartTime = std::chrono::high_resolution_clock::now();

    while(NumProgramRuns != 0)
    {
        system(ExecutableName.c_str());
        --NumProgramRuns;
    }

    auto EndTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float64> DeltaTime{EndTime - StartTime};

    __builtin_printf("execution time: %f", DeltaTime.count());

    return 0;
}

