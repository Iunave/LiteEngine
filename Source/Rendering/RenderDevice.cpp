#include "RenderDevice.hpp"
#include "Assert.hpp"
#include "EngineGlobals.hpp"
#include "RenderWindow.hpp"

#define GLFW_INCLUDE_NONE
namespace Glfw
{
#include "GLFW/glfw3.h"
}
#undef GLFW_INCLUDE_NONE
/*
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    LOG(LogVulkanVL, "validation-layer: {}", pCallbackData->pMessage);
    return VK_FALSE;
}
*/
FQueueFamilyIndices::FQueueFamilyIndices()
    : Graphic{UINT32_MAX}
    , Compute{UINT32_MAX}
    , Protected{UINT32_MAX}
    , SparseBinding{UINT32_MAX}
    , Transfer{UINT32_MAX}
    , Presentation{UINT32_MAX}
{
}

#if USE_VULKAN_VALIDATION_LAYERS
FRenderDevice::FRenderDevice(TDynamicArray<const char8*>&& RequiredDeviceExtensions, TDynamicArray<const char8*>&& RequestedValidationLayers)
    : DeviceExtensions{Move(RequiredDeviceExtensions)}
    , ValidationLayers{Move(RequestedValidationLayers)}
    , DebugMessenger{NULL_HANDLE}
    , VulkanInstance{NULL_HANDLE}
    , PhysicalDevice{NULL_HANDLE}
    , LogicalDevice{NULL_HANDLE}
    , Surface{NULL_HANDLE}
    , GraphicsQueue{NULL_HANDLE}
    , PresentationQueue{NULL_HANDLE}
    , CommandPool{NULL_HANDLE}
{
}
#else
FRenderDevice::FRenderDevice(TArray<const char8*>&& RequiredDeviceExtensions)
    : DeviceExtensions{Move(RequiredDeviceExtensions)}
    , VulkanInstance{NULL_HANDLE}
    , PhysicalDevice{NULL_HANDLE}
    , LogicalDevice{NULL_HANDLE}
    , Surface{NULL_HANDLE}
    , GraphicsQueue{NULL_HANDLE}
    , PresentationQueue{NULL_HANDLE}
    , CommandPool{NULL_HANDLE}
{
}
#endif

void FRenderDevice::Initialize()
{
    CreateInstance();

#if USE_VULKAN_VALIDATION_LAYERS
    SetupDebugMessenger();
#endif

    CreateSurface();

    ChoosePhysicalDevice();

    CreateLogicalDevice();

    CreateQueues();

    CreateCommandPool();
}

void FRenderDevice::ShutDown()
{
    if(LogicalDevice)
    {
        if(CommandPool)
        {
            LogicalDevice.destroyCommandPool(CommandPool);
        }

        LogicalDevice.destroy();
    }
/*
#if USE_VULKAN_VALIDATION_LAYERS
    PFN_vkDestroyDebugUtilsMessengerEXT Function{reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(VulkanInstance, "vkDestroyDebugUtilsMessengerEXT"))};

    if(Function && DebugMessenger)
    {
        Function(VulkanInstance, DebugMessenger, nullptr);
    }
#endif
*/
    if(VulkanInstance)
    {
        if(Surface)
        {
            VulkanInstance.destroySurfaceKHR(Surface);
        }

        VulkanInstance.destroy();
    }
}

FRenderDevice::~FRenderDevice()
{
}

#if USE_VULKAN_VALIDATION_LAYERS
const TDynamicArray<const char8*>& FRenderDevice::GetValidationLayers() const
{
    return ValidationLayers;
}
#endif

const TDynamicArray<const char8*>& FRenderDevice::GetDeviceExtensions() const
{
    return DeviceExtensions;
}

Vk::Instance FRenderDevice::GetVulkanInstance() const
{
    return VulkanInstance;
}

Vk::PhysicalDevice FRenderDevice::GetPhysicalDevice() const
{
    return PhysicalDevice;
}

Vk::Device FRenderDevice::GetLogicalDevice() const
{
    return LogicalDevice;
}

Vk::SurfaceKHR FRenderDevice::GetSurface() const
{
    return Surface;
}

Vk::Queue FRenderDevice::GetGraphicsQueue() const
{
    return GraphicsQueue;
}

Vk::Queue FRenderDevice::GetPresentationQueue() const
{
    return PresentationQueue;
}

