#include "Definitions.hpp"
#include "Object/Object.hpp"
#include "Interface/Tick.hpp"
#include "EngineGlobals.hpp"

#define INCLUDE_ALL_VULKAN_FILES
#include "VulkanCommon.hpp"
#undef INCLUDE_ALL_VULKAN_FILES

#define GLFW_INCLUDE_NONE
namespace Glfw
{
#include "GLFW/glfw3.h"
}
#undef GLFW_INCLUDE_NONE

void MainEngineLoop()
{
    while(!GRenderWindow.ShouldClose() && !GIsEngineExitRequested)
    {
        Glfw::glfwPollEvents();
    }
}

OBJECT_CLASS(OFoo) : public OTickable,  public OObject
{
    OBJECT_BODY(OTickable, OObject)
public:

    OFoo()
        : OTickable(true)
    {
    }

    virtual void Tick(float64 DeltaTime) override {}

};

OBJECT_CLASS(OBar1) : public OObject
{
    OBJECT_BODY(OObject)
};

OBJECT_CLASS(OBar2) : public OBar1
{
    OBJECT_BODY(OBar1)
};

OBJECT_CLASS(OBar3) : public OBar2
{
    OBJECT_BODY(OBar2)
};

OBJECT_CLASS(OBar4) : public OBar3
{
    OBJECT_BODY(OBar3)
};

OBJECT_CLASS(OBar5) : public OBar4
{
    OBJECT_BODY(OBar4)
};

OBJECT_CLASS(OBar6) : public OBar5
{
    OBJECT_BODY(OBar5)
};

OBJECT_CLASS(OBar7) : public OBar6
{
    OBJECT_BODY(OBar6)
};

OBJECT_CLASS(OBar8) : public OBar7
{
    OBJECT_BODY(OBar7)
};

OBJECT_CLASS(OBar9) : public OBar8
{
    OBJECT_BODY(OBar8)
};

OBJECT_CLASS(OBar10) : public OBar9
{
    OBJECT_BODY(OBar9)
};

OBJECT_CLASS(OBar11) : public OBar10
{
    OBJECT_BODY(OBar10)
};

OBJECT_CLASS(OBar12) : public OBar11
{
    OBJECT_BODY(OBar11)
};

OBJECT_CLASS(OBar13) : public OBar12
{
    OBJECT_BODY(OBar12)
};

OBJECT_CLASS(OBar14) : public OBar13
{
    OBJECT_BODY(OBar13)
};

OBJECT_CLASS(OBar15) : public OBar14
{
    OBJECT_BODY(OBar14)
};

OBJECT_CLASS(OBar16) : public OBar15
{
    OBJECT_BODY(OBar15)
};

OBJECT_CLASS(OBar17) : public OBar16
{
    OBJECT_BODY(OBar16)
};

OBJECT_CLASS(OBar18) : public OBar17
{
    OBJECT_BODY(OBar17)
};

OBJECT_CLASS(OBar19) : public OBar18
{
    OBJECT_BODY(OBar18)
};

OBJECT_CLASS(OBar20) : public OBar19
{
    OBJECT_BODY(OBar19)
};

OBJECT_CLASS(OBar21) : public OBar20
{
    OBJECT_BODY(OBar20)
};

OBJECT_CLASS(OBar22) : public OBar21
{
    OBJECT_BODY(OBar21)
};

OBJECT_CLASS(OBar23) : public OBar22
{
    OBJECT_BODY(OBar22)
};

OBJECT_CLASS(OObject1) : public OObject
{
    OBJECT_BODY(OObject)
public:
    float64 A{1};
};

OBJECT_CLASS(OObject2)
{
    OBJECT_BODY()
public:
    float64 B{2};
};

OBJECT_CLASS(OObject3) : public OObject1, public OObject2
{
    OBJECT_BODY(OObject1, OObject2)
public:
    float64 C{3};
};

int32 main(const int32 NumProgramOptions, const char8** ProgramOptions)
{
    LOG(LogProgram, "entered main with thread-ID: {}", StrUtil::IntToHex(pthread_self()));

    OObject1* Object1{new OObject3};
    OObject2* Object2{ObjectCast<OObject2*>(Object1)};
    OObject3* Object3{ObjectCast<OObject3*>(Object1)};
    OObject* Object0{ObjectCast<OObject*>(Object3)};

    LOG(LogTemp, "{} {} {} {}", (void*)Object0, Object1->A, Object2->B, Object3->C);

    return 0;
}
