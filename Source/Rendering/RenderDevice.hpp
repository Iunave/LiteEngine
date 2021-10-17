#pragma once

#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/Array.hpp"

#include <vulkan/vulkan.hpp>
/*
struct FSwapChainSupportDetails
{
    Vk::SurfaceCapabilitiesKHR SurfaceCapabilities;
    TDynamicArray<Vk::SurfaceFormatKHR> SurfaceFormats;
    TDynamicArray<Vk::PresentModeKHR> PresentModes;
};

struct FQueueFamilyIndices
{
    FQueueFamilyIndices();

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

class FRenderDeviceManager final
{
public:

    FRenderDeviceManager();
    ~FRenderDeviceManager();

    TDynamicArray<Vk::ExtensionProperties> FindPhysicalDeviceProperties() const;
    TDynamicArray<Vk::QueueFamilyProperties> FindQueueFamilyProperties() const;

    void PopulateQueueFamilyIndices(Vk::SurfaceKHR Surface);

    bool DoesPhysicalDeviceSupportExtensions() const;
    bool DoesPhysicalDeviceSupportFeatures() const;
    bool DoesPhysicalDeviceSupportQueues() const;
    bool DoesPhysicalDeviceSupportSwapChain() const;

    Vk::SurfaceCapabilitiesKHR FindSurfaceCapabilities(Vk::SurfaceKHR Surface);
    TDynamicArray<Vk::SurfaceFormatKHR> FindSurfaceFormats(Vk::SurfaceKHR Surface);
    TDynamicArray<Vk::PresentModeKHR> FindSurfacePresentModes(Vk::SurfaceKHR Surface);

    static TDynamicArray<Vk::PhysicalDevice> FindAvailablePhysicalDevices(Vk::Instance VulkanInstance);

    void PopulateSwapChainSupportDetails(Vk::SurfaceKHR Surface);

    void PickPhysicalDevice(Vk::Instance VulkanInstance, Vk::SurfaceKHR Surface);
    void DestroyPhysicalDevice();

    void CreateLogicalDevice(const TDynamicArray<const char8*>& ValidationLayers);
    void DestroyLogicalDevice();

    void InitializeQueues();

    Vk::PhysicalDevice GetPhysicalDeviceHandle() const {return PhysicalDeviceHandle;}
    Vk::Device GetLogicalDeviceHandle() const {return LogicalDeviceHandle;}
    const FQueueFamilyIndices& GetQueueFamilyIndices() const {return QueueFamilyIndices;}
    const FSwapChainSupportDetails& GetSwapChainSupportDetails() const {return SwapChainSupportDetails;}

private:

    Vk::PhysicalDevice PhysicalDeviceHandle;
    Vk::Device LogicalDeviceHandle;

    Vk::Queue GraphicsQueue;
    Vk::Queue PresentationQueue;
    Vk::Queue ComputeQueue;

    FQueueFamilyIndices QueueFamilyIndices;
    FSwapChainSupportDetails SwapChainSupportDetails;

    TDynamicArray<const char8*> DeviceExtensions;
};
*/
