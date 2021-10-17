#include "RenderInstance.hpp"
#include "RenderDebug.hpp"
#include "CoreFiles/Log.hpp"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
/*
FRenderInstanceManager::FRenderInstanceManager()
    : VulkanInstanceHandle{NULL_HANDLE}
{
}

FRenderInstanceManager::~FRenderInstanceManager()
{
    ASSERT(!VulkanInstanceHandle, "vulkan instance handle was not destroyed");
}

void FRenderInstanceManager::PopulateExtensionLayers()
{
    uint32 GLFWExtensionCount{0};
    const char8** GLFWExtensionNames{glfwGetRequiredInstanceExtensions(&GLFWExtensionCount)};

    InstanceExtensionLayers.ReserveUndefined(GLFWExtensionCount + DEBUG);

    for(uint32 Index{0}; Index < GLFWExtensionCount; ++Index)
    {
        InstanceExtensionLayers.Append(GLFWExtensionNames[Index]);
    }
#if DEBUG
    InstanceExtensionLayers.Append(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    for(const char8* Extension : InstanceExtensionLayers)
    {
        LOG(LogVulkan, "using instance extension \"{}\"", Extension);
    }
#endif
}

void FRenderInstanceManager::CreateInstance(const TDynamicArray<const char8*>& ValidationLayers)
{
    Vk::ApplicationInfo ApplicationInfo{};
    ApplicationInfo.pApplicationName = PROJECT_NAME;
    ApplicationInfo.applicationVersion = VulkanVersion,
    ApplicationInfo.pEngineName = ENGINE_NAME;
    ApplicationInfo.engineVersion = ENGINE_VERSION;
    ApplicationInfo.apiVersion = VulkanAPIVersion;

    Vk::InstanceCreateInfo InstanceInfo;
    InstanceInfo.flags = Vk::InstanceCreateFlagBits{};
    InstanceInfo.pApplicationInfo = &ApplicationInfo;
    InstanceInfo.enabledLayerCount = ValidationLayers.Num();
    InstanceInfo.ppEnabledLayerNames = ValidationLayers.Data();
    InstanceInfo.enabledExtensionCount = InstanceExtensionLayers.Num();
    InstanceInfo.ppEnabledExtensionNames = InstanceExtensionLayers.Data();

#if DEBUG
    const Vk::DebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo{FRenderDebugManager::MakeDebugUtilsMessengerCreateInfo()};
    InstanceInfo.pNext = &DebugMessengerCreateInfo;
#endif

    VulkanInstanceHandle = Vk::createInstance(InstanceInfo);

    LOG(LogVulkan, "created vulkan instance");
}

void FRenderInstanceManager::DestroyInstance()
{
    if(!VulkanInstanceHandle)
    {
        LOGW(LogVulkan, "failed to destroy vulkan instance [its already invalid]");
        return;
    }

    VulkanInstanceHandle.destroy();
    VulkanInstanceHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed vulkan instance");
}
*/
