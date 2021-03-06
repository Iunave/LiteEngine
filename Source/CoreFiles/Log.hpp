#pragma once

#include "String.hpp"
#include "Thread/Thread.hpp"

#include <fmt/core.h>
#include <fmt/color.h>

struct FLogCategory
{
    template<uint64 NumChars>
    constexpr FLogCategory(const fmt::color InMessageColor, const char8 (&MessageString)[NumChars])
        : MessageColor{InMessageColor}
        , PreMessageString{MessageString}
    {
    }

    const fmt::color MessageColor;
    const FString<SS60> PreMessageString;
};

F2DCoordinate<int32> GetTerminalCursorPosition(); //todo does not even work

inline constexpr FLogCategory LogTemp{fmt::color::dark_gray, "Temp: "};
inline constexpr FLogCategory LogProgram{fmt::color::chocolate, "Program: "};
inline constexpr FLogCategory LogThread{fmt::color::dark_olive_green, "Thread: "};
inline constexpr FLogCategory LogMemory{fmt::color::golden_rod, "Memory: "};
inline constexpr FLogCategory LogGLFW{fmt::color::brown, "GLFW: "};
inline constexpr FLogCategory LogVulkan{fmt::color::medium_violet_red, "Vulkan: "};

#if DEBUG

//normal log
#define LOG(category, string, ...)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg((category).MessageColor), ((category).PreMessageString + string "\n").Data() __VA_OPT__(,) __VA_ARGS__);\
Thread::GLogMutex.Unlock()

#define LOG_UNGUARDED(category, string, ...)\
fmt::print(fmt::fg((category).MessageColor), ((category).PreMessageString + string "\n").Data() __VA_OPT__(,) __VA_ARGS__)

//logs a warning
#define LOG_WARNING(category, string, ...)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg(fmt::color::yellow), ("!!!" + category.PreMessageString + string "!!!\n").Data() __VA_OPT__(,) __VA_ARGS__);\
Thread::GLogMutex.Unlock()

#define LOG_WARNING_UNGUARDED(category, string, ...)\
fmt::print(fmt::fg(fmt::color::yellow), ("!!!" + category.PreMessageString + string "!!!\n").Data() __VA_OPT__(,) __VA_ARGS__)

//logs an error
#define LOG_ERROR(category, string, ...)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg(fmt::color::red), ("!!!" + category.PreMessageString + string "!!!\n").Data() __VA_OPT__(,) __VA_ARGS__);\
Thread::GLogMutex.Unlock()

#define LOG_ERROR_UNGUARDED(category, string, ...)\
fmt::print(fmt::fg(fmt::color::red), ("!!!" + category.PreMessageString + string "!!!\n").Data() __VA_OPT__(,) __VA_ARGS__)

#else

#define LOG(Category, string, ...)
#define LOG_UNGUARDED(category, string, ...)
#define LOG_WARNING(Category, string, ...)
#define LOG_WARNING_UNGUARDED(category, string, ...)
#define LOG_ERROR(Category, string, ...)
#define LOG_ERROR_UNGUARDED(category, string, ...)

#endif //DEBUG

//normal log
#define LOG_ALWAYS(category, string, ...)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg((category).MessageColor), ((category).PreMessageString + string "\n").Data() __VA_OPT__(,) __VA_ARGS__);\
Thread::GLogMutex.Unlock()

#define LOG_UNGUARDED_ALWAYS(category, string, ...)\
fmt::print(fmt::fg((category).MessageColor), ((category).PreMessageString + string "\n").Data() __VA_OPT__(,) __VA_ARGS__)

//logs a warning
#define LOG_WARNING_ALWAYS(category, string, ...)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg(fmt::color::yellow), ("!!!" + category.PreMessageString + string "!!!\n").Data() __VA_OPT__(,) __VA_ARGS__);\
Thread::GLogMutex.Unlock()

#define LOG_WARNING_UNGUARDED_ALWAYS(category, string, ...)\
fmt::print(fmt::fg(fmt::color::yellow), ("!!!" + category.PreMessageString + string "!!!\n").Data() __VA_OPT__(,) __VA_ARGS__)

//logs an error
#define LOG_ERROR_ALWAYS(category, string, ...)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg(fmt::color::red), ("!!!" + category.PreMessageString + string "!!!\n").Data() __VA_OPT__(,) __VA_ARGS__);\
Thread::GLogMutex.Unlock()

#define LOG_ERROR_UNGUARDED_ALWAYS(category, string, ...)\
fmt::print(fmt::fg(fmt::color::red), ("!!!" + category.PreMessageString + string "!!!\n").Data() __VA_OPT__(,) __VA_ARGS__)