Vk::CommandPool FRenderDevice::GetCommandPool() const
{
    return CommandPool;
}

FQueueFamilyIndices FRenderDevice::GetQueueFamilyIndices() const
{
    return QueueFamilyIndices;
}

const FSwapChainSupportDetails& FRenderDevice::GetSwapChainSupportDetails() const
{
    return SwapChainSupportDetails;
}

void FRenderDevice::CreateInstance()
{
    uint32 ApiVersion{0};
    Vk::Result Result{Vk::enumerateInstanceVersion(&ApiVersion)};
    ASSERT(Result == Vk::Result::eSuccess, "could not enumerate instance version {}", Vk::to_string(Result));

    Vk::ApplicationInfo ApplicationInfo{};
    ApplicationInfo.pApplicationName = GProjectName;
    ApplicationInfo.applicationVersion = Vk::VulkanVersion;
    ApplicationInfo.pEngineName = GEngineName;
    ApplicationInfo.engineVersion = GEngineVersion;
    ApplicationInfo.apiVersion = ApiVersion;

    uint32 GLFWExtensionCount{0};
    const char8** GLFWExtensionNames{Glfw::glfwGetRequiredInstanceExtensions(&GLFWExtensionCount)};

#if USE_VULKAN_VALIDATION_LAYERS

    const TDynamicArray<Vk::LayerProperties> FoundValidationLayers{FindAvailableValidationLayers()};

    auto IsLayerAvailable = [this, &FoundValidationLayers](const int64 Index) -> bool
    {
        for(const Vk::LayerProperties& LayerProperty : FoundValidationLayers)
        {
            if(Memory::StringCompare(ValidationLayers[Index], LayerProperty.layerName) == 0)
            {
                return true;
            }
        }
        return false;
    };

    for(int64 Index{0}; Index < ValidationLayers.Num(); ++Index)
    {
        if(!IsLayerAvailable(Index))
        {
            LOG(LogVulkan, "removed validation-layer: \"{}\" as it was not available", ValidationLayers[Index]);
            ValidationLayers.RemoveAtSwap(Index);
        }
        else
        {
            LOG(LogVulkan, "using validation-layer: {}", ValidationLayers[Index]);
        }
    }

#endif //DEBUG

    const Vk::InstanceCreateFlagBits InstanceCreateFlags{};

    Vk::InstanceCreateInfo InstanceInfo
    {
        InstanceCreateFlags,
        &ApplicationInfo,
        #if USE_VULKAN_VALIDATION_LAYERS
        ValidationLayers.Num<uint32>(),
        ValidationLayers.GetData(),
        #else
        0,
        nullptr,
        #endif //USE_VULKAN_VALIDATION_LAYERS
        GLFWExtensionCount,
        GLFWExtensionNames
    };

    VulkanInstance = Vk::createInstance(InstanceInfo);

    ASSERT(VulkanInstance);
    LOG(LogVulkan, "created vulkan-instance");
}

#if USE_VULKAN_VALIDATION_LAYERS
Vk::Result FRenderDevice::SetupDebugMessenger()
{
    PFN_vkCreateDebugUtilsMessengerEXT Function{reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(VulkanInstance,"vkCreateDebugUtilsMessengerEXT"))};

    if(Function)
    {
        Vk::DebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo{};

        VkDebugUtilsMessengerEXT Messenger{DebugMessenger};
        Vk::Result Result{Function(VulkanInstance, &DebugUtilsMessengerCreateInfo.operator VkDebugUtilsMessengerCreateInfoEXT&(), nullptr, &Messenger)};
        DebugMessenger = Messenger;

        return Result;
    }

    return Vk::Result::eErrorExtensionNotPresent;
}
#endif

void FRenderDevice::CreateSurface()
{
    ASSERT(VulkanInstance && GRenderWindow.GetWindow());

    VkSurfaceKHR SurfaceHandle{Surface};

    Vk::Result Result{static_cast<Vk::Result>(Glfw::glfwCreateWindowSurface(VulkanInstance, GRenderWindow.GetWindow(), nullptr, &SurfaceHandle))};
    ASSERT(Result == Vk::Result::eSuccess, "could not create surface {}", Vk::to_string(Result));

    Surface = SurfaceHandle;

    LOG(LogVulkan, "created surface");
}

