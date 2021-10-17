#include "RenderDevice.hpp"
#include "CoreFiles/Assert.hpp"
#include "CoreFiles/Log.hpp"

/*
FQueueFamilyIndices::FQueueFamilyIndices()
    : Graphic{UINT32_MAX}
    , Compute{UINT32_MAX}
    , Protected{UINT32_MAX}
    , SparseBinding{UINT32_MAX}
    , Transfer{UINT32_MAX}
    , Presentation{UINT32_MAX}
{
}

FRenderDeviceManager::FRenderDeviceManager()
    : PhysicalDeviceHandle{NULL_HANDLE}
    , LogicalDeviceHandle{NULL_HANDLE}
    , GraphicsQueue{NULL_HANDLE}
    , PresentationQueue{NULL_HANDLE}
    , ComputeQueue{NULL_HANDLE}
    , DeviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME}
{
}

FRenderDeviceManager::~FRenderDeviceManager()
{
    ASSERT(!PhysicalDeviceHandle, "physical device was never destroyed");
    ASSERT(!LogicalDeviceHandle, "logical device was never destroyed");
}

TDynamicArray<Vk::ExtensionProperties> FRenderDeviceManager::FindPhysicalDeviceProperties() const
{
    TDynamicArray<Vk::ExtensionProperties> FoundExtensions{};

    uint32 ExtensionCount{0};
    Vk::Result Result{PhysicalDeviceHandle.enumerateDeviceExtensionProperties(nullptr, &ExtensionCount, nullptr)};
    ASSERT(!!Result, "failed to enumerate extensions: {}", Vk::to_string(Result));

    if(ExtensionCount > 0)
    {
        FoundExtensions.ResizeTo(ExtensionCount);

        Result = PhysicalDeviceHandle.enumerateDeviceExtensionProperties(nullptr, &ExtensionCount, FoundExtensions.GetData());
        ASSERT(!!Result, "failed to enumerate extensions: {}", Vk::to_string(Result));
    }

    return FoundExtensions;
}

TDynamicArray<Vk::QueueFamilyProperties> FRenderDeviceManager::FindQueueFamilyProperties() const
{
    TDynamicArray<Vk::QueueFamilyProperties> QueueProperties{};

    uint32 QueueFamilyPropertyCount{0};
    PhysicalDeviceHandle.getQueueFamilyProperties(&QueueFamilyPropertyCount, nullptr);

    if(QueueFamilyPropertyCount > 0)
    {
        QueueProperties.ResizeTo(QueueFamilyPropertyCount);

        PhysicalDeviceHandle.getQueueFamilyProperties(&QueueFamilyPropertyCount, QueueProperties.GetData());
    }

    return QueueProperties;
}

void FRenderDeviceManager::PopulateQueueFamilyIndices(Vk::SurfaceKHR Surface)
{
    TDynamicArray<Vk::QueueFamilyProperties> QueueProperties{FindQueueFamilyProperties()};

    Vk::ArrayWrapper1D<char8, 256u> DeviceName{PhysicalDeviceHandle.getProperties().deviceName};

    LOG(LogVulkan, "searching queue indices for: {}", DeviceName);

    for(uint32 Index{0}; Index < QueueProperties.Num<uint32>(); ++Index)
    {
        auto WantsBreak = [this]() -> bool
        {
            const bool AreIndicesValid{FQueueFamilyIndices::CheckValidity(QueueFamilyIndices.Presentation, QueueFamilyIndices.Graphic, QueueFamilyIndices.Compute)};
            const bool AreIndicesEqual{QueueFamilyIndices.Presentation == QueueFamilyIndices.Graphic}; //todo how many should be equal?

            return AreIndicesValid && AreIndicesEqual;
        };

        const Vk::Flags<Vk::QueueFlagBits> QueueFlags{QueueProperties[Index].queueFlags};

        if(QueueFlags & Vk::QueueFlagBits::eCompute)
        {
            QueueFamilyIndices.Compute = Index;
        }

        if(WantsBreak())
        {
            break;
        }

        if(PhysicalDeviceHandle.getSurfaceSupportKHR(Index, Surface))
        {
            QueueFamilyIndices.Presentation = Index;
        }

        if(WantsBreak())
        {
            break;
        }

        if(QueueFlags & Vk::QueueFlagBits::eGraphics)
        {
            QueueFamilyIndices.Graphic = Index;
        }

        if(WantsBreak())
        {
            break;
        }
    }

    LOG(LogVulkan, "{} has graphic queue {}", DeviceName, QueueFamilyIndices.Graphic);
    LOG(LogVulkan, "{} has presentation queue {}", DeviceName, QueueFamilyIndices.Presentation);
    LOG(LogVulkan, "{} has compute queue {}", DeviceName, QueueFamilyIndices.Compute);
}

bool FRenderDeviceManager::DoesPhysicalDeviceSupportExtensions() const
{
    if(DeviceExtensions.IsEmpty())
    {
        return true;
    }

    const TDynamicArray<Vk::ExtensionProperties> AvailableExtensions{FindPhysicalDeviceProperties()};

    auto FindExtension = [&AvailableExtensions](const char8* Name) -> bool
    {
        for(const Vk::ExtensionProperties& Property : AvailableExtensions)
        {
            if(Memory::StringCompare(Property.extensionName, Name))
            {
                return true;
            }
        }
        return false;
    };

    for(const char8* ExtensionName : DeviceExtensions)
    {
        if(!FindExtension(ExtensionName))
        {
            return false;
        }
    }

    return true;
}

bool FRenderDeviceManager::DoesPhysicalDeviceSupportFeatures() const
{
    const Vk::PhysicalDeviceFeatures& DeviceFeatures{PhysicalDeviceHandle.getFeatures()};

    return DeviceFeatures.shaderFloat64 && DeviceFeatures.geometryShader;
}

bool FRenderDeviceManager::DoesPhysicalDeviceSupportQueues() const
{
    return FQueueFamilyIndices::CheckValidity(QueueFamilyIndices.Graphic, QueueFamilyIndices.Presentation, QueueFamilyIndices.Compute);
}

bool FRenderDeviceManager::DoesPhysicalDeviceSupportSwapChain() const
{
    return !SwapChainSupportDetails.SurfaceFormats.IsEmpty() && !SwapChainSupportDetails.PresentModes.IsEmpty();
}

Vk::SurfaceCapabilitiesKHR FRenderDeviceManager::FindSurfaceCapabilities(Vk::SurfaceKHR Surface)
{
    return PhysicalDeviceHandle.getSurfaceCapabilitiesKHR(Surface);
}

TDynamicArray<Vk::SurfaceFormatKHR> FRenderDeviceManager::FindSurfaceFormats(Vk::SurfaceKHR Surface)
{
    TDynamicArray<Vk::SurfaceFormatKHR> FoundFormats{};

    uint32 SurfaceFormatCount{0};
    Vk::Result Result{PhysicalDeviceHandle.getSurfaceFormatsKHR(Surface, &SurfaceFormatCount, nullptr)};
    ASSERT(!!Result, "failed to get surface formats: {}", Vk::to_string(Result));

    if(SurfaceFormatCount >= 0)
    {
        FoundFormats.ResizeTo(SurfaceFormatCount);

        Result = PhysicalDeviceHandle.getSurfaceFormatsKHR(Surface, &SurfaceFormatCount, FoundFormats.GetData());
        ASSERT(!!Result, "failed to get surface formats: {}", Vk::to_string(Result));
    }

    return FoundFormats;
}

TDynamicArray<Vk::PresentModeKHR> FRenderDeviceManager::FindSurfacePresentModes(Vk::SurfaceKHR Surface)
{
    TDynamicArray<Vk::PresentModeKHR> FoundModes{};

    uint32 PresentModeCount{0};
    Vk::Result Result{PhysicalDeviceHandle.getSurfacePresentModesKHR(Surface, &PresentModeCount, nullptr)};
    ASSERT(!!Result, "failed to get present modes: {}", Vk::to_string(Result));

    if(PresentModeCount >= 0)
    {
        FoundModes.ResizeTo(PresentModeCount);

        Result = PhysicalDeviceHandle.getSurfacePresentModesKHR(Surface, &PresentModeCount, FoundModes.GetData());
        ASSERT(!!Result, "failed to get present modes: {}", Vk::to_string(Result));
    }

    return FoundModes;
}

void FRenderDeviceManager::PopulateSwapChainSupportDetails(Vk::SurfaceKHR Surface)
{
    SwapChainSupportDetails.PresentModes = FindSurfacePresentModes(Surface);
    SwapChainSupportDetails.SurfaceCapabilities = FindSurfaceCapabilities(Surface);
    SwapChainSupportDetails.SurfaceFormats = FindSurfaceFormats(Surface);
}

TDynamicArray<Vk::PhysicalDevice> FRenderDeviceManager::FindAvailablePhysicalDevices(Vk::Instance VulkanInstance)
{
    TDynamicArray<Vk::PhysicalDevice> PhysicalDeviceArray{};

    uint32 PhysicalDeviceCount{0};
    Vk::Result EnumerateResult{VulkanInstance.enumeratePhysicalDevices(&PhysicalDeviceCount, nullptr)};
    ASSERT(EnumerateResult == Vk::Result::eSuccess, "failed to enumerate physical devices: {}", Vk::to_string(EnumerateResult));

    if(PhysicalDeviceCount > 0)
    {
        PhysicalDeviceArray.ResizeTo(PhysicalDeviceCount);

        EnumerateResult = VulkanInstance.enumeratePhysicalDevices(&PhysicalDeviceCount, PhysicalDeviceArray.GetData());
        ASSERT(EnumerateResult == Vk::Result::eSuccess, "failed to enumerate physical devices: {}", Vk::to_string(EnumerateResult));
    }

    return PhysicalDeviceArray;
}

void FRenderDeviceManager::PickPhysicalDevice(Vk::Instance VulkanInstance, Vk::SurfaceKHR Surface)
{
    TDynamicArray<Vk::PhysicalDevice> AvailablePhysicalDevices{FindAvailablePhysicalDevices(VulkanInstance)};

    auto FindPhysicalDevice = [this, Surface, &AvailablePhysicalDevices]<bool(*Check)(Vk::PhysicalDevice)>() -> bool
    {
        for(int64 Index{0}; Index < AvailablePhysicalDevices.Num(); ++Index)
        {
            PhysicalDeviceHandle = AvailablePhysicalDevices[Index];

            if((Check == nullptr) || (*Check)(PhysicalDeviceHandle))
            {
                if(DoesPhysicalDeviceSupportExtensions() && DoesPhysicalDeviceSupportFeatures())
                {
                    PopulateQueueFamilyIndices(Surface);

                    if(DoesPhysicalDeviceSupportQueues())
                    {
                        PopulateSwapChainSupportDetails(Surface);

                        if(DoesPhysicalDeviceSupportSwapChain())
                        {
                            break;
                        }
                    }
                }
            }

            PhysicalDeviceHandle = NULL_HANDLE;
        }

        return !!PhysicalDeviceHandle;
    };

    constexpr auto IsDiscreteGPU = [](Vk::PhysicalDevice Device) -> bool
    {
        return Device.getProperties().deviceType == Vk::PhysicalDeviceType::eDiscreteGpu;
    };

    if(!FindPhysicalDevice.operator()<IsDiscreteGPU>())
    {
        CHECK(FindPhysicalDevice.operator()<nullptr>(), "failed to find a suitable GPU");
    }

    LOG(LogVulkan, "using physical-device: {}", PhysicalDeviceHandle.getProperties().deviceName);
}

void FRenderDeviceManager::DestroyPhysicalDevice()
{
    PhysicalDeviceHandle = NULL_HANDLE;
}

void FRenderDeviceManager::CreateLogicalDevice(const TDynamicArray<const char8*>& ValidationLayers)
{
    float32 QueuePriority{1.f};
    TCountedArray<Vk::DeviceQueueCreateInfo, 3> DeviceQueueInfos
    {
        Vk::DeviceQueueCreateInfo{{}, QueueFamilyIndices.Graphic, 1u, &QueuePriority},
        Vk::DeviceQueueCreateInfo{{}, QueueFamilyIndices.Compute, 1u, &QueuePriority},
        Vk::DeviceQueueCreateInfo{{}, QueueFamilyIndices.Presentation, 1u, &QueuePriority}
    };

    ArrUtil::RemoveDuplicates(DeviceQueueInfos);

    const Vk::PhysicalDeviceFeatures DeviceFeatures{PhysicalDeviceHandle.getFeatures()};

    Vk::DeviceCreateInfo DeviceCreateInfo{};
    DeviceCreateInfo.flags = Vk::DeviceCreateFlagBits{};
    DeviceCreateInfo.queueCreateInfoCount = DeviceQueueInfos.Num();
    DeviceCreateInfo.pQueueCreateInfos = DeviceQueueInfos.Data();
    DeviceCreateInfo.enabledLayerCount = ValidationLayers.Num();
    DeviceCreateInfo.ppEnabledLayerNames = ValidationLayers.Data();
    DeviceCreateInfo.enabledExtensionCount = DeviceExtensions.Num();
    DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.Data();
    DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;

    LogicalDeviceHandle = PhysicalDeviceHandle.createDevice(DeviceCreateInfo);

    ASSERT(LogicalDeviceHandle);
    LOG(LogVulkan, "created logical device");
}

void FRenderDeviceManager::DestroyLogicalDevice()
{
    if(!LogicalDeviceHandle)
    {
        LOGW(LogVulkan, "failed to destroy logical device [its already invalid]");
        return;
    }

    LogicalDeviceHandle.destroy();
    LogicalDeviceHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed logical device");
}

void FRenderDeviceManager::InitializeQueues()
{
    GraphicsQueue = LogicalDeviceHandle.getQueue(QueueFamilyIndices.Graphic, 0);
    PresentationQueue = LogicalDeviceHandle.getQueue(QueueFamilyIndices.Presentation, 0);
    ComputeQueue = LogicalDeviceHandle.getQueue(QueueFamilyIndices.Compute, 0);

    ASSERT(GraphicsQueue && PresentationQueue && ComputeQueue);
    LOG(LogVulkan, "created graphics, presentation and compute queues");
}
*/
