#pragma once

#include "Definitions.hpp"
#include "VulkanCommon.hpp"
#include "SmartPointer.hpp"
#include "Array.hpp"

class FRenderWindow;

struct FSwapChainSupportDetails
{
    Vk::SurfaceCapabilitiesKHR SurfaceCapabilities;

    TDynamicArray<Vk::SurfaceFormatKHR> SurfaceFormats;

    TDynamicArray<Vk::PresentModeKHR> PresentModes;
};

struct FQueueFamilyIndices
{
    UINLINE FQueueFamilyIndices();

    template<typename... Ts>
    static bool CheckValidity(Ts... Indices)
    {
        return ((Indices != UINT32_MAX) && ...);
    }

    uint32 Graphic;
    uint32 Compute;
    uint32 Protected;
    uint32 SparseBinding;
    uint32 Transfer;
    uint32 Presentation;
};

class FRenderDevice final : private FNonCopyable
{
public:

#if USE_VULKAN_VALIDATION_LAYERS
    FRenderDevice(TDynamicArray<const char8*>&& RequiredDeviceExtensions, TDynamicArray<const char8*>&& RequestedValidationLayers);
#else
    FRenderDevice(TArray<const char8*>&& RequiredDeviceExtensions);
#endif
    virtual ~FRenderDevice() override;

public:

    void Initialize();

    void ShutDown();

#if USE_VULKAN_VALIDATION_LAYERS
    UINLINE const TDynamicArray<const char8*>& GetValidationLayers() const;
#endif
    UINLINE const TDynamicArray<const char8*>& GetDeviceExtensions() const;

    UINLINE Vk::Instance GetVulkanInstance() const;
    UINLINE Vk::PhysicalDevice GetPhysicalDevice() const;
    UINLINE Vk::Device GetLogicalDevice() const;
    UINLINE Vk::SurfaceKHR GetSurface() const;
    UINLINE Vk::Queue GetGraphicsQueue() const;
    UINLINE Vk::Queue GetPresentationQueue() const;
    UINLINE Vk::CommandPool GetCommandPool() const;
    UINLINE FQueueFamilyIndices GetQueueFamilyIndices() const;
    UINLINE const FSwapChainSupportDetails& GetSwapChainSupportDetails() const;

protected:

    void CreateInstance();

#if USE_VULKAN_VALIDATION_LAYERS
    Vk::Result SetupDebugMessenger();
#endif

    void CreateSurface();

    void ChoosePhysicalDevice();

    void CreateLogicalDevice();

    void CreateQueues();

    void CreateCommandPool();

public:

    bool DoesDeviceSupportExtensions() const;

    TDynamicArray<Vk::PhysicalDevice> FindAvailablePhysicalDevices() const;

    static TDynamicArray<Vk::LayerProperties> FindAvailableValidationLayers();

    TDynamicArray<Vk::ExtensionProperties> FindAvailableExtensions() const;

    TDynamicArray<Vk::QueueFamilyProperties> FindQueueFamilyProperties() const;

    FQueueFamilyIndices QueryQueueFamilyIndices() const;

    Vk::SurfaceCapabilitiesKHR FindSurfaceCapabilities() const;

    TDynamicArray<Vk::SurfaceFormatKHR> FindSurfaceFormats() const;

    TDynamicArray<Vk::PresentModeKHR> FindSurfacePresentModes() const;

    FSwapChainSupportDetails QuerySwapChainSupportDetails() const;

    TDynamicArray<Vk::CommandBuffer> BeginCommandBuffers(const int64 NumCommandBuffers) const;

    void EndCommandBuffers(TDynamicArray<Vk::CommandBuffer>& CommandBuffers) const;

protected:

    TDynamicArray<const char8*> DeviceExtensions;

#if USE_VULKAN_VALIDATION_LAYERS
    TDynamicArray<const char8*> ValidationLayers;

    Vk::DebugUtilsMessengerEXT DebugMessenger;
#endif

    Vk::Instance VulkanInstance;
    Vk::PhysicalDevice PhysicalDevice;
    Vk::Device LogicalDevice;

    Vk::SurfaceKHR Surface;

    Vk::Queue GraphicsQueue;
    Vk::Queue PresentationQueue;

    Vk::CommandPool CommandPool;

    FQueueFamilyIndices QueueFamilyIndices;
    FSwapChainSupportDetails SwapChainSupportDetails;
};
