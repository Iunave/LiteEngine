#pragma once

#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/Array.hpp"

#include <vulkan/vulkan.hpp>

namespace Render
{
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

    class FDeviceManager final
    {
    public:

        FDeviceManager();
        ~FDeviceManager();

        TDynamicArray<Vk::ExtensionProperties> FindPhysicalDeviceProperties();
        TDynamicArray<Vk::QueueFamilyProperties> FindQueueFamilyProperties();

        void PopulateQueueFamilyIndices();

        bool DoesPhysicalDeviceSupportExtensions() const;
        bool DoesPhysicalDeviceSupportFeatures() const;
        bool DoesPhysicalDeviceSupportSwapChain() const;

        Vk::SurfaceCapabilitiesKHR FindSurfaceCapabilities();
        TDynamicArray<Vk::SurfaceFormatKHR> FindSurfaceFormats();
        TDynamicArray<Vk::PresentModeKHR> FindSurfacePresentModes();

        void PopulateSwapChainSupportDetails();

        void PickPhysicalDevice(Vk::Instance VulkanInstance);
        void DestroyPhysicalDevice();

        void CreateLogicalDevice(const TDynamicArray<const char8*>& ValidationLayers);
        void DestroyLogicalDevice();

        Vk::PhysicalDevice GetPhysicalDeviceHandle() const {return PhysicalDeviceHandle;}
        Vk::Device GetLogicalDeviceHandle() const {return LogicalDeviceHandle;}

    private:

        Vk::PhysicalDevice PhysicalDeviceHandle;
        Vk::Device LogicalDeviceHandle;

        Vk::Queue GraphicsQueue;
        Vk::Queue PresentationQueue;

        FQueueFamilyIndices QueueFamilyIndices;
        FSwapChainSupportDetails SwapChainSupportDetails;

        TDynamicArray<const char8*> DeviceExtensions;
    };
}