void FRenderDevice::ChoosePhysicalDevice()
{
    ASSERT(VulkanInstance);

    TDynamicArray<Vk::PhysicalDevice> PhysicalDevices{FindAvailablePhysicalDevices()};

    auto FindPhysicalDevice = [&PhysicalDevices, this]<bool(*Check)(Vk::PhysicalDevice)>() -> bool
    {
        for(int64 Index{0}; Index < PhysicalDevices.Num(); ++Index)
        {
            PhysicalDevice = PhysicalDevices[Index];

            if((Check == nullptr) || (*Check)(PhysicalDevice))
            {
                if(DoesDeviceSupportExtensions())
                {
                    QueueFamilyIndices = QueryQueueFamilyIndices();

                    if(FQueueFamilyIndices::CheckValidity(QueueFamilyIndices.Graphic, QueueFamilyIndices.Presentation))
                    {
                        SwapChainSupportDetails = QuerySwapChainSupportDetails();

                        if(SwapChainSupportDetails.SurfaceFormats.Num() > 0 && SwapChainSupportDetails.PresentModes.Num() > 0)
                        {
                            break;
                        }
                    }
                }
            }

            PhysicalDevice = NULL_HANDLE;
        }

        return !!PhysicalDevice;
    };

    constexpr auto IsDiscreteGPU = [](Vk::PhysicalDevice Device) -> bool
    {
        return Device.getProperties().deviceType == Vk::PhysicalDeviceType::eDiscreteGpu;
    };

    if(!FindPhysicalDevice.operator()<IsDiscreteGPU>())
    {
        FindPhysicalDevice.operator()<nullptr>();
    }

    ASSERT(PhysicalDevice);
    LOG(LogVulkan, "using physical-device: {}", PhysicalDevice.getProperties().deviceName);
}

void FRenderDevice::CreateLogicalDevice()
{
    ASSERT(PhysicalDevice && FQueueFamilyIndices::CheckValidity(QueueFamilyIndices.Graphic, QueueFamilyIndices.Presentation));

    float32 QueuePriority{1.f};

    const TStaticArray<Vk::DeviceQueueCreateInfo, 2> DeviceQueueInfos
    {
        Vk::DeviceQueueCreateInfo{Vk::DeviceQueueCreateFlagBits{}, QueueFamilyIndices.Graphic, static_cast<uint32>(1), &QueuePriority},
        Vk::DeviceQueueCreateInfo{Vk::DeviceQueueCreateFlagBits{}, QueueFamilyIndices.Presentation, static_cast<uint32>(1), &QueuePriority}
    };

    const Vk::PhysicalDeviceFeatures DeviceFeatures{PhysicalDevice.getFeatures()};

    Vk::DeviceCreateInfo DeviceCreateInfo{};
    DeviceCreateInfo.flags = Vk::DeviceCreateFlagBits{};
    DeviceCreateInfo.queueCreateInfoCount = DeviceQueueInfos.Num();
    DeviceCreateInfo.pQueueCreateInfos = DeviceQueueInfos.GetData();
    DeviceCreateInfo.enabledLayerCount = 0;
    DeviceCreateInfo.ppEnabledLayerNames = nullptr;
    DeviceCreateInfo.enabledExtensionCount = DeviceExtensions.Num();
    DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.GetData();
    DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;

#if DEBUG
    for(const char8* CharString : DeviceExtensions)
    {
        LOG(LogVulkan, "using device-extension: {}", CharString);
    }
#endif //DEBUG

    LogicalDevice = PhysicalDevice.createDevice(DeviceCreateInfo);

    ASSERT(LogicalDevice);
    LOG(LogVulkan, "created logical device");
}

void FRenderDevice::CreateQueues()
{
    ASSERT(LogicalDevice && FQueueFamilyIndices::CheckValidity(QueueFamilyIndices.Graphic, QueueFamilyIndices.Presentation));

    GraphicsQueue = LogicalDevice.getQueue(QueueFamilyIndices.Graphic, 0);

    ASSERT(GraphicsQueue);
    LOG(LogVulkan, "created graphics-queue");

    PresentationQueue = LogicalDevice.getQueue(QueueFamilyIndices.Presentation, 0);

    ASSERT(PresentationQueue);
    LOG(LogVulkan, "created presentation-queue");
}

