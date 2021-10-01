#pragma once

#include "Definitions.hpp"
#include "String.hpp"

#include <vulkan/vulkan.hpp>

namespace vk
{
    class Extent2D;
    class Instance;
    class SurfaceKHR;
}

struct GLFWwindow;

namespace Render
{
    struct FWindowDimensions
    {
        int32 CoordinateWidth;
        int32 CoordinateHeight;
        int32 PixelWidth;
        int32 PixelHeight;
    };

    class FRenderWindow final
    {
    public:

        FRenderWindow();
        ~FRenderWindow();

        void CreateWindow(int32 Width, int32 Height, FString<ss60> InWindowName, const bool bFullScreen = false);
        void CloseWindow();

        void CreateSurface(Vk::Instance VulkanInstance);
        void DestroySurface(Vk::Instance VulkanInstance);

        bool ShouldClose() const;

        FWindowDimensions GetWindowDimensions() const {return WindowDimensions;}
        Vk::Extent2D GetImageExtent() const {return Vk::Extent2D{static_cast<uint32>(WindowDimensions.PixelHeight), static_cast<uint32>(WindowDimensions.PixelWidth)};}
        const FString<ss60>& GetWindowName() const {return WindowName;}
        GLFWwindow* GetWindowHandle() const {return WindowHandle;}
        Vk::SurfaceKHR GetSurfaceHandle() const {return SurfaceHandle;}

    protected:

        FWindowDimensions WindowDimensions;

        FString<ss60> WindowName;

        GLFWwindow* WindowHandle;

        Vk::SurfaceKHR SurfaceHandle;
    };
}
