#pragma once

#include "Definitions.hpp"
#include "Thread/Thread.hpp"

#include <fmt/core.h>
#include <fmt/color.h>

namespace AssertInternal
{
    void Crash();
}

#define PRINT_ASSERTION(reason)\
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg(fmt::color::dark_red), "\nassertion failed: {}\n\n{}\nin file: {}\nin line: {}\n", reason, __PRETTY_FUNCTION__, __FILE__, __LINE__);\
Thread::GLogMutex.Unlock()

#define PRINT_ASSERTION_MSG(reason, message, ...) \
Thread::GLogMutex.Lock();\
fmt::print(fmt::fg(fmt::color::dark_red), "\nassertion failed: {}\n" message "\n\n{}\nin file: {}\nin line: {}\n", reason __VA_OPT__(,) __VA_ARGS__, __PRETTY_FUNCTION__, __FILE__, __LINE__);\
Thread::GLogMutex.Unlock()
#if DEBUG

//fatal assertion, evaluates expr in release builds
#define CHECK(expr, ...) \
do \
{ \
    if EXPECT(!(expr), false) \
    { \
        PRINT_ASSERTION##__VA_OPT__(_MSG)(#expr __VA_OPT__(,) __VA_ARGS__); \
        CRASH_TRAP; \
    } \
} \
while(false)

//non-fatal assertion
#define ENSURE(expr, ...) \
do \
{ \
    if EXPECT(!(expr), false) \
    { \
        PRINT_ASSERTION##__VA_OPT__(_MSG)(#expr __VA_OPT__(,) __VA_ARGS__); \
    } \
} \
while(false)

//fatal assertion
#define ASSERT(expr, ...) \
do \
{ \
    if EXPECT(!(expr), false) \
    { \
        PRINT_ASSERTION##__VA_OPT__(_MSG)(#expr __VA_OPT__(,) __VA_ARGS__); \
        CRASH_TRAP; \
    } \
} \
while(false)

#else

//fatal assertion, evaluates expr in release builds
#define CHECK(expr, ...) do { expr; } while(false)

//non-fatal assertion
#define ENSURE(expr, ...)

//fatal assertion
#define ASSERT(expr, ...)

#endif //DEBUG
