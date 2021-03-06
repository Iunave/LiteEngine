#pragma once

#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/String.hpp"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace vk
{
    class Extent2D;
}

struct FWindowDimensions
{
    int32 CoordinateWidth;
    int32 CoordinateHeight;
    int32 PixelWidth;
    int32 PixelHeight;
};

class FWindow
{
private:

    static void FrameBufferResizeCallback(GLFWwindow* Window, int32 Width, int32 Height);

public:

    FWindow();
    ~FWindow();

    void CreateWindow(int32 Width, int32 Height, FString<SS60> InWindowName, const bool bFullScreen = false);

    void CloseWindow();

    bool ShouldClose() const;

    bool IsMinimized() const;

    void UpdateWindowDimensions();

    FWindowDimensions GetWindowDimensions() const
    {
        return WindowDimensions;
    }

    Vk::Extent2D GetImageExtent() const
    {
        return Vk::Extent2D{static_cast<uint32>(WindowDimensions.PixelHeight), static_cast<uint32>(WindowDimensions.PixelWidth)};
    }

    const FString<SS60>& GetWindowName() const
    {
        return WindowName;
    }

    GLFWwindow* GetWindowHandle() const
    {
        return WindowHandle;
    }

    bool HasBeenResized;

protected:

    FWindowDimensions WindowDimensions;
    FString<SS60> WindowName;
    GLFWwindow* WindowHandle;
};
