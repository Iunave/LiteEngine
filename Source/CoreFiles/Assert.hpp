#pragma once

#include "Definitions.hpp"

#include <fmt/core.h>
#include <fmt/color.h>

#define PRINT_ASSERTION(reason)\
fmt::print(fmt::fg(fmt::color::dark_red), "\nassertion failed: {}\n\n{}\nin file: {}\nin line: {}\n", reason, __PRETTY_FUNCTION__, __FILE__, __LINE__);\

#define PRINT_ASSERTION_MSG(reason, message, ...) \
fmt::print(fmt::fg(fmt::color::dark_red), "\nassertion failed: {}\n" message "\n\n{}\nin file: {}\nin line: {}\n", reason __VA_OPT__(,) __VA_ARGS__, __PRETTY_FUNCTION__, __FILE__, __LINE__);\

#if DEBUG

//fatal assertion, evaluates expr in release builds
#define CHECK(expr, ...)\
[&](void) -> bool\
{\
    const bool __expr_value__{!!(expr)};\
    \
    if EXPECT(!__expr_value__, false)\
    {\
        PRINT_ASSERTION##__VA_OPT__(_MSG)(#expr __VA_OPT__(,) __VA_ARGS__); \
        CRASH_TRAP;\
    }\
\
    return __expr_value__;\
}()

//non-fatal assertion, evaluates expr in release builds
#define ENSURE(expr, ...) \
[&](void) -> bool\
{\
    const bool __expr_value__{!!(expr)};\
    \
    if EXPECT(!__expr_value__, false)\
    {\
        PRINT_ASSERTION##__VA_OPT__(_MSG)(#expr __VA_OPT__(,) __VA_ARGS__);\
    }\
\
    return __expr_value__;\
}()

//fatal assertion
#define ASSERT(expr, ...)\
[&](void) -> bool\
{\
    const bool __expr_value__{!!(expr)};\
    \
    if EXPECT(!__expr_value__, false)\
    {\
        PRINT_ASSERTION##__VA_OPT__(_MSG)(#expr __VA_OPT__(,) __VA_ARGS__); \
        CRASH_TRAP;\
    }\
\
    return __expr_value__;\
}()

#else

//fatal assertion, evaluates expr in release builds
#define CHECK(expr, ...) (expr)

//non-fatal assertion, evaluates expr in release builds
#define ENSURE(expr, ...) (expr)

//fatal assertion, does not evaluate expr in release builds
#define ASSERT(expr, ...)

#endif //DEBUG

//non-fatal assertion, works in release build
#define ENSURE_ALWAYS(expr, ...)\
[&](void) -> bool\
{\
    const bool __expr_value__{!!(expr)};\
    \
    if EXPECT(!__expr_value__, false)\
    {\
        PRINT_ASSERTION##__VA_OPT__(_MSG)(#expr __VA_OPT__(,) __VA_ARGS__);\
    }\
\
    return __expr_value__;\
}()

//fatal assertion, works in release build
#define ASSERT_ALWAYS(expr, ...)\
[&](void) -> bool\
{\
    const bool __expr_value__{!!(expr)};\
    \
    if EXPECT(!__expr_value__, false)\
    {\
        PRINT_ASSERTION##__VA_OPT__(_MSG)(#expr __VA_OPT__(,) __VA_ARGS__); \
        CRASH_TRAP;\
    }\
\
    return __expr_value__;\
}()
