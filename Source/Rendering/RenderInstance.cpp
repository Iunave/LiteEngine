#include "RenderInstance.hpp"

using namespace Render;

FInstanceManager::FInstanceManager()
    : VulkanInstanceHandle{NULL_HANDLE}
{
}

FInstanceManager::~FInstanceManager()
{
    ASSERT(!VulkanInstanceHandle, "vulkan instance handle was not destroyed");
}

void FInstanceManager::PopulateExtensionLayers()
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

TDynamicArray<Vk::PhysicalDevice> FInstanceManager::FindAvailablePhysicalDevices() const
{
    TDynamicArray<Vk::PhysicalDevice> PhysicalDeviceArray{};

    uint32 PhysicalDeviceCount{0};
    Vk::Result EnumerateResult{VulkanInstanceHandle.enumeratePhysicalDevices(&PhysicalDeviceCount, nullptr)};
    ASSERT(EnumerateResult == Vk::Result::eSuccess, "failed to enumerate physical devices: {}", Vk::to_string(EnumerateResult));

    if(PhysicalDeviceCount > 0)
    {
        PhysicalDeviceArray.ResizeTo(PhysicalDeviceCount);

        EnumerateResult = VulkanInstanceHandle.enumeratePhysicalDevices(&PhysicalDeviceCount, PhysicalDeviceArray.GetData());
        ASSERT(EnumerateResult == Vk::Result::eSuccess, "failed to enumerate physical devices: {}", Vk::to_string(EnumerateResult));
    }

    return PhysicalDeviceArray;
}

void FInstanceManager::CreateInstance(const TDynamicArray<const char8*>& ValidationLayers)
{
    Vk::ApplicationInfo ApplicationInfo{};
    ApplicationInfo.pApplicationName = PROJECT_NAME;
    ApplicationInfo.applicationVersion = Data::VulkanVersion,
    ApplicationInfo.pEngineName = ENGINE_NAME;
    ApplicationInfo.engineVersion = ENGINE_VERSION;
    ApplicationInfo.apiVersion = Data::VulkanAPIVersion;

    Vk::InstanceCreateInfo InstanceInfo;
    InstanceInfo.flags = Vk::InstanceCreateFlagBits{};
    InstanceInfo.pApplicationInfo = &ApplicationInfo;
    InstanceInfo.enabledLayerCount = ValidationLayers.Num();
    InstanceInfo.ppEnabledLayerNames = ValidationLayers.Data();
    InstanceInfo.enabledExtensionCount = InstanceExtensionLayers.Num();
    InstanceInfo.ppEnabledExtensionNames = InstanceExtensionLayers.Data();

#if DEBUG
    const Vk::DebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo{FDebugManager::MakeDebugUtilsMessengerCreateInfo()};
    InstanceInfo.pNext = &DebugMessengerCreateInfo;
#endif

    VulkanInstanceHandle = Vk::createInstance(InstanceInfo);

    LOG(LogVulkan, "created vulkan instance");
}

void FInstanceManager::DestroyInstance()
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
