
#include <Interface/Damageable.hpp>
#include <Interface/IniConfig.hpp>
#include <Actor/BuildComponent/BuildComponent.hpp>
#include <Actor/Robot.hpp>

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
class OFoo : public OTickable
{
    OBJECT_BASES(OTickable)
public:

    virtual void Tick(float64 DeltaTime) override
    {
    }

    int32 lol{43214};

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

    const char8* String{"-84003"};

    int64 Val = StrUtl::ToValue<int64>(String, String + 5);
    LOG(LogProgram, "{}", Val);

    Render::Initialize();
    Render::Loop();
    Render::ShutDown();

    return 0;
}
