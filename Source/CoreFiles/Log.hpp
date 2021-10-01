#pragma once

#include "String.hpp"
#include "Thread/Thread.hpp"

#include <fmt/core.h>
#include <fmt/color.h>

struct TLogCategory
{
    template<uint64 NumChars>
    constexpr TLogCategory(const fmt::color InMessageColor, const char8 (&MessageString)[NumChars])
        : MessageColor{InMessageColor}
        , PreMessageString{MessageString}
    {
    }

    const fmt::color MessageColor;
    const FString<ss60> PreMessageString;
};

F2DCoordinate<int32> GetTerminalCursorPosition(); //todo does not even work

#if DEBUG

inline constexpr TLogCategory LogTemp{fmt::color::dark_gray, "Temp: "};
inline constexpr TLogCategory LogProgram{fmt::color::chocolate, "Program: "};
inline constexpr TLogCategory LogThread{fmt::color::dark_olive_green, "Thread: "};
inline constexpr TLogCategory LogMemory{fmt::color::golden_rod, "Memory: "};
inline constexpr TLogCategory LogGLFW{fmt::color::brown, "GLFW: "};
inline constexpr TLogCategory LogVulkan{fmt::color::medium_violet_red, "Vulkan: "};

//normal log
#define LOG(category, string, ...)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg((category).MessageColor), ((category).PreMessageString + string "\n").Data() __VA_OPT__(,) __VA_ARGS__);\
Thread::GLogMutex.Unlock()

//logs a warning
#define LOGW(category, string, ...)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg(fmt::color::yellow), ("!!!" + category.PreMessageString + string "!!!\n").Data() __VA_OPT__(,) __VA_ARGS__);\
Thread::GLogMutex.Unlock()

//logs an error
#define LOGE(category, string, ...)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg(fmt::color::red), ("!!!" + category.PreMessageString + string "!!!\n").Data() __VA_OPT__(,) __VA_ARGS__);\
Thread::GLogMutex.Unlock()

//rewrites the line todo need to find a way to save the cursor position
#define LOGR(category, string, ...)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg((category).MessageColor), ("\r" + (category).PreMessageString + string).Data() __VA_OPT__(,) __VA_ARGS__);\
Thread::GLogMutex.Unlock()

#else

#define LOG(Category, string, ...)
#define LOGW(Category, string, ...)
#define LOGE(Category, string, ...)
#define LOGR(category, string, ...)

#endif //DEBUG
