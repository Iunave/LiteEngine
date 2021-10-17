#pragma once

#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/Array.hpp"
#include <vulkan/vulkan.hpp>
/*
struct GLFWwindow;
struct FWindowDimensions;
struct FSwapChainSupportDetails;
struct FQueueFamilyIndices;

struct FSwapChainData
{
    Vk::Extent2D ImageExtent;
    Vk::SurfaceFormatKHR SurfaceFormat;
    Vk::PresentModeKHR PresentMode;
};

class FRenderSwapChainManager final
{
public:

    FRenderSwapChainManager();
    ~FRenderSwapChainManager();

    void PopulateImages(Vk::Device LogicalDevice);
    void DestroyImages(Vk::Device LogicalDevice);

    void CreateImageViews(Vk::Device LogicalDevice);
    void DestroyImageViews(Vk::Device LogicalDevice);

    static Vk::Extent2D ChooseImageExtent(const Vk::SurfaceCapabilitiesKHR& SurfaceCapabilities, FWindowDimensions Dimensions);
    static Vk::SurfaceFormatKHR ChooseSurfaceFormat(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats);
    static Vk::PresentModeKHR ChoosePresentationMode(const TDynamicArray<Vk::PresentModeKHR>& PresentModes);

    void PopulateSwapChainData(const FSwapChainSupportDetails& SwapChainSupportDetails, FWindowDimensions WindowDimensions);

    void CreateSwapChain(Vk::Device LogicalDevice, Vk::SurfaceKHR Surface, const FSwapChainSupportDetails& SwapChainSupportDetails, const FQueueFamilyIndices& QueueIndices, Vk::SwapchainKHR OldSwapChain = NULL_HANDLE);
    void DestroySwapChain(Vk::Device LogicalDevice);

    FSwapChainData GetSwapChainData() const {return SwapChainData;}

private:

    Vk::SwapchainKHR SwapChainHandle;
    FSwapChainData SwapChainData;

    TDynamicArray<Vk::Image> Images;
    TDynamicArray<Vk::ImageView> ImageViews;
};


class FRenderSwapChainManager final : private FNonCopyable
{
public:

    FRenderSwapChainManager(TSharedPtr<FRenderSwapChainManager> InOldSwapChain);

    virtual ~FRenderSwapChainManager() override;

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
*/
