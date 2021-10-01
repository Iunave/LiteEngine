
#include <Interface/Damageable.hpp>
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


OBJECT_CLASS(OFoo) : public OTickable
{
    OBJECT_BASES(OTickable)
public:

    virtual void Tick(float64 DeltaTime) override
    {
    }

    int32 lol{43214};

};

int32 main()
{
    LOG(LogProgram, "running: {}", EXECUTABLE_NAME);
    LOG(LogProgram, "entered main with thread-ID: {}", (void*)pthread_self());

    TSharedPtr<OTickable> Foo{MakeShared<OFoo>()};
    TSharedPtr<OFoo> Tickable{ObjectCast<OFoo*>(Foo)};
    LOG(LogProgram, "{}", Tickable->lol);

    FColor Color1{0.3, 0.6, 1.6, 0.04};
    LOG(LogProgram, "{}", StrUtil::ToString(Color1));
    RGBA16I Color2{static_cast<RGBA16I>(Color1)};
    LOG(LogProgram, "{} {} {} {}", Color2.R, Color2.G, Color2.B, Color2.A);

    TCountedStaticArray<int32, 10> Array{1, 3, 5, 5, 2, 0, 3, 1, 3, 5};

    ArrUtil::RemoveDuplicates(Array);
    LOG(LogProgram, "num {}", Array.Num());

    for(int32 I : Array)
    {
        LOG(LogProgram, "{}", I);
    }

    Render::Initialize();
    Render::Loop();
    Render::ShutDown();

    return 0;
}
