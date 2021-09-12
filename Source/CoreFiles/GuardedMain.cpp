
#include <Interface/Damageable.hpp>
#include <Actor/BuildComponent/BuildComponent.hpp>
#include <Actor/Robot.hpp>


#include "Definitions.hpp"
#include "Object/Object.hpp"
#include "SmartPointer.hpp"
#include "Interface/Tick.hpp"
#include "Vector.hpp"
#include "Log.hpp"
#include "Quaternion.hpp"
#include "../Rendering/RenderWindow.hpp"


OBJECT_CLASS(OFoo) : public OTickable
{
    OBJECT_BASES(OTickable)
public:

    virtual void Tick(float64) override{}

    int32 lol{43214};

};

int32 main()
{
#if defined(AVX512)
    LOG(LogProgram, "executing avx512 version");
#elif defined(AVX256)
    LOG(LogProgram, "executing avx2 version");
#endif
    LOG(LogProgram, "entered main with thread-ID: {}", StrUtil::IntToHex(pthread_self()));

    TSharedPtr<OTickable> Foo{MakeShared<OFoo>()};
    TSharedPtr<OFoo> Tickable{ObjectCast<OFoo*>(Foo)};
    LOG(LogProgram, "{}", Tickable->lol);

    FRenderWindow RenderWindow{};
    RenderWindow.CreateWindow(1000, 1000, "my window", false);

    return 0;
}
