#include "RenderWindow.hpp"
#include "Log.hpp"

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

using namespace Render;

FRenderWindow::FRenderWindow()
    : WindowDimensions{0, 0, 0, 0}
    , WindowName{"unnamed"}
    , WindowHandle{NULL_HANDLE}
    , SurfaceHandle{NULL_HANDLE}
{
}


FRenderWindow::~FRenderWindow()
{
    ASSERT(WindowHandle == nullptr, "window was not destroyed");
    ASSERT(!SurfaceHandle, "surface was not destroyed");
}

void FRenderWindow::CreateWindow(int32 Width, int32 Height, FString<ss60> WindowName, const bool bFullScreen)
{
    WindowDimensions.CoordinateWidth = Width;
    WindowDimensions.CoordinateHeight = Height;
    WindowName = Move(WindowName);

    if(glfwInit())
    {
        glfwWindowHint(GLFW_CLIENT_API, false);
        glfwWindowHint(GLFW_RESIZABLE, false);
        glfwWindowHint(GLFW_FOCUSED, false);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, true);

        GLFWmonitor* Monitor{nullptr};

        if(bFullScreen)
        {
            Monitor = glfwGetPrimaryMonitor();
            ASSERT(Monitor);
        }

        WindowHandle = glfwCreateWindow(WindowDimensions.CoordinateWidth, WindowDimensions.CoordinateHeight, WindowName.RawString(), Monitor, nullptr);

        if(bFullScreen)
        {
            glfwGetWindowSize(WindowHandle, &WindowDimensions.CoordinateWidth, &WindowDimensions.CoordinateHeight);
        }

        glfwGetFramebufferSize(WindowHandle, &WindowDimensions.PixelWidth, &WindowDimensions.PixelHeight);
    }

    ASSERT(WindowHandle);
    LOG(LogGLFW, "created window with width: {} height: {} fullscreen: {}", WindowDimensions.CoordinateWidth, WindowDimensions.CoordinateHeight, Math::ChooseVar<const char8*>(bFullScreen, "true", "false"));
}

void FRenderWindow::CloseWindow()
{
    if(WindowHandle)
    {
        glfwDestroyWindow(WindowHandle);
        WindowHandle = nullptr;
    }

    glfwTerminate();
}

void FRenderWindow::CreateSurface(Vk::Instance VulkanInstance)
{
    VkSurfaceKHR SurfaceHandleResult{NULL_HANDLE};

    Vk::Result Result{static_cast<Vk::Result>(glfwCreateWindowSurface(VulkanInstance, WindowHandle, nullptr, &SurfaceHandleResult))};
    ASSERT(Result == Vk::Result::eSuccess, "could not create surface [{}]", Vk::to_string(Result));

    LOG(LogVulkan, "created surface");

    SurfaceHandle = SurfaceHandleResult;
}

void FRenderWindow::DestroySurface(Vk::Instance VulkanInstance)
{
    if(!SurfaceHandle)
    {
        LOGW(LogVulkan, "failed to destroy surface [its already invalid]");
        return;
    }

    VulkanInstance.destroySurfaceKHR(SurfaceHandle);
    SurfaceHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed surface");
}

bool FRenderWindow::ShouldClose() const
{
    return glfwWindowShouldClose(WindowHandle);
}
