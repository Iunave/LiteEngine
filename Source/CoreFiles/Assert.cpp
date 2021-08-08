#include "Assert.hpp"
#include <csignal>

#ifdef __unix__

#include "Array.hpp"
#include "Memory.hpp"
#include "String.hpp"
#include <execinfo.h>
#include <stdio.h>
#include <regex>
#include <unistd.h>
#include <zconf.h>

std::string GetExecutionPath()
{
    TStaticArray<char8, PATH_MAX> Buffer;

    int64 Count{readlink("/proc/self/exe", Buffer.GetData(), PATH_MAX)};

    return std::string{Buffer.GetData(), (Count > 0) ? static_cast<uint64>(Count) : 0ULL};
}

std::string BashCommand(const std::string& Command)
{
    TStaticArray<char8, 1024> Buffer;

    std::string Result;

    std::shared_ptr<FILE> pipe(popen(Command.c_str(), "r"), pclose);

    while(!feof(pipe.get()))
    {
        if(fgets(Buffer.GetData(), 128, pipe.get()) != nullptr)
        {
            Result += Buffer.GetData();
        }
    }
    return Result;
}


void PrintStackTrace()
{
    TStaticArray<void*, 1024> BacktraceBuffer;

    const int32 BacktraceSize{backtrace(BacktraceBuffer.GetData(), BacktraceBuffer.Num())};
    char8** BacktraceSymbols{backtrace_symbols(BacktraceBuffer.GetData(), BacktraceSize)};

    std::regex Regex1{"\\[(.+)\\]"};
    std::regex Regex2{"\\n$"};

    std::smatch Smatch;

    std::string ExecutionPath{GetExecutionPath()};

    for(int32 Index{1}; Index < BacktraceSize; Index++)
    {
        std::string Symbol{BacktraceSymbols[Index]};

        if(std::regex_search(Symbol, Smatch, Regex1))
        {
            std::string Address{Smatch[1]};
            std::string Command{"addr2line -e " + ExecutionPath + " -f -C " + Address};

            std::string AddressLine{BashCommand(Command)};
            std::string AddressLineReplaced{std::regex_replace(AddressLine, Regex2, "")};

            fmt::print("\n{}", AddressLineReplaced);
        }
    }

    free(BacktraceSymbols);
}
#endif

void AssertInternal::Crash()
{
#ifdef __unix__
    PrintStackTrace();
#endif
    std::raise(SIGTERM);
}

