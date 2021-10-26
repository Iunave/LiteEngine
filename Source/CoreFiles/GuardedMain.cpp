
#include <Interface/Damageable.hpp>
#include <Interface/IniConfig.hpp>
#include <Actor/BuildComponent/BuildComponent.hpp>
#include <Actor/Robot.hpp>
#include "Object/Allocator.hpp"

#include "Definitions.hpp"
#include "Object/Object.hpp"
#include "SmartPointer.hpp"
#include "Interface/Tick.hpp"
#include "Array.hpp"
#include "Vector.hpp"
#include "Log.hpp"
#include "Quaternion.hpp"
#include "Rendering/VulkanRenderer.hpp"


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

void InitializeVariables()
{
    Thread::MainThreadID = pthread_self();
}

int32 main()
{
    LOG(LogProgram, "running: {}", EXECUTABLE_NAME);
    LOG(LogProgram, "entered main with thread-ID: {}", (void*)pthread_self());

    InitializeVariables();

    const char8* String{"-845703.42"};

    float64 Val = StrUtl::ToValue<float64>(String, String + 9);
    LOG(LogProgram, "{}", Val);

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

    //Obj1.Reset();
    //Obj4.Reset();
    //Obj5.Reset();

    LOG(LogProgram, "\n");

    for(TObjectIterator<OObject> Iterator{}; Iterator; ++Iterator)
    {
        LOG(LogProgram, "{}", Iterator->GetClassName());
    }

    Render::Initialize();
    Render::Loop();
    Render::ShutDown();

    return 0;
}
