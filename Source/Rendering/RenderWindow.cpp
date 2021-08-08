#include "RenderWindow.hpp"
#include "VulkanCommon.hpp"
#include "Log.hpp"

#define GLFW_INCLUDE_NONE
namespace Glfw
{
#include "GLFW/glfw3.h"
}

FRenderWindow::FRenderWindow()
    : CoordinateWidth{0}
    , CoordinateHeight{0}
    , PixelWidth{0}
    , PixelHeight{0}
    , Name{"unnamed"}
    , Window{nullptr}
{
}

void FRenderWindow::CreateWindow(int32 Width, int32 Height, FString WindowName, const bool bFullScreen)
{
    CoordinateWidth = Width;
    CoordinateHeight = Height;
    Name = Move(WindowName);

    if(Glfw::glfwInit())
    {
        Glfw::glfwWindowHint(GLFW_CLIENT_API, false);
        Glfw::glfwWindowHint(GLFW_RESIZABLE, false);
        Glfw::glfwWindowHint(GLFW_FOCUSED, false);
        Glfw::glfwWindowHint(GLFW_FOCUS_ON_SHOW, true);

        Glfw::GLFWmonitor* Monitor{nullptr};

        if(bFullScreen)
        {
            Monitor = Glfw::glfwGetPrimaryMonitor();
            ASSERT(Monitor);
        }

        Window = glfwCreateWindow(CoordinateWidth, CoordinateHeight, Name.RawString(), Monitor, nullptr);

        if(bFullScreen)
        {
            Glfw::glfwGetWindowSize(Window, &CoordinateWidth, &CoordinateHeight);
        }

        Glfw::glfwGetFramebufferSize(Window, &PixelWidth, &PixelHeight);
    }

    ASSERT(Window);
    LOG(LogGLFW, "created window. width: {} height: {}", CoordinateWidth, CoordinateHeight);
    LOG(LogGLFW, "fullscreen: {}", bFullScreen ? "true" : "false");
}

void FRenderWindow::CloseWindow()
{
    if(Window)
    {
        Glfw::glfwDestroyWindow(Window);
    }

    Glfw::glfwTerminate();
}

bool FRenderWindow::ShouldClose() const
{
    return Glfw::glfwWindowShouldClose(Window);
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

const FString& FRenderWindow::GetName() const
{
    return Name;
}

GLFWwindow* FRenderWindow::GetWindow() const
{
    return Window;
}
