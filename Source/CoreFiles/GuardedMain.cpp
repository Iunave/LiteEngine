#include <atomic>
#include "Definitions.hpp"
#include "Log.hpp"
#include "Time.hpp"
#include "Object/Object.hpp"
#include "Rendering/VulkanRenderer.hpp"

inline float64 ProgramStartTime;
inline float64 ProgramEndTime;

void ChangeWorkingDirectoryToSource()
{
    ::chdir("../..");
}

CONSTRUCTOR(1) void OnProgramStart()
{
    ProgramStartTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "starting at {}", ProgramStartTime);

    ChangeWorkingDirectoryToSource();
    Thread::MainThreadID = pthread_self();
}

DESTRUCTOR(100) void OnProgramEnd()
{
    ProgramEndTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "ending at {}", ProgramEndTime);
    LOG_UNGUARDED(LogProgram, "total time in execution: {} seconds", ProgramEndTime - ProgramStartTime);
}

OBJECT_CLASS(OFoo)
class OFoo : public OObject
{
    OBJECT_BASES(OObject)
public:

};

template<int32... I>
concept ValidSelectElementsChoice = requires()
{
    {(((I | 1) == 1) && ...)};
};

static_assert(ValidSelectElementsChoice<1, 0>); //ok
static_assert(ValidSelectElementsChoice<1, 2>); //no

int32 main()
{
    TSharedPtr<OFoo> A{MakeShared<OFoo>()};
    TSharedPtr<OFoo> B{MakeShared<OFoo>()};
    TSharedPtr<OFoo> C{MakeShared<OFoo>()};
    TSharedPtr<OFoo> D{MakeShared<OFoo>()};

    for(TObjectIterator<OFoo> Iterator{}; Iterator; ++Iterator)
    {
        LOG(LogProgram, "{}", Iterator->GetClassName());
    }

    Render::Initialize();
    Render::Loop();
    Render::Shutdown();

    return 0;
}

