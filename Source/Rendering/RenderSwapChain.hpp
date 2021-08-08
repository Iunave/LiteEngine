#pragma once

#include "Definitions.hpp"
#include "SmartPointer.hpp"
#include "VulkanCommon.hpp"

class FRenderDevice;
class FRenderWindow;

class FRenderSwapChain final : private FNonCopyable
{
public:

    FRenderSwapChain(TSharedPtr<FRenderSwapChain> InOldSwapChain);

    virtual ~FRenderSwapChain() override;

    void Initialize();

    void ShutDown();

    UINLINE Vk::SwapchainKHR GetSwapChain() const;
    UINLINE Vk::RenderPass GetRenderPass() const;
    UINLINE Vk::Extent2D GetImageExtent() const;

    TDynamicArray<Vk::Image> FindSwapChainImages() const;

    static Vk::Extent2D ChooseImageExtent(const Vk::SurfaceCapabilitiesKHR SurfaceCapabilities);

    static Vk::SurfaceFormatKHR ChooseSurfaceFormat(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats);

    static Vk::PresentModeKHR ChoosePresentationMode(const TDynamicArray<Vk::PresentModeKHR>& PresentModes);

    void RecordCommandBuffer();

protected:

    void CreateSwapChain();

    void CreateImageViews();

    void CreateDepthResources();

    void CreateRenderPass();

    void CreateFrameBuffers();

    void CreateCommandBuffers();

    void CreateSyncObjects();

protected:

    TSharedPtr<FRenderSwapChain> OldSwapChain;

    Vk::SwapchainKHR SwapChain;

    Vk::RenderPass RenderPass;

    Vk::Extent2D ImageExtent;
    Vk::SurfaceFormatKHR SurfaceFormat;
    Vk::PresentModeKHR PresentMode;

    TDynamicArray<Vk::Image> Images;

    TDynamicArray<Vk::ImageView> ImageViews;

    TDynamicArray<Vk::Framebuffer> FrameBuffers;

    TDynamicArray<Vk::CommandBuffer> CommandBuffers;
};