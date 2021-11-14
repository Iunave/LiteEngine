#include "Definitions.hpp"
#include "Object/Object.hpp"
#include "Log.hpp"
#include "Rendering/VulkanRenderer.hpp"
#include "Time.hpp"


OBJECT_CLASS(OFoo)
class OFoo : public OObject
{
    OBJECT_BASES(OObject)
public:
    int32 lol{43214};
};

OBJECT_CLASS(OBar)
class OBar : public OFoo
{
    OBJECT_BASES(OFoo)
public:
    int32 lol{43214};
    uint64 wow[428]{};
};

inline float64 ProgramStartTime;
inline float64 ProgramEndTime;

CONSTRUCTOR(1) inline void OnProgramStart()
{
    ProgramStartTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "starting at {} seconds", ProgramStartTime);
}

DESTRUCTOR(100) inline void OnProgramEnd()
{
    ProgramEndTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "ending at {} second", ProgramEndTime);
    LOG_UNGUARDED(LogProgram, "total time in execution: {} seconds", ProgramEndTime - ProgramStartTime);
}

void InitializeVariables()
{
    Thread::MainThreadID = pthread_self();
}

struct A
{
    union UCharacters
    {
        char Stack[60];
        char* Heap;
    };
    UCharacters Characters;
    uint32 End;
};

struct __attribute__((packed)) B
{
    union UCharacters
    {
        char Stack[60];
        char* Heap;
    };
    UCharacters Characters;
    uint32 End;
};

struct __attribute__((packed)) alignas(64) C
{
    union UCharacters
    {
        char Stack[60];
        char* Heap;
    };
    UCharacters Characters;
    uint32 End;
};

struct D
{
    union __attribute__((packed)) UCharacters
    {
        char Stack[60];
        char* Heap;
    };
    UCharacters Characters;
    uint32 End;
};

struct __attribute__((packed)) E
{
    union __attribute__((packed)) UCharacters
    {
        char Stack[60];
        char* Heap;
    };
    UCharacters Characters;
    uint32 End;
};

struct __attribute__((packed)) alignas(64) F
{
    union __attribute__((packed)) UCharacters
    {
        char Stack[60];
        char* Heap;
    };
    UCharacters Characters;
    uint32 End;
};

int32 main()
{
    InitializeVariables();

    __builtin_printf("alignof A %lu, sizeof A %lu\n", alignof(A), sizeof(A));
    __builtin_printf("alignof B %lu, sizeof B %lu\n", alignof(B), sizeof(B));
    __builtin_printf("alignof C %lu, sizeof C %lu\n", alignof(C), sizeof(C));
    __builtin_printf("alignof D %lu, sizeof D %lu\n", alignof(D), sizeof(D));
    __builtin_printf("alignof E %lu, sizeof E %lu\n", alignof(E), sizeof(E));
    __builtin_printf("alignof F %lu, sizeof F %lu\n", alignof(F), sizeof(F));

    TSharedPtr<OObject> Obj1{MakeShared<OFoo>()};
    TSharedPtr<OObject> Obj2{MakeShared<OObject>()};
    TSharedPtr<OObject> Obj3{MakeShared<OBar>()};
    TSharedPtr<OObject> Obj4{MakeShared<OFoo>()};
    TSharedPtr<OObject> Obj5{MakeShared<OBar>()};
    TSharedPtr<OObject> Obj6{MakeShared<OBar>()};
    TSharedPtr<OObject> Obj7{MakeShared<OBar>()};
    TSharedPtr<OObject> Obj8{MakeShared<OBar>()};
    TSharedPtr<OObject> Obj9{MakeShared<OBar>()};
    TSharedPtr<OObject> Obj10{MakeShared<OBar>()};

    for(TObjectIterator<OObject> Iterator{}; Iterator; ++Iterator)
    {
        LOG(LogProgram, "{}", Iterator->GetClassName());
    }

    Obj1.Reset();
    Obj4.Reset();
    Obj5.Reset();

    for(TObjectIterator<OObject> Iterator{}; Iterator; ++Iterator)
    {
        LOG(LogProgram, "{}", Iterator->GetClassName());
    }

    Render::Initialize();
    Render::Loop();
    Render::ShutDown();

    return 0;
}
