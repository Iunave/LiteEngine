#pragma once

namespace vk
{
}

namespace Vk = vk;

namespace SoLoud
{
}

namespace Sld = SoLoud;

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

static_assert(sizeof(char8) == 1);
static_assert(sizeof(char16) == 2);
static_assert(sizeof(char32) == 4);
static_assert(sizeof(int8) == 1);
static_assert(sizeof(uint8) == 1);
static_assert(sizeof(int16) == 2);
static_assert(sizeof(uint16) == 2);
static_assert(sizeof(int32) == 4);
static_assert(sizeof(uint32) == 4);
static_assert(sizeof(int64) == 8);
static_assert(sizeof(uint64) == 8);
static_assert(sizeof(int128) == 16);
static_assert(sizeof(uint128) == 16);
static_assert(sizeof(float32) == 4);
static_assert(sizeof(float64) == 8);
static_assert(sizeof(float128) == 16);

#ifndef DEBUG
#define DEBUG false
#endif

#ifndef UNITY_BUILD
#define UNITY_BUILD false
#endif

#ifndef USE_VULKAN_VALIDATION_LAYERS
#define USE_VULKAN_VALIDATION_LAYERS false
#endif

#if UNITY_BUILD
#define UINLINE __attribute__((always_inline)) //when in unity mode we can force-inline functions defined in the translation unit
#else
#define UINLINE //does nothing when not in unity mode
#endif

#define implicit //be explicit about implicit
#define INDEX_NONE (-1)
#define PACKED __attribute__((__packed__))
#define MAY_ALIAS __attribute__((__may_alias__))
#define HAS_BUILTIN(builtin) __has_builtin(builtin)
#define EMPTY_STATEMENT void()
#define ATTRINLINE __attribute__((always_inline))
#define INLINE inline ATTRINLINE
#define NOINLINE __attribute__((noinline))
#define FLATTEN __attribute__((flatten))
#define ASSUME(expr) __builtin_assume(expr)
#define FUNC_ASSUME(...) __attribute__((assume(__VA_ARGS__)))
#define EXPECT(cond, tf) (__builtin_expect((cond), tf))
#define PURE __attribute__((pure)) //function may access global variables
#define CONST __attribute__((const)) //function may not access global variables
#define NONNULL __attribute__((nonnull))
#define UNREACHABLE __builtin_unreachable()
#define NODISCARD [[nodiscard]]
#define SMALL_DOUBLE 0.0001
#define RESTRICT __restrict__
#define NORETURN __attribute__((noreturn))
#define ALLOC_ALIGN(align) __attribute__((alloc_align(align))
#define ALLOC_SIZE(n) __attribute__((alloc_size(n))
#define ALLOC_SIZE2(n, m) __attribute__((alloc_size(n, m))
#define CALLBACK(name, ...) __attribute__((callback(name, __VA_ARGS__)))
#define CONVERGENT __attribute__((convergent))
#define INTERNAL_LINKAGE __attribute__((internal_linkage))
#define LIFETIME_BOUND [[clang::lifetimebound]]
#define MIN_VECTOR_WIDTH(width) __attribute__((min_vector_width(width)))
#define ASSUME_ALIGNED(address, alignment) __builtin_assume_aligned(address, alignment)
#define CLEAR_PADDING(address) __builtin_clear_padding(address)
#define CRASH_TRAP __builtin_trap()

#define STACK_ALLOCATE(size) __builtin_alloca(size)

#define CHOOSE_EXPR(cond, truecase, falsecase) __builtin_choose_expr(cond, truecase, falsecase)

#ifndef NULL_HANDLE
#define NULL_HANDLE nullptr
#endif

#define STRINGIFY(arg) #arg

#define FIRST_ARG(first, ...) first
#define STRINGIFY_FIRST_ARG(first, ...) #first

#define NUM_ARGS(...) __VA_OPT__(PP_NARG_(__VA_ARGS__,PP_RSEQ_N())) 0
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

enum class ENoInit : uint8
{
    NoInit
};

enum class EInit : uint8
{
    Zero,
    Default
};

enum class EChooseConstructor : uint8
{
};

template<typename... T>
extern void __SideEffect__(T...);

template<typename T>
INLINE void DoNotOptimize(T& value)
{
    asm volatile("" : "+r,m"(value) : : "memory");
}

class FNonCopyable
{
public:

    FNonCopyable() = default;
    virtual ~FNonCopyable() = default;

    FNonCopyable(const FNonCopyable&) = delete;
    FNonCopyable& operator=(const FNonCopyable&) = delete;

};

template<typename DerivedClass, auto... InitializerArguments>
class FSingleton
{
public:

    FSingleton() = default;
    virtual ~FSingleton() = default;

    inline static DerivedClass& Instance()
    {
        static DerivedClass Derived{InitializerArguments...};
        return Derived;
    }

};

template<typename DerivedClass, auto... InitializerArguments>
class FConstexprSingleton
{
public:

    FConstexprSingleton() = default;
    virtual ~FConstexprSingleton() = default;

    inline static DerivedClass& Instance()
    {
        static constexpr DerivedClass Derived{InitializerArguments...};
        return Derived;
    }

};

template<typename CoordinateType>
struct F2DCoordinate
{
    CoordinateType X;
    CoordinateType Y;
};

template<typename CoordinateType>
struct F3DCoordinate
{
    CoordinateType X;
    CoordinateType Y;
    CoordinateType Z;
};
