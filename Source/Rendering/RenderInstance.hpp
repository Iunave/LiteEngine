#pragma once
#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/Array.hpp"

#include <vulkan/vulkan.hpp>

namespace Render
{
    class FInstanceManager final
    {
    public:

        FInstanceManager();
        ~FInstanceManager();

        void CreateInstance(const TDynamicArray<const char8*>& ValidationLayers);
        void DestroyInstance();

        void PopulateExtensionLayers();

        TDynamicArray<Vk::PhysicalDevice> FindAvailablePhysicalDevices() const;

        Vk::Instance GetVulkanInstanceHandle() const {return VulkanInstanceHandle;}
        const TDynamicArray<const char8*>& GetInstanceExtensionLayers() const {return InstanceExtensionLayers;}

    private:

        Vk::Instance VulkanInstanceHandle;
        TDynamicArray<const char8*> InstanceExtensionLayers;
    };
}
