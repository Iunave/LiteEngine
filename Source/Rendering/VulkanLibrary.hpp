#pragma once

#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/Array.hpp"
#include "CoreFiles/String.hpp"
#include "RenderWindow.hpp"
#include <vulkan/vulkan.hpp>

namespace Render
{
    struct FQueue
    {
        constexpr FQueue()
            : Index{UINT32_MAX}
            , Handle{NULL_HANDLE}
        {
        }

        uint32 Index;
        Vk::Queue Handle;
    };

    struct FQueueFamilies
    {
        FQueue Graphic;
        FQueue Compute;
        FQueue Protected;
        FQueue SparseBinding;
        FQueue Transfer;
        FQueue Presentation;
    };

    template<typename... TQueues>
    inline constexpr bool AreQueuesValid(TQueues&&... Queues)
    {
        return ((Queues.Index != UINT32_MAX) && ...);
    }

    struct FAllocation
    {
        Vk::Buffer Buffer;
        Vk::DeviceMemory Memory;
    };

    struct FSwapChainData
    {
        inline bool IsSwapChainSupported()
        {
            return !SurfaceFormats.IsEmpty() && !PresentModes.IsEmpty();
        }

        Vk::SurfaceCapabilitiesKHR SurfaceCapabilities;
        TDynamicArray<Vk::SurfaceFormatKHR> SurfaceFormats;
        TDynamicArray<Vk::PresentModeKHR> PresentModes;

        Vk::Extent2D ImageExtent;
        Vk::SurfaceFormatKHR SurfaceFormat;
        Vk::PresentModeKHR PresentMode;

        TDynamicArray<Vk::Image> Images;
        TDynamicArray<Vk::ImageView> ImageViews;
        TDynamicArray<Vk::Framebuffer> FrameBuffers;
    };

}

namespace Render
{
    inline constexpr int64 MaxFramesInFlight{3};

    extern const uint32 VulkanVersion;
    extern const uint32 VulkanAPIVersion;

    TDynamicArray<const char8*> FindValidationLayers();
    TDynamicArray<const char8*> FindInstanceExtensionLayers();
    TDynamicArray<const char8*> FindDeviceExtensionLayers();

    TDynamicArray<Vk::ExtensionProperties> FindPhysicalDeviceProperties(Vk::PhysicalDevice Device);
    TDynamicArray<Vk::QueueFamilyProperties> FindQueueFamilyProperties(Vk::PhysicalDevice Device);

    FQueueFamilies PopulateQueueFamilyIndices(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface);

    bool DoesPhysicalDeviceSupportExtensions(Vk::PhysicalDevice Device, const TDynamicArray<const char8*>& DeviceExtensionLayers);
    bool DoesPhysicalDeviceSupportFeatures(Vk::PhysicalDevice Device);
    bool AreQueuesSupported(FQueueFamilies QueueFamilies);
    bool IsSwapChainSupported(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats, const TDynamicArray<Vk::PresentModeKHR>& PresentModes);
    bool SwapChainNeedsRecreation(Vk::Result Result);

    Vk::SurfaceCapabilitiesKHR FindSurfaceCapabilities(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface);

    TDynamicArray<Vk::SurfaceFormatKHR> FindSurfaceFormats(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface);
    void FindSurfaceFormats(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface, TDynamicArray<Vk::SurfaceFormatKHR>& OutFormats);

    TDynamicArray<Vk::PresentModeKHR> FindSurfacePresentModes(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface);
    void FindSurfacePresentModes(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface, TDynamicArray<Vk::PresentModeKHR>& OutPresentModes);

    TDynamicArray<Vk::PhysicalDevice> FindAvailablePhysicalDevices(Vk::Instance VulkanInstance);

    TDynamicArray<Vk::Image> FindSwapChainImages(Vk::Device LogicalDevice, Vk::SwapchainKHR SwapChain);
    void FindSwapChainImages(Vk::Device LogicalDevice, Vk::SwapchainKHR SwapChain, TDynamicArray<Vk::Image>& OutImages);

    TDynamicArray<Vk::ImageView> CreateSwapChainImageViews(Vk::Device LogicalDevice, const TDynamicArray<Vk::Image>& Images, Vk::SurfaceFormatKHR SurfaceFormat);

    Vk::Extent2D ChooseImageExtent(FWindowDimensions WindowDimensions, Vk::SurfaceCapabilitiesKHR SurfaceCapabilities);
    Vk::SurfaceFormatKHR ChooseSurfaceFormat(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats);
    Vk::PresentModeKHR ChoosePresentationMode(const TDynamicArray<Vk::PresentModeKHR>& PresentModes);

    template<EStackSize SS>
    extern TDynamicArray<uint8> ReadShaderFile(FString<SS> ShaderFileName);

    Vk::ShaderModule CreateShaderModule(Vk::Device LogicalDevice, const TDynamicArray<uint8>& Code);

    template<EStackSize SS>
    extern Vk::ShaderModule CreateShaderModule(Vk::Device LogicalDevice, FString<SS> ShaderFileName);