void FRenderDevice::CreateCommandPool()
{
    ASSERT(FQueueFamilyIndices::CheckValidity(QueueFamilyIndices.Graphic));

    const Vk::CommandPoolCreateFlags Flags{0};
    const Vk::CommandPoolCreateInfo CommandPoolInfo{Flags, QueueFamilyIndices.Graphic};

    CommandPool = LogicalDevice.createCommandPool(CommandPoolInfo);

    ASSERT(CommandPool);
    LOG(LogVulkan, "created command-pool");
}

bool FRenderDevice::DoesDeviceSupportExtensions() const
{
    ASSERT(PhysicalDevice);

    if(DeviceExtensions.Num() <= 0)
    {
        return true;
    }

    const TDynamicArray<Vk::ExtensionProperties> AvailableExtensions{FindAvailableExtensions()};

    auto FindExtension = [&AvailableExtensions](const char8* Name) -> bool
    {
        for(const Vk::ExtensionProperties& Property : AvailableExtensions)
        {
            if(Memory::StringCompare(Property.extensionName, Name) == 0)
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

TDynamicArray<Vk::PhysicalDevice> FRenderDevice::FindAvailablePhysicalDevices() const
{
    ASSERT(VulkanInstance);

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

TDynamicArray<Vk::LayerProperties> FRenderDevice::FindAvailableValidationLayers()
{
    TDynamicArray<Vk::LayerProperties> Layers{};

    uint32 LayerPropertyCount{0};
    Vk::Result Result{Vk::enumerateInstanceLayerProperties(&LayerPropertyCount, nullptr)};
    ASSERT(Result == Vk::Result::eSuccess, "failed to enumerate layers: {}", Vk::to_string(Result));

    if(LayerPropertyCount > 0)
    {
        Layers.ResizeTo(LayerPropertyCount);

        Result = Vk::enumerateInstanceLayerProperties(&LayerPropertyCount, Layers.GetData());
        ASSERT(Result == Vk::Result::eSuccess, "failed to enumerate layers: {}", Vk::to_string(Result));
    }

    return Layers;
}

TDynamicArray<Vk::ExtensionProperties> FRenderDevice::FindAvailableExtensions() const
{
    ASSERT(PhysicalDevice);

    TDynamicArray<Vk::ExtensionProperties> FoundExtensions{};

    uint32 ExtensionCount{0};
    Vk::Result Result{PhysicalDevice.enumerateDeviceExtensionProperties(nullptr, &ExtensionCount, nullptr)};
    ASSERT(Result == Vk::Result::eSuccess, "failed to enumerate extensions: {}", Vk::to_string(Result));

    if(ExtensionCount > 0)
    {
        FoundExtensions.ResizeTo(ExtensionCount);

        Result = PhysicalDevice.enumerateDeviceExtensionProperties(nullptr, &ExtensionCount, FoundExtensions.GetData());
        ASSERT(Result == Vk::Result::eSuccess, "failed to enumerate extensions: {}", Vk::to_string(Result));
    }

    return FoundExtensions;
}

TDynamicArray<Vk::QueueFamilyProperties> FRenderDevice::FindQueueFamilyProperties() const
{
    ASSERT(PhysicalDevice);

    TDynamicArray<Vk::QueueFamilyProperties> QueueProperties{};

    uint32 QueueFamilyPropertyCount{0};
    PhysicalDevice.getQueueFamilyProperties(&QueueFamilyPropertyCount, nullptr);

    if(QueueFamilyPropertyCount > 0)
    {
        QueueProperties.ResizeTo(QueueFamilyPropertyCount);

        PhysicalDevice.getQueueFamilyProperties(&QueueFamilyPropertyCount, QueueProperties.GetData());
    }

    return QueueProperties;
}

FQueueFamilyIndices FRenderDevice::QueryQueueFamilyIndices() const
{
    ASSERT(PhysicalDevice && Surface);

    FQueueFamilyIndices QueueIndices{};

    TDynamicArray<Vk::QueueFamilyProperties> QueueProperties{FindQueueFamilyProperties()};

    for(int32 Index{0}; Index < QueueProperties.Num(); ++Index)
    {
        const Vk::Flags<Vk::QueueFlagBits> QueueFlags{QueueProperties[Index].queueFlags};

        if(PhysicalDevice.getSurfaceSupportKHR(Index, Surface))
        {
            QueueIndices.Presentation = Index;
        }
        if(QueueFlags & Vk::QueueFlagBits::eGraphics)
        {
            QueueIndices.Graphic = Index;
        }
        if(QueueFlags & Vk::QueueFlagBits::eCompute)
        {
            QueueIndices.Compute = Index;
        }
        if(QueueFlags & Vk::QueueFlagBits::eProtected)
        {
            QueueIndices.Protected = Index;
        }
        if(QueueFlags & Vk::QueueFlagBits::eSparseBinding)
        {
            QueueIndices.SparseBinding = Index;
        }
        if(QueueFlags & Vk::QueueFlagBits::eTransfer)
        {
            QueueIndices.Transfer = Index;
        }
    }

    return QueueIndices;
}

Vk::SurfaceCapabilitiesKHR FRenderDevice::FindSurfaceCapabilities() const
{
    ASSERT(PhysicalDevice && Surface);

    return PhysicalDevice.getSurfaceCapabilitiesKHR(Surface);
}

TDynamicArray<Vk::SurfaceFormatKHR> FRenderDevice::FindSurfaceFormats() const
{
    ASSERT(PhysicalDevice && Surface);

    TDynamicArray<Vk::SurfaceFormatKHR> FoundFormats{};

    uint32 SurfaceFormatCount{0};
    Vk::Result Result{PhysicalDevice.getSurfaceFormatsKHR(Surface, &SurfaceFormatCount, nullptr)};
    ASSERT(Result == Vk::Result::eSuccess, "failed to get surface formats: {}", Vk::to_string(Result));

    if(SurfaceFormatCount >= 0)
    {
        FoundFormats.ResizeTo(SurfaceFormatCount);

        Result = PhysicalDevice.getSurfaceFormatsKHR(Surface, &SurfaceFormatCount, FoundFormats.GetData());
        ASSERT(Result == Vk::Result::eSuccess, "failed to get surface formats: {}", Vk::to_string(Result));
    }

    return FoundFormats;
}

TDynamicArray<Vk::PresentModeKHR> FRenderDevice::FindSurfacePresentModes() const
{
    ASSERT(PhysicalDevice && Surface);

    TDynamicArray<Vk::PresentModeKHR> FoundModes{};

    uint32 PresentModeCount{0};
    Vk::Result Result{PhysicalDevice.getSurfacePresentModesKHR(Surface, &PresentModeCount, nullptr)};
    ASSERT(Result == Vk::Result::eSuccess, "failed to get present modes: {}", Vk::to_string(Result));

    if(PresentModeCount >= 0)
    {
        FoundModes.ResizeTo(PresentModeCount);

        Result = PhysicalDevice.getSurfacePresentModesKHR(Surface, &PresentModeCount, FoundModes.GetData());
        ASSERT(Result == Vk::Result::eSuccess, "failed to get present modes: {}", Vk::to_string(Result));
    }

    return FoundModes;
}

FSwapChainSupportDetails FRenderDevice::QuerySwapChainSupportDetails() const
{
    return FSwapChainSupportDetails{FindSurfaceCapabilities(), FindSurfaceFormats(), FindSurfacePresentModes()};
}

TDynamicArray<Vk::CommandBuffer> FRenderDevice::BeginCommandBuffers(const int64 NumCommandBuffers) const
{
    TDynamicArray<Vk::CommandBuffer> CommandBuffers{};
    CommandBuffers.ResizeTo(NumCommandBuffers);

    Vk::CommandBufferAllocateInfo CommandBufferAllocateInfo{CommandPool, Vk::CommandBufferLevel::ePrimary, (uint32)NumCommandBuffers};

    const Vk::Result Result{LogicalDevice.allocateCommandBuffers(&CommandBufferAllocateInfo, CommandBuffers.GetData())};
    ASSERT(Result == Vk::Result::eSuccess, "could not allocate command buffers", Vk::to_string(Result));

    static constexpr Vk::CommandBufferUsageFlags Flags{0};
    static constexpr Vk::CommandBufferBeginInfo BeginInfo{Flags, nullptr};

    for(Vk::CommandBuffer Buffer : CommandBuffers)
    {
        Buffer.begin(BeginInfo);
    }

    return CommandBuffers;
}

void FRenderDevice::EndCommandBuffers(TDynamicArray<Vk::CommandBuffer>& CommandBuffers) const
{
    for(Vk::CommandBuffer Buffer : CommandBuffers)
    {
        Buffer.end();
    }
}

