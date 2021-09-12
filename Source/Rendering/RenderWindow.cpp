#include "RenderWindow.hpp"
#include "VulkanCommon.hpp"
#include "Log.hpp"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

FRenderWindow::FRenderWindow()
    : CoordinateWidth{0}
    , CoordinateHeight{0}
    , PixelWidth{0}
    , PixelHeight{0}
    , Name{"unnamed"}
    , Window{nullptr}
{
}

void FRenderWindow::CreateWindow(int32 Width, int32 Height, FString<ss60> WindowName, const bool bFullScreen)
{
    CoordinateWidth = Width;
    CoordinateHeight = Height;
    Name = Move(WindowName);

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

        Window = glfwCreateWindow(CoordinateWidth, CoordinateHeight, Name.RawString(), Monitor, nullptr);

        if(bFullScreen)
        {
            glfwGetWindowSize(Window, &CoordinateWidth, &CoordinateHeight);
        }

        glfwGetFramebufferSize(Window, &PixelWidth, &PixelHeight);
    }

    ASSERT(Window);
    LOG(LogGLFW, "created window with width: {} height: {} fullscreen: {}", CoordinateWidth, CoordinateHeight, Math::ChooseVar<const char8*>(bFullScreen, "true", "false"));
}

void FRenderWindow::CloseWindow()
{
    if(Window)
    {
        glfwDestroyWindow(Window);
    }

    glfwTerminate();
}

bool FRenderWindow::ShouldClose() const
{
    return glfwWindowShouldClose(Window);
}

int32 FRenderWindow::GetCoordinateWidth() const
{
    return CoordinateWidth;
}

int32 FRenderWindow::GetCoordinateHeight() const
{
    return CoordinateHeight;
}

int32 FRenderWindow::GetPixelWidth() const
{
    return PixelWidth;
}

int32 FRenderWindow::GetPixelHeight() const
{
    return PixelHeight;
}

Vk::Extent2D FRenderWindow::GetImageExtent() const
{
    return Vk::Extent2D{static_cast<uint32>(PixelHeight), static_cast<uint32>(PixelWidth)};
}

const FString<ss60>& FRenderWindow::GetWindowName() const
{
    return Name;
}

GLFWwindow* FRenderWindow::GetWindow() const
{
    return Window;
}