    uint32 FindMemoryType(Vk::PhysicalDevice PhysicalDevice, uint32 TypeFilter, Vk::MemoryPropertyFlags Flags);
    Vk::Buffer CreateBuffer(Vk::Device LogicalDevice, Vk::BufferUsageFlags BufferUsage, uint64 NumBytes);
    Vk::DeviceMemory AllocateGPUMemory(Vk::PhysicalDevice PhysicalDevice, Vk::Device LogicalDevice, Vk::Buffer& Buffer, Vk::MemoryPropertyFlags MemoryFlags);
    void CopyDataToGPU(Vk::Device LogicalDevice, Vk::Buffer Buffer, Vk::DeviceMemory DeviceMemory, const void* Data, uint64 NumBytes);
    void CopyBuffer(Vk::Buffer Source, Vk::Buffer Target, Vk::Device LogicalDevice, Vk::CommandPool CommandPool, Vk::Queue SubmitQueue, uint64 NumBytes);

    Vk::Semaphore CreateSemaphore(Vk::Device LogicalDevice);
    Vk::Fence CreateFence(Vk::Device LogicalDevice);

    uint32 VulkanDebugCallback(Vk::DebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, Vk::DebugUtilsMessageTypeFlagsEXT MessageType, const Vk::DebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData);
    Vk::DebugUtilsMessengerCreateInfoEXT MakeDebugUtilsMessengerCreateInfo();
    Vk::DebugUtilsMessengerEXT CreateDebugMessenger(Vk::Instance VulkanInstance);
    void DestroyDebugMessenger(Vk::Instance VulkanInstance, Vk::DebugUtilsMessengerEXT DebugHandle);

    Vk::Instance CreateInstance(const TDynamicArray<const char8*>& ValidationLayers, const TDynamicArray<const char8*>& InstanceLayers);
    Vk::SurfaceKHR CreateSurface(Vk::Instance VulkanInstance, GLFWwindow* Window);

    Vk::PhysicalDevice PickPhysicalDevice(Vk::Instance VulkanInstance,Vk::SurfaceKHR Surface,const TDynamicArray<const char8*>& DeviceExtensionLayers, FQueueFamilies& OutQueueFamilies, Vk::SurfaceCapabilitiesKHR& OutSurfaceCapabilities, TDynamicArray<Vk::SurfaceFormatKHR>& OutSurfaceFormats, TDynamicArray<Vk::PresentModeKHR>& OutPresentModes);
    Vk::Device CreateLogicalDevice(Vk::PhysicalDevice PhysicalDevice, FQueueFamilies QueueFamilies, const TDynamicArray<const char8*>& ValidationLayers, const TDynamicArray<const char8*>& DeviceExtensionLayers);
    Vk::SwapchainKHR CreateSwapChain(Vk::SwapchainKHR OldSwapChain, Vk::Device LogicalDevice, Vk::SurfaceKHR Surface, Vk::SurfaceCapabilitiesKHR SurfaceCapabilities, FQueueFamilies QueueFamilies, Vk::Extent2D ImageExtent, Vk::PresentModeKHR PresentMode, Vk::SurfaceFormatKHR SurfaceFormat);
    Vk::RenderPass CreateRenderPass(Vk::Device LogicalDevice, Vk::SurfaceFormatKHR SurfaceFormat);
    Vk::PipelineLayout CreateGraphicsPipelineLayout(Vk::Device LogicalDevice);
    Vk::Pipeline CreateGraphicsPipeline(Vk::Device LogicalDevice, Vk::RenderPass RenderPass, Vk::PipelineLayout PipelineLayout, Vk::ShaderModule FragShaderModule, Vk::ShaderModule VertShaderModule, Vk::Extent2D ImageExtent);
    TDynamicArray<Vk::Framebuffer> CreateFrameBuffers(Vk::Device LogicalDevice, Vk::RenderPass RenderPass, Vk::Extent2D ImageExtent, const TDynamicArray<Vk::ImageView>& ImageViews);
    Vk::CommandPool CreateCommandPool(Vk::Device LogicalDevice, uint32 QueueIndex);

    ///this command pool is used for creating short lived command buffers
    Vk::CommandPool CreateCommandPoolTransient(Vk::Device LogicalDevice, uint32 QueueIndex);
    TDynamicArray<Vk::CommandBuffer> CreateCommandBuffers(Vk::Device LogicalDevice, Vk::CommandPool CommandPool, uint64 NumBuffers);
    Vk::CommandBuffer CreateCommandBuffer(Vk::Device LogicalDevice, Vk::CommandPool CommandPool);
    void BeginRenderPass(uint32 VertexCount, Vk::RenderPass RenderPass, Vk::Pipeline Pipeline, Vk::Buffer Buffer, Vk::Extent2D ImageExtent, const TDynamicArray<Vk::Framebuffer>& FrameBuffers, const TDynamicArray<Vk::CommandBuffer>& CommandBuffers);
}
