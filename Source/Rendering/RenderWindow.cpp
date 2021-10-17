#include "RenderWindow.hpp"
#include "Log.hpp"

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

void Render::OWindow::FrameBufferResizeCallback(GLFWwindow* Window, int32 PixelWidth, int32 PixelHeight)
{
    OWindow* RenderWindowPtr{static_cast<OWindow*>(glfwGetWindowUserPointer(Window))};

    glfwGetWindowSize(Window, &RenderWindowPtr->WindowDimensions.CoordinateWidth, &RenderWindowPtr->WindowDimensions.CoordinateHeight);

    RenderWindowPtr->HasBeenResized = true;

    RenderWindowPtr->WindowDimensions.PixelWidth = PixelWidth;
    RenderWindowPtr->WindowDimensions.PixelHeight = PixelHeight;

    LOG(LogGLFW, "window has been resized. new width: {} new height: {}", RenderWindowPtr->WindowDimensions.PixelWidth, RenderWindowPtr->WindowDimensions.PixelHeight);
}

Render::OWindow::OWindow()
    : HasBeenResized{false}
    , WindowDimensions{0, 0, 0, 0}
    , WindowName{"unnamed"}
    , WindowHandle{NULL_HANDLE}
{
}

Render::OWindow::~OWindow()
{
    ASSERT(WindowHandle == nullptr, "window was not destroyed");
}

void Render::OWindow::CreateWindow(int32 Width, int32 Height, FString<SS60> InWindowName, const bool bFullScreen)
{
    WindowDimensions.CoordinateWidth = Width;
    WindowDimensions.CoordinateHeight = Height;
    WindowName = Move(InWindowName);

    if(glfwInit())
    {
        glfwWindowHint(GLFW_CLIENT_API, false);
        glfwWindowHint(GLFW_RESIZABLE, true);
        glfwWindowHint(GLFW_FOCUSED, false);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, true);

        GLFWmonitor* Monitor{nullptr};

        if(bFullScreen)
        {
            Monitor = glfwGetPrimaryMonitor();
            ASSERT(Monitor);
        }

        WindowHandle = glfwCreateWindow(WindowDimensions.CoordinateWidth, WindowDimensions.CoordinateHeight, WindowName.RawString(), Monitor, nullptr);

        glfwSetWindowUserPointer(WindowHandle, this);
        glfwSetFramebufferSizeCallback(WindowHandle, FrameBufferResizeCallback);

        if(bFullScreen)
        {
            glfwGetWindowSize(WindowHandle, &WindowDimensions.CoordinateWidth, &WindowDimensions.CoordinateHeight);
        }

        glfwGetFramebufferSize(WindowHandle, &WindowDimensions.PixelWidth, &WindowDimensions.PixelHeight);
    }

    ASSERT(WindowHandle);
    LOG(LogGLFW, "created window with width: {} height: {} fullscreen: {}", WindowDimensions.CoordinateWidth, WindowDimensions.CoordinateHeight, Math::ChooseVar<const char8*>(bFullScreen, "true", "false"));
}

void Render::OWindow::CloseWindow()
{
    if(WindowHandle)
    {
        glfwDestroyWindow(WindowHandle);
        WindowHandle = nullptr;

        LOG(LogGLFW, "destroyed window");
    }

    glfwTerminate();
}

bool Render::OWindow::ShouldClose() const
{
    return glfwWindowShouldClose(WindowHandle);
}

bool Render::OWindow::IsMinimized() const
{
    return WindowDimensions.PixelWidth == 0 || WindowDimensions.PixelHeight == 0;
}

void Render::OWindow::UpdateWindowDimensions()
{
    glfwGetWindowSize(WindowHandle, &WindowDimensions.CoordinateWidth, &WindowDimensions.CoordinateHeight);
    glfwGetFramebufferSize(WindowHandle, &WindowDimensions.PixelWidth, &WindowDimensions.PixelHeight);
}

