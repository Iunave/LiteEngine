#pragma once

#include "CoreFiles/Definitions.hpp"

#include <vulkan/vulkan.hpp>

namespace Render
{
    struct FSwapChainData
    {
        Vk::Extent2D ImageExtent;
        Vk::SurfaceFormatKHR SurfaceFormat;
        Vk::PresentModeKHR PresentMode;
    };

    class FSwapChainManager final
    {
    public:

        TDynamicArray<Vk::Image> FindSwapChainImages(Vk::Device LogicalDevice, Vk::SwapchainKHR SwapChain);
        TDynamicArray<Vk::ImageView> CreateImageViews(Vk::Device LogicalDevice, const TDynamicArray<Vk::Image>& SourceImages);

        void DestroyImageViews(Vk::Device LogicalDevice, TDynamicArray<Vk::ImageView>& ImageViews);
        void DestroyImages(Vk::Device LogicalDevice, TDynamicArray<Vk::Image>& Images);

        Vk::Extent2D ChooseImageExtent(const Vk::SurfaceCapabilitiesKHR SurfaceCapabilities);
        Vk::SurfaceFormatKHR ChooseSurfaceFormat(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats);
        Vk::PresentModeKHR ChoosePresentationMode(const TDynamicArray<Vk::PresentModeKHR>& PresentModes);

        Vk::SwapchainKHR CreateSwapChain(Vk::Device LogicalDevice, Vk::SurfaceKHR Surface, Vk::SwapchainKHR OldSwapChain = NULL_HANDLE);
        void DestroySwapChain(Vk::SwapchainKHR SwapChain, Vk::Device LogicalDevice);

        Vk::SurfaceKHR CreateSurface(Vk::Instance VulkanInstance, GLFWwindow* RenderWindow);
        void DestroySurface(Vk::Instance VulkanInstance, Vk::SurfaceKHR Surface);

    private:

        Vk::SwapchainKHR SwapChainHandle;

        FSwapChainData SwapChainData;
    };
}

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
