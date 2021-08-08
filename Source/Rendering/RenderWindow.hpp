#pragma once

#include "Definitions.hpp"
#include "SmartPointer.hpp"
#include "String.hpp"

namespace vk
{
    class Extent2D;
}

struct GLFWwindow;

class FRenderWindow final : private FNonCopyable
{
public:

    FRenderWindow();

    void CreateWindow(int32 Width, int32 Height, FString WindowName, const bool bFullScreen = false);

    void CloseWindow();

    virtual ~FRenderWindow() override = default;

    bool ShouldClose() const;

    UINLINE int32 GetCoordinateWidth() const;
    UINLINE int32 GetCoordinateHeight() const;
    UINLINE int32 GetPixelWidth() const;
    UINLINE int32 GetPixelHeight() const;
    UINLINE Vk::Extent2D GetImageExtent() const;
    UINLINE const FString& GetName() const;
    UINLINE GLFWwindow* GetWindow() const;

protected:

    int32 CoordinateWidth;
    int32 CoordinateHeight;
    int32 PixelWidth;
    int32 PixelHeight;

    FString Name;

    GLFWwindow* Window;

};
