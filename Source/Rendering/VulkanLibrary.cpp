#include "VulkanLibrary.hpp"
#include "CoreFiles/Log.hpp"
#include "Vertex.hpp"
#include <fmt/os.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifndef VK_KHR_VALIDATION_LAYER_NAME
#define VK_KHR_VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"
#endif

inline bool operator!(const Vk::Result VulkanResult)
{
    return VulkanResult != Vk::Result::eSuccess;
}

inline constexpr FString RelativeShaderPath{"/Shaders/Compiled/"};
inline constexpr FString FragShaderPostfix{".frag.spv"};
inline constexpr FString VertShaderPostfix{".vert.spv"};

const uint32 Render::VulkanVersion{VK_MAKE_VERSION(1,2,172)};
const uint32 Render::VulkanAPIVersion{[]()
{
    uint32 VulkanAPIVersion;
    Vk::Result Result{vk::enumerateInstanceVersion(&VulkanAPIVersion)};
    ENSURE(!!Result, "could not enumerate instance version [{}]", Vk::to_string(Result));
    return VulkanAPIVersion;
}()};

TDynamicArray<const char8*> Render::FindValidationLayers()
{
    TDynamicArray<const char8*> FoundValidationLayers{};
#if DEBUG
    FoundValidationLayers.Append(VK_KHR_VALIDATION_LAYER_NAME);

    auto FindAvailableValidationLayerProperties = []() -> TDynamicArray<Vk::LayerProperties>
    {
        TDynamicArray<Vk::LayerProperties> Layers{};

        uint32 LayerPropertyCount{0};
        Vk::Result Result{Vk::enumerateInstanceLayerProperties(&LayerPropertyCount, nullptr)};
        ASSERT(!!Result, "failed to enumerate layers: [{}]", Vk::to_string(Result));

        if(LayerPropertyCount > 0)
        {
            Layers.ResizeTo(LayerPropertyCount);

            Result = Vk::enumerateInstanceLayerProperties(&LayerPropertyCount, Layers.GetData());
            ASSERT(!!Result, "failed to enumerate layers: {}", Vk::to_string(Result));
        }

        return Layers;
    };

    const TDynamicArray<Vk::LayerProperties> FoundValidationLayerProperties{FindAvailableValidationLayerProperties()};

    auto IsLayerAvailable = [&FoundValidationLayerProperties, &FoundValidationLayers](const int64 Index) -> bool
    {
        for(const Vk::LayerProperties& LayerProperty : FoundValidationLayerProperties)
        {
            if(Memory::StringCompare(FoundValidationLayers[Index], LayerProperty.layerName))
            {
                return true;
            }
        }
        return false;
    };

    for(int64 Index{0}; Index < FoundValidationLayers.Num(); ++Index)
    {
        ASSERT(IsLayerAvailable(Index), "validation layer {} was requested but is not available", FoundValidationLayers[Index]);
        LOG(LogVulkan, "using validation-layer: \"{}\"", FoundValidationLayers[Index]);
    }
#endif
    return FoundValidationLayers;
}

TDynamicArray<const char8*> Render::FindInstanceExtensionLayers()
{
    TDynamicArray<const char8*> FoundInstanceExtensionLayers{};

    uint32 GLFWExtensionCount{0};
    const char8** GLFWExtensionNames{glfwGetRequiredInstanceExtensions(&GLFWExtensionCount)};

    FoundInstanceExtensionLayers.ReserveUndefined(GLFWExtensionCount + DEBUG);

    for(uint32 Index{0}; Index < GLFWExtensionCount; ++Index)
    {
        FoundInstanceExtensionLayers.Append(GLFWExtensionNames[Index]);
    }
#if DEBUG
    FoundInstanceExtensionLayers.Append(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    for(const char8* Extension : FoundInstanceExtensionLayers)
    {
        LOG(LogVulkan, "using instance extension \"{}\"", Extension);
    }
#endif
    return FoundInstanceExtensionLayers;
}

TDynamicArray<const char8*> Render::FindDeviceExtensionLayers()
{
    return TDynamicArray<const char8*>{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

uint32 Render::VulkanDebugCallback(Vk::DebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, Vk::DebugUtilsMessageTypeFlagsEXT MessageType, const Vk::DebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
{
    switch(MessageSeverity)
    {
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
        {
            break;
        }
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
        {
            break;
        }
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
        {
            LOG_WARNING(LogVulkan, "{}", CallbackData->pMessage);
            break;
        }
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
        {
            LOG_ERROR(LogVulkan, "{}", CallbackData->pMessage);
            CRASH_TRAP;
        }
    }
    return false;
}

Vk::DebugUtilsMessengerCreateInfoEXT Render::MakeDebugUtilsMessengerCreateInfo()
{
    Vk::DebugUtilsMessengerCreateInfoEXT CreateInfo{};
    CreateInfo.messageSeverity = Vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    CreateInfo.messageType = Vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | Vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | Vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    CreateInfo.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(&VulkanDebugCallback);
    CreateInfo.pUserData = nullptr;
    return CreateInfo;
}

Vk::DebugUtilsMessengerEXT Render::CreateDebugMessenger(Vk::Instance VulkanInstance)
{
    Vk::DebugUtilsMessengerEXT DebugMessengerHandle{NULL_HANDLE};
#if DEBUG
    const Vk::DebugUtilsMessengerCreateInfoEXT CreateInfo{MakeDebugUtilsMessengerCreateInfo()};

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXTFunction{reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(VulkanInstance, "vkCreateDebugUtilsMessengerEXT"))};
    ASSERT(vkCreateDebugUtilsMessengerEXTFunction != nullptr);

    Vk::Result Result{vkCreateDebugUtilsMessengerEXTFunction(VulkanInstance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&CreateInfo), nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT_T**>(&DebugMessengerHandle))};
    ASSERT(!!Result);

    LOG(LogVulkan, "created debug messenger");
#endif
    return DebugMessengerHandle;
}

void Render::DestroyDebugMessenger(Vk::Instance VulkanInstance, Vk::DebugUtilsMessengerEXT DebugHandle)
{
#if DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXTFunction{reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(VulkanInstance, "vkDestroyDebugUtilsMessengerEXT"))};
    ASSERT(vkDestroyDebugUtilsMessengerEXTFunction != nullptr);

    vkDestroyDebugUtilsMessengerEXTFunction(VulkanInstance, DebugHandle.operator VkDebugUtilsMessengerEXT_T*(), nullptr);

    LOG(LogVulkan, "destroyed debug messenger");
#endif
}

Vk::Instance Render::CreateInstance(const TDynamicArray<const char8*>& ValidationLayers, const TDynamicArray<const char8*>& InstanceLayers)
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
    InstanceInfo.enabledExtensionCount = InstanceLayers.Num();
    InstanceInfo.ppEnabledExtensionNames = InstanceLayers.Data();

#if DEBUG
    const Vk::DebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo{MakeDebugUtilsMessengerCreateInfo()};
    InstanceInfo.pNext = &DebugMessengerCreateInfo;
#endif

    LOG(LogVulkan, "created vulkan instance");

    return Vk::createInstance(InstanceInfo);
}

TDynamicArray<Vk::ExtensionProperties> Render::FindPhysicalDeviceProperties(Vk::PhysicalDevice DeviceToCheck)
{
    TDynamicArray<Vk::ExtensionProperties> FoundExtensions{};

    uint32 ExtensionCount{0};
    Vk::Result Result{DeviceToCheck.enumerateDeviceExtensionProperties(nullptr, &ExtensionCount, nullptr)};
    ASSERT(!!Result, "failed to enumerate extensions: {}", Vk::to_string(Result));

    if(ExtensionCount > 0)
    {
        FoundExtensions.ResizeTo(ExtensionCount);

        Result = DeviceToCheck.enumerateDeviceExtensionProperties(nullptr, &ExtensionCount, FoundExtensions.GetData());
        ASSERT(!!Result, "failed to enumerate extensions: {}", Vk::to_string(Result));
    }

    return FoundExtensions;
}

TDynamicArray<Vk::QueueFamilyProperties> Render::FindQueueFamilyProperties(Vk::PhysicalDevice Device)
{
    TDynamicArray<Vk::QueueFamilyProperties> QueueProperties{};

    uint32 QueueFamilyPropertyCount{0};
    Device.getQueueFamilyProperties(&QueueFamilyPropertyCount, nullptr);

    if(QueueFamilyPropertyCount > 0)
    {
        QueueProperties.ResizeTo(QueueFamilyPropertyCount);

        Device.getQueueFamilyProperties(&QueueFamilyPropertyCount, QueueProperties.GetData());
    }

    return QueueProperties;
}

Render::FQueueFamilies Render::PopulateQueueFamilyIndices(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface)
{
    TDynamicArray<Vk::QueueFamilyProperties> QueueProperties{FindQueueFamilyProperties(Device)};

    Vk::ArrayWrapper1D<char8, 256u> DeviceName{Device.getProperties().deviceName};

    LOG(LogVulkan, "searching queue indices for: {}", DeviceName);

    TCountedArray<uint32, 10> PresentationIndices{};
    TCountedArray<uint32, 10> GraphicsIndices{};
    TCountedArray<uint32, 10> ComputeIndices{}; //todo not yet used
    TCountedArray<uint32, 10> TransferIndices{};

    for(uint32 Index{0}; Index < QueueProperties.Num<uint32>(); ++Index)
    {
        const Vk::Flags<Vk::QueueFlagBits> QueueFlags{QueueProperties[Index].queueFlags};

        if(Device.getSurfaceSupportKHR(Index, Surface))
        {
            PresentationIndices.Append(Index);
        }
        if(QueueFlags & Vk::QueueFlagBits::eGraphics)
        {
            GraphicsIndices.Append(Index);
        }
        if(QueueFlags & Vk::QueueFlagBits::eCompute)
        {
            ComputeIndices.Append(Index);
        }
        if(QueueFlags & Vk::QueueFlagBits::eTransfer)
        {
            TransferIndices.Append(Index);
        }
    }

    TCountedArray<uint32, 10> SharedIndices_Present_And_Graphics{};

    for(uint32 PresentIndex : PresentationIndices)
    {
        for(uint32 GraphicsIndex : GraphicsIndices)
        {
            if(PresentIndex == GraphicsIndex)
            {
                SharedIndices_Present_And_Graphics.Append(PresentIndex);
            }
        }
    }

    TCountedArray<uint32, 10> TransferIndices_Unique{};

    if(!SharedIndices_Present_And_Graphics.IsEmpty())
    {
        for(uint32 TransferIndex : TransferIndices)
        {
            for(uint32 SharedIndex : SharedIndices_Present_And_Graphics)
            {
                if(TransferIndex != SharedIndex)
                {
                    TransferIndices_Unique.Append(TransferIndex);
                }
            }
        }
    }
    else
    {
        for(uint32 TransferIndex : TransferIndices)
        {
            for(uint32 PresentIndex : PresentationIndices)
            {
                if(TransferIndex != PresentIndex)
                {
                    for(uint32 GraphicsIndex : GraphicsIndices)
                    {
                        if(TransferIndex != GraphicsIndex)
                        {
                            TransferIndices_Unique.Append(TransferIndex);
                        }
                    }
                }
            }
        }
    }

    FQueueFamilies QueueIndices{};

    if(!SharedIndices_Present_And_Graphics.IsEmpty())
    {
        QueueIndices.Presentation.Index = SharedIndices_Present_And_Graphics[0];
        QueueIndices.Graphic.Index = SharedIndices_Present_And_Graphics[0];
    }
    else
    {
        if(!PresentationIndices.IsEmpty())
        {
            QueueIndices.Presentation.Index = PresentationIndices[0];
        }

        if(!GraphicsIndices.IsEmpty())
        {
            QueueIndices.Graphic.Index = GraphicsIndices[0];
        }
    }

    if(!TransferIndices_Unique.IsEmpty())
    {
        QueueIndices.Transfer.Index = TransferIndices_Unique[0];
    }
    else
    {
        QueueIndices.Transfer.Index = GraphicsIndices[0]; //graphics support transfer
    }

    if(!ComputeIndices.IsEmpty())
    {
        QueueIndices.Compute.Index = ComputeIndices[0];
    }

    LOG(LogVulkan, "{} has graphic queue {}", DeviceName, QueueIndices.Graphic.Index);
    LOG(LogVulkan, "{} has presentation queue {}", DeviceName, QueueIndices.Presentation.Index);
    LOG(LogVulkan, "{} has compute queue {}", DeviceName, QueueIndices.Compute.Index);
    LOG(LogVulkan, "{} has transfer queue {}", DeviceName, QueueIndices.Transfer.Index);

    return QueueIndices;
}

bool Render::DoesPhysicalDeviceSupportExtensions(Vk::PhysicalDevice DeviceToCheck, const TDynamicArray<const char8*>& DeviceExtensionLayers)
{
    if(DeviceExtensionLayers.IsEmpty())
    {
        return true;
    }

    const TDynamicArray<Vk::ExtensionProperties> AvailableExtensions{FindPhysicalDeviceProperties(DeviceToCheck)};

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

    for(const char8* ExtensionName : DeviceExtensionLayers)
    {
        if(!FindExtension(ExtensionName))
        {
            return false;
        }
    }

    return true;
}

bool Render::DoesPhysicalDeviceSupportFeatures(Vk::PhysicalDevice DeviceToCheck)
{
    const Vk::PhysicalDeviceFeatures& DeviceFeatures{DeviceToCheck.getFeatures()};
    return DeviceFeatures.shaderFloat64 && DeviceFeatures.geometryShader;
}

bool Render::AreQueuesSupported(FQueueFamilies QueueFamilies)
{
    return AreQueuesValid(QueueFamilies.Graphic, QueueFamilies.Presentation, QueueFamilies.Compute, QueueFamilies.Transfer);
}

bool Render::IsSwapChainSupported(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats, const TDynamicArray<Vk::PresentModeKHR>& PresentModes)
{
    return !SurfaceFormats.IsEmpty() && !PresentModes.IsEmpty();
}

bool Render::SwapChainNeedsRecreation(Vk::Result Result)
{
    return Result == Vk::Result::eErrorOutOfDateKHR || Result == Vk::Result::eSuboptimalKHR;
}

Vk::SurfaceCapabilitiesKHR Render::FindSurfaceCapabilities(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface)
{
    return Device.getSurfaceCapabilitiesKHR(Surface);
}

TDynamicArray<Vk::SurfaceFormatKHR> Render::FindSurfaceFormats(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface)
{
    TDynamicArray<Vk::SurfaceFormatKHR> FoundFormats{};
    FindSurfaceFormats(Device, Surface, FoundFormats);
    return FoundFormats;
}

void Render::FindSurfaceFormats(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface, TDynamicArray<Vk::SurfaceFormatKHR>& OutFormats)
{
    uint32 SurfaceFormatCount{0};
    Vk::Result Result{Device.getSurfaceFormatsKHR(Surface, &SurfaceFormatCount, nullptr)};
    ASSERT(!!Result, "failed to get surface formats: {}", Vk::to_string(Result));

    if(SurfaceFormatCount >= 0)
    {
        OutFormats.ResizeTo(SurfaceFormatCount);

        Result = Device.getSurfaceFormatsKHR(Surface, &SurfaceFormatCount, OutFormats.GetData());
        ASSERT(!!Result, "failed to get surface formats: {}", Vk::to_string(Result));
    }
}

TDynamicArray<Vk::PresentModeKHR> Render::FindSurfacePresentModes(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface)
{
    TDynamicArray<Vk::PresentModeKHR> FoundModes{};
    FindSurfacePresentModes(Device, Surface, FoundModes);
    return FoundModes;
}

void Render::FindSurfacePresentModes(Vk::PhysicalDevice Device, Vk::SurfaceKHR Surface, TDynamicArray<Vk::PresentModeKHR>& OutPresentModes)
{
    uint32 PresentModeCount{0};
    Vk::Result Result{Device.getSurfacePresentModesKHR(Surface, &PresentModeCount, nullptr)};
    ASSERT(!!Result, "failed to get present modes: {}", Vk::to_string(Result));

    if(PresentModeCount >= 0)
    {
        OutPresentModes.ResizeTo(PresentModeCount);

        Result = Device.getSurfacePresentModesKHR(Surface, &PresentModeCount, OutPresentModes.GetData());
        ASSERT(!!Result, "failed to get present modes: {}", Vk::to_string(Result));
    }
}

TDynamicArray<Vk::PhysicalDevice> Render::FindAvailablePhysicalDevices(Vk::Instance VulkanInstance)
{
    TDynamicArray<Vk::PhysicalDevice> PhysicalDeviceArray{};

    uint32 PhysicalDeviceCount{0};
    Vk::Result EnumerateResult{VulkanInstance.enumeratePhysicalDevices(&PhysicalDeviceCount, nullptr)};
    ASSERT(!!EnumerateResult, "failed to enumerate physical devices: {}", Vk::to_string(EnumerateResult));

    if(PhysicalDeviceCount > 0)
    {
        PhysicalDeviceArray.ResizeTo(PhysicalDeviceCount);

        EnumerateResult = VulkanInstance.enumeratePhysicalDevices(&PhysicalDeviceCount, PhysicalDeviceArray.GetData());
        ASSERT(!!EnumerateResult, "failed to enumerate physical devices: {}", Vk::to_string(EnumerateResult));
    }

    return PhysicalDeviceArray;
}

Vk::SurfaceKHR Render::CreateSurface(Vk::Instance VulkanInstance, GLFWwindow* Window)
{
    VkSurfaceKHR SurfaceHandleResult{NULL_HANDLE};

    Vk::Result Result{static_cast<Vk::Result>(glfwCreateWindowSurface(VulkanInstance, Window, nullptr, &SurfaceHandleResult))};
    ASSERT(!!Result);

    LOG(LogVulkan, "created window surface");

    return SurfaceHandleResult;
}

Vk::PhysicalDevice Render::PickPhysicalDevice(Vk::Instance VulkanInstance, Vk::SurfaceKHR Surface, const TDynamicArray<const char8*>& DeviceExtensionLayers, FQueueFamilies& OutQueueFamilies, Vk::SurfaceCapabilitiesKHR& OutSurfaceCapabilities, TDynamicArray<Vk::SurfaceFormatKHR>& OutSurfaceFormats, TDynamicArray<Vk::PresentModeKHR>& OutPresentModes)
{
    TDynamicArray<Vk::PhysicalDevice> AvailablePhysicalDevices{FindAvailablePhysicalDevices(VulkanInstance)};

    Vk::PhysicalDevice FoundDevice{NULL_HANDLE};

    auto FindPhysicalDevice = [&, Surface](bool(*Check)(Vk::PhysicalDevice)) -> bool
    {
        for(int64 Index{0}; Index < AvailablePhysicalDevices.Num(); ++Index)
        {
            FoundDevice = AvailablePhysicalDevices[Index];

            if(Check == nullptr || Check(FoundDevice))
            {
                if(DoesPhysicalDeviceSupportExtensions(FoundDevice, DeviceExtensionLayers) && DoesPhysicalDeviceSupportFeatures(FoundDevice))
                {
                    OutQueueFamilies = PopulateQueueFamilyIndices(FoundDevice, Surface);

                    if(AreQueuesSupported(OutQueueFamilies))
                    {
                        OutPresentModes = FindSurfacePresentModes(FoundDevice, Surface);
                        OutSurfaceFormats = FindSurfaceFormats(FoundDevice, Surface);
                        OutSurfaceCapabilities = FindSurfaceCapabilities(FoundDevice, Surface);

                        if(IsSwapChainSupported(OutSurfaceFormats, OutPresentModes))
                        {
                            break;
                        }
                    }
                }
            }

            FoundDevice = NULL_HANDLE;
        }

        return !!FoundDevice;
    };

    constexpr auto IsDiscreteGPU = [](Vk::PhysicalDevice Device) -> bool
    {
        return Device.getProperties().deviceType == Vk::PhysicalDeviceType::eDiscreteGpu;
    };

    if(!FindPhysicalDevice(IsDiscreteGPU))
    {
        CHECK(FindPhysicalDevice(nullptr));
    }

    LOG(LogVulkan, "using physical-device: {}", FoundDevice.getProperties().deviceName);

    return FoundDevice;
}

Vk::Device Render::CreateLogicalDevice(Vk::PhysicalDevice PhysicalDevice, FQueueFamilies QueueFamilies, const TDynamicArray<const char8*>& ValidationLayers, const TDynamicArray<const char8*>& DeviceExtensionLayers)
{
    float32 QueuePriority{1.f};
    TCountedArray<Vk::DeviceQueueCreateInfo, 3> DeviceQueueInfos
    {
        Vk::DeviceQueueCreateInfo{{}, QueueFamilies.Graphic.Index, 1u, &QueuePriority},
        Vk::DeviceQueueCreateInfo{{}, QueueFamilies.Presentation.Index, 1u, &QueuePriority},
        Vk::DeviceQueueCreateInfo{{}, QueueFamilies.Transfer.Index, 1u, &QueuePriority}
    };

    ArrUtil::RemoveDuplicates(DeviceQueueInfos);

    const Vk::PhysicalDeviceFeatures DeviceFeatures{PhysicalDevice.getFeatures()};

    Vk::DeviceCreateInfo DeviceCreateInfo{};
    DeviceCreateInfo.flags = Vk::DeviceCreateFlagBits{};
    DeviceCreateInfo.queueCreateInfoCount = DeviceQueueInfos.Num();
    DeviceCreateInfo.pQueueCreateInfos = DeviceQueueInfos.Data();
    DeviceCreateInfo.enabledLayerCount = ValidationLayers.Num();
    DeviceCreateInfo.ppEnabledLayerNames = ValidationLayers.Data();
    DeviceCreateInfo.enabledExtensionCount = DeviceExtensionLayers.Num();
    DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtensionLayers.Data();
    DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;

    LOG(LogVulkan, "created logical device");

    return PhysicalDevice.createDevice(DeviceCreateInfo);
}

TDynamicArray<Vk::Image> Render::FindSwapChainImages(Vk::Device LogicalDevice, Vk::SwapchainKHR SwapChain)
{
    TDynamicArray<Vk::Image> Images{};
    FindSwapChainImages(LogicalDevice, SwapChain, Images);
    return Images;
}

void Render::FindSwapChainImages(Vk::Device LogicalDevice, Vk::SwapchainKHR SwapChain, TDynamicArray<Vk::Image>& OutImages)
{
    uint32 ImageCount{0};
    Vk::Result Result{LogicalDevice.getSwapchainImagesKHR(SwapChain, &ImageCount, nullptr)};
    ASSERT(!!Result, "failed to get swapchain images {}", Vk::to_string(Result));

    if(ImageCount > 0)
    {
        OutImages.ResizeTo(ImageCount);

        Result = LogicalDevice.getSwapchainImagesKHR(SwapChain, &ImageCount, OutImages.Data());
        ASSERT(!!Result, "failed to get swapchain images {}", Vk::to_string(Result));
    }

    LOG(LogVulkan, "populated swapchain images");
}

TDynamicArray<Vk::ImageView> Render::CreateSwapChainImageViews(Vk::Device LogicalDevice, const TDynamicArray<Vk::Image>& Images, Vk::SurfaceFormatKHR SurfaceFormat)
{
    TDynamicArray<Vk::ImageView> ImageViews{};
    ImageViews.ResizeTo(Images.Num());

    Vk::ImageViewCreateInfo ImageViewCreateInfo{};
    ImageViewCreateInfo.viewType = Vk::ImageViewType::e2D;
    ImageViewCreateInfo.format = SurfaceFormat.format;

    ImageViewCreateInfo.components.r = Vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.g = Vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.b = Vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.a = Vk::ComponentSwizzle::eIdentity;

    ImageViewCreateInfo.subresourceRange.aspectMask = Vk::ImageAspectFlagBits::eColor;
    ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    ImageViewCreateInfo.subresourceRange.levelCount = 1;
    ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    ImageViewCreateInfo.subresourceRange.layerCount = 1;

    for(int64 Index{0}; Index < Images.Num(); ++Index)
    {
        ImageViewCreateInfo.image = Images[Index];
        ImageViews[Index] = LogicalDevice.createImageView(ImageViewCreateInfo);

        ASSERT(ImageViews[Index]);
    }

    LOG(LogVulkan, "created image views");

    return ImageViews;
}

Vk::Extent2D Render::ChooseImageExtent(FWindowDimensions WindowDimensions, Vk::SurfaceCapabilitiesKHR SurfaceCapabilities)
{
    const uint32 ClampedWidth{Math::Clamp(static_cast<uint32>(WindowDimensions.PixelWidth), SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width)};
    const uint32 ClampedHeight{Math::Clamp(static_cast<uint32>(WindowDimensions.PixelHeight), SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height)};

    LOG(LogVulkan, "using image width: {} height: {}", ClampedWidth, ClampedHeight);

    return Vk::Extent2D{ClampedWidth, ClampedHeight};
}

Vk::SurfaceFormatKHR Render::ChooseSurfaceFormat(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats)
{
    Vk::SurfaceFormatKHR FoundFormat{SurfaceFormats[0]};

    for(int32 Index{1}; Index < SurfaceFormats.Num(); ++Index)
    {
        if(FoundFormat.format == Vk::Format::eR8G8B8A8Srgb && FoundFormat.colorSpace == Vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            break;
        }

        FoundFormat = SurfaceFormats[Index];
    }

    LOG(LogVulkan, "using color-format: {}", Vk::to_string(FoundFormat.format));
    LOG(LogVulkan, "using color-space: {}", Vk::to_string(FoundFormat.colorSpace));

    return FoundFormat;
}

Vk::PresentModeKHR Render::ChoosePresentationMode(const TDynamicArray<Vk::PresentModeKHR>& PresentModes)
{
    uint32 MailboxFound{0};
    uint32 ImmediateFound{0};

    for(const Vk::PresentModeKHR Mode : PresentModes)
    {
        MailboxFound += (Mode == Vk::PresentModeKHR::eMailbox);
        ImmediateFound += (Mode == Vk::PresentModeKHR::eImmediate);
    }

    if(MailboxFound > 0)
    {
        LOG(LogVulkan, "using presentation-mode: {}", Vk::to_string(Vk::PresentModeKHR::eMailbox));
        return Vk::PresentModeKHR::eMailbox;
    }
    else if(ImmediateFound > 0)
    {
        LOG(LogVulkan, "using presentation-mode: {}", Vk::to_string(Vk::PresentModeKHR::eImmediate));
        return Vk::PresentModeKHR::eImmediate;
    }

    LOG(LogVulkan, "using presentation-mode: {}", Vk::to_string(Vk::PresentModeKHR::eFifo));
    return Vk::PresentModeKHR::eFifo; //is guaranteed to exist
}

template<EStackSize SS>
TDynamicArray<uint8> Render::ReadShaderFile(FString<SS> ShaderFileName)
{
    ShaderFileName.PushBack_Assign(RelativeShaderPath);
    ShaderFileName.PushBack_Assign(StrUtl::GetWorkingDirectory());

    fmt::file InputFile{ShaderFileName.Data(), fmt::file::RDONLY};

    TDynamicArray<uint8> FileContents{};
    FileContents.ResizeTo(InputFile.size());

    LOG(LogVulkan, "reading {}", ShaderFileName);

    InputFile.read(FileContents.Data(), FileContents.Num());
    InputFile.close();

    return FileContents;
}

Vk::ShaderModule Render::CreateShaderModule(Vk::Device LogicalDevice, const TDynamicArray<uint8>& Code)
{
    Vk::ShaderModuleCreateInfo ShaderModuleCreateInfo{};
    ShaderModuleCreateInfo.codeSize = Code.Num();
    ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32*>(Code.Data());

    return LogicalDevice.createShaderModule(ShaderModuleCreateInfo);
}

template<EStackSize SS>
Vk::ShaderModule Render::CreateShaderModule(Vk::Device LogicalDevice, FString<SS> ShaderFileName)
{
    TDynamicArray<uint8> Code{ReadShaderFile(Move(ShaderFileName))};
    return CreateShaderModule(LogicalDevice, Code);
}

Vk::SwapchainKHR
Render::CreateSwapChain(Vk::SwapchainKHR OldSwapChain, Vk::Device LogicalDevice, Vk::SurfaceKHR Surface, Vk::SurfaceCapabilitiesKHR SurfaceCapabilities, FQueueFamilies QueueFamilies, Vk::Extent2D ImageExtent, Vk::PresentModeKHR PresentMode, Vk::SurfaceFormatKHR SurfaceFormat)
{
    uint32 MinImageCount{SurfaceCapabilities.minImageCount + 1};

    if(SurfaceCapabilities.maxImageCount != 0 && MinImageCount > SurfaceCapabilities.maxImageCount)
    {
        MinImageCount = SurfaceCapabilities.maxImageCount;
    }

    TCountedArray<uint32, 3> QueueFamilyIndicesArray{QueueFamilies.Graphic.Index, QueueFamilies.Presentation.Index, QueueFamilies.Transfer.Index};
    ArrUtil::RemoveDuplicates(QueueFamilyIndicesArray);

    const Vk::SwapchainCreateFlagsKHR SwapChainCreateFlags{};
    const Vk::ImageUsageFlags ImageUsageFlags{Vk::ImageUsageFlagBits::eColorAttachment};

    Vk::SwapchainCreateInfoKHR SwapChainCreateInfo{};
    SwapChainCreateInfo.flags = SwapChainCreateFlags;
    SwapChainCreateInfo.surface = Surface;
    SwapChainCreateInfo.minImageCount = MinImageCount;
    SwapChainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    SwapChainCreateInfo.imageFormat = SurfaceFormat.format;
    SwapChainCreateInfo.imageExtent = ImageExtent;
    SwapChainCreateInfo.imageArrayLayers = 1;
    SwapChainCreateInfo.imageUsage = ImageUsageFlags;
    SwapChainCreateInfo.preTransform = SurfaceCapabilities.currentTransform;
    SwapChainCreateInfo.compositeAlpha = Vk::CompositeAlphaFlagBitsKHR::eOpaque;
    SwapChainCreateInfo.presentMode = PresentMode;
    SwapChainCreateInfo.clipped = true;
    SwapChainCreateInfo.oldSwapchain = OldSwapChain;

    if(QueueFamilyIndicesArray.Num() == 1) //all same queue
    {
        SwapChainCreateInfo.imageSharingMode = Vk::SharingMode::eExclusive;
        SwapChainCreateInfo.queueFamilyIndexCount = 0;
        SwapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }
    else
    {
        SwapChainCreateInfo.imageSharingMode = Vk::SharingMode::eConcurrent;
        SwapChainCreateInfo.queueFamilyIndexCount = QueueFamilyIndicesArray.Num();
        SwapChainCreateInfo.pQueueFamilyIndices = QueueFamilyIndicesArray.Data();
    }


    LOG(LogVulkan, "created swapchain");

    return LogicalDevice.createSwapchainKHR(SwapChainCreateInfo);
}

Vk::RenderPass Render::CreateRenderPass(Vk::Device LogicalDevice, Vk::SurfaceFormatKHR SurfaceFormat)
{
    Vk::AttachmentDescription ColorAttachment{};
    ColorAttachment.format = SurfaceFormat.format;
    ColorAttachment.samples = Vk::SampleCountFlagBits::e1;
    ColorAttachment.loadOp = Vk::AttachmentLoadOp::eClear;
    ColorAttachment.storeOp = Vk::AttachmentStoreOp::eStore;
    ColorAttachment.stencilLoadOp = Vk::AttachmentLoadOp::eDontCare;
    ColorAttachment.stencilStoreOp = Vk::AttachmentStoreOp::eDontCare;
    ColorAttachment.initialLayout = Vk::ImageLayout::eUndefined;
    ColorAttachment.finalLayout = Vk::ImageLayout::ePresentSrcKHR;

    Vk::AttachmentReference ColorAttachmentReference{};
    ColorAttachmentReference.attachment = 0;
    ColorAttachmentReference.layout = Vk::ImageLayout::eColorAttachmentOptimal;

    Vk::SubpassDescription SubpassDescription{};
    SubpassDescription.pipelineBindPoint = Vk::PipelineBindPoint::eGraphics;
    SubpassDescription.colorAttachmentCount = 1;
    SubpassDescription.pColorAttachments = &ColorAttachmentReference;

    Vk::SubpassDependency SubpassDependency{};
    SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    SubpassDependency.dstSubpass = 0;
    SubpassDependency.srcStageMask = Vk::PipelineStageFlagBits::eColorAttachmentOutput;
    SubpassDependency.srcAccessMask = Vk::AccessFlagBits{};
    SubpassDependency.dstStageMask = Vk::PipelineStageFlagBits::eColorAttachmentOutput;
    SubpassDependency.dstAccessMask = Vk::AccessFlagBits::eColorAttachmentWrite;

    Vk::RenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.attachmentCount = 1;
    RenderPassInfo.pAttachments = &ColorAttachment;
    RenderPassInfo.subpassCount = 1;
    RenderPassInfo.pSubpasses = &SubpassDescription;
    RenderPassInfo.dependencyCount = 1;
    RenderPassInfo.pDependencies = &SubpassDependency;

    LOG(LogVulkan, "created render pass");

    return LogicalDevice.createRenderPass(RenderPassInfo);
}

Vk::PipelineLayout Render::CreateGraphicsPipelineLayout(Vk::Device LogicalDevice)
{
    Vk::PipelineLayoutCreateInfo PipelineLayoutInfo{};
    PipelineLayoutInfo.setLayoutCount = 0;
    PipelineLayoutInfo.pSetLayouts = nullptr;
    PipelineLayoutInfo.pushConstantRangeCount = 0;
    PipelineLayoutInfo.pPushConstantRanges = nullptr;

    LOG(LogVulkan, "created graphics pipeline layout");

    return LogicalDevice.createPipelineLayout(PipelineLayoutInfo);
}

Vk::Pipeline Render::CreateGraphicsPipeline(Vk::Device LogicalDevice, Vk::RenderPass RenderPass, Vk::PipelineLayout PipelineLayout, Vk::ShaderModule FragShaderModule, Vk::ShaderModule VertShaderModule, Vk::Extent2D ImageExtent)
{
    TStaticArray<Vk::PipelineShaderStageCreateInfo, 2> ShaderStages{};
    ShaderStages[0].stage = Vk::ShaderStageFlagBits::eVertex;
    ShaderStages[0].module = VertShaderModule;
    ShaderStages[0].pName = "main";
    ShaderStages[0].pNext = nullptr;
    ShaderStages[0].pSpecializationInfo = nullptr;

    ShaderStages[1].stage = Vk::ShaderStageFlagBits::eFragment;
    ShaderStages[1].module = FragShaderModule;
    ShaderStages[1].pName = "main";
    ShaderStages[1].pNext = nullptr;
    ShaderStages[1].pSpecializationInfo = nullptr;

    Vk::PipelineVertexInputStateCreateInfo VertexInputInfo{};
    VertexInputInfo.vertexBindingDescriptionCount = 1;
    VertexInputInfo.pVertexBindingDescriptions = &FVertex::BindingDescription;
    VertexInputInfo.vertexAttributeDescriptionCount = FVertex::AttributeDescriptions.Num();
    VertexInputInfo.pVertexAttributeDescriptions = FVertex::AttributeDescriptions.Data();

    Vk::PipelineInputAssemblyStateCreateInfo AssemblyStateInfo{};
    AssemblyStateInfo.topology = Vk::PrimitiveTopology::eTriangleList;
    AssemblyStateInfo.primitiveRestartEnable = false;

    Vk::Viewport Viewport{};
    Viewport.x = 0.f;
    Viewport.y = 0.f;
    Viewport.width = static_cast<float32>(ImageExtent.width);
    Viewport.height = static_cast<float32>(ImageExtent.height);
    Viewport.minDepth = 0.f;
    Viewport.maxDepth = 1.f;

    Vk::Rect2D Scissor{};
    Scissor.offset = Vk::Offset2D{0, 0};
    Scissor.extent = ImageExtent;

    Vk::PipelineViewportStateCreateInfo ViewportInfo{};
    ViewportInfo.scissorCount = 1;
    ViewportInfo.pScissors = &Scissor;
    ViewportInfo.viewportCount = 1;
    ViewportInfo.pViewports = &Viewport;

    Vk::PipelineRasterizationStateCreateInfo RasterizerInfo{};
    RasterizerInfo.depthClampEnable = false;
    RasterizerInfo.rasterizerDiscardEnable = false;
    RasterizerInfo.polygonMode = Vk::PolygonMode::eFill;
    RasterizerInfo.lineWidth = 1.f;
    RasterizerInfo.cullMode = Vk::CullModeFlagBits::eNone;
    RasterizerInfo.frontFace = Vk::FrontFace::eClockwise;
    RasterizerInfo.depthBiasEnable = false;
    RasterizerInfo.depthBiasClamp = 0.f;
    RasterizerInfo.depthBiasConstantFactor = 0.f;
    RasterizerInfo.depthBiasSlopeFactor = 0.f;

    Vk::PipelineMultisampleStateCreateInfo MultisamplerInfo{};
    MultisamplerInfo.sampleShadingEnable = false;
    MultisamplerInfo.rasterizationSamples = Vk::SampleCountFlagBits::e1;
    MultisamplerInfo.minSampleShading = 1.0f;
    MultisamplerInfo.pSampleMask = nullptr;
    MultisamplerInfo.alphaToCoverageEnable = false;
    MultisamplerInfo.alphaToOneEnable = false;

    Vk::PipelineDepthStencilStateCreateInfo DepthStencilInfo{};
    DepthStencilInfo.depthTestEnable = true;
    DepthStencilInfo.depthWriteEnable = true;
    DepthStencilInfo.depthCompareOp = Vk::CompareOp::eLess;
    DepthStencilInfo.depthBoundsTestEnable = false;
    DepthStencilInfo.minDepthBounds = 0.f;
    DepthStencilInfo.maxDepthBounds = 1.f;
    DepthStencilInfo.stencilTestEnable = false;
    DepthStencilInfo.front = Vk::StencilOp::eZero;
    DepthStencilInfo.back = Vk::StencilOp::eZero;

    Vk::PipelineColorBlendAttachmentState ColorBlendAttachmentState{};
    ColorBlendAttachmentState.blendEnable = false;
    ColorBlendAttachmentState.colorWriteMask = Vk::ColorComponentFlagBits::eR | Vk::ColorComponentFlagBits::eG | Vk::ColorComponentFlagBits::eB | Vk::ColorComponentFlagBits::eA;
    ColorBlendAttachmentState.srcColorBlendFactor = Vk::BlendFactor::eSrcAlpha;
    ColorBlendAttachmentState.dstColorBlendFactor = Vk::BlendFactor::eOneMinusSrcAlpha;
    ColorBlendAttachmentState.colorBlendOp = Vk::BlendOp::eAdd;
    ColorBlendAttachmentState.srcAlphaBlendFactor = Vk::BlendFactor::eOne;
    ColorBlendAttachmentState.dstAlphaBlendFactor = Vk::BlendFactor::eZero;
    ColorBlendAttachmentState.alphaBlendOp = Vk::BlendOp::eAdd;

    Vk::PipelineColorBlendStateCreateInfo ColorBlendInfo{};
    ColorBlendInfo.attachmentCount = 1;
    ColorBlendInfo.pAttachments = &ColorBlendAttachmentState;
    ColorBlendInfo.logicOpEnable = false;
    ColorBlendInfo.logicOp = Vk::LogicOp::eCopy;
    ColorBlendInfo.blendConstants = std::array<float32, 4>{0.F, 0.F, 0.F, 0.F};

    TStaticArray<Vk::DynamicState, 2> DynamicStates{Vk::DynamicState::eViewport, Vk::DynamicState::eLineWidth};

    Vk::PipelineDynamicStateCreateInfo DynamicStateInfo{};
    DynamicStateInfo.dynamicStateCount = DynamicStates.Num();
    DynamicStateInfo.pDynamicStates = DynamicStates.GetData();

    Vk::GraphicsPipelineCreateInfo PipelineCreateInfo{};
    PipelineCreateInfo.stageCount = ShaderStages.Num();
    PipelineCreateInfo.pStages = ShaderStages.Data();
    PipelineCreateInfo.pVertexInputState = &VertexInputInfo;
    PipelineCreateInfo.pInputAssemblyState = &AssemblyStateInfo;
    PipelineCreateInfo.pViewportState = &ViewportInfo;
    PipelineCreateInfo.pRasterizationState = &RasterizerInfo;
    PipelineCreateInfo.pMultisampleState = &MultisamplerInfo;
    PipelineCreateInfo.pDepthStencilState = &DepthStencilInfo;
    PipelineCreateInfo.pColorBlendState = &ColorBlendInfo;
    PipelineCreateInfo.pDynamicState = nullptr;//&DynamicStateInfo; todo not implemented
    PipelineCreateInfo.renderPass = RenderPass;
    PipelineCreateInfo.layout = PipelineLayout;
    PipelineCreateInfo.subpass = 0; //index to subpass
    PipelineCreateInfo.basePipelineIndex = -1;
    PipelineCreateInfo.basePipelineHandle = NULL_HANDLE;

    Vk::Pipeline GraphicsPipeline{NULL_HANDLE};

    Vk::Result Result{LogicalDevice.createGraphicsPipelines(NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &GraphicsPipeline)};
    ASSERT(!!Result, "{}", Vk::to_string(Result));

    LOG(LogVulkan, "created graphics pipeline");

    return GraphicsPipeline;
}

TDynamicArray<Vk::Framebuffer> Render::CreateFrameBuffers(Vk::Device LogicalDevice, Vk::RenderPass RenderPass, Vk::Extent2D ImageExtent, const TDynamicArray<Vk::ImageView>& ImageViews)
{
    TDynamicArray<Vk::Framebuffer> FrameBuffers{};
    FrameBuffers.ResizeTo(ImageViews.Num());

    for(int64 Index{0}; Index < FrameBuffers.Num(); ++Index)
    {
        Vk::FramebufferCreateInfo FrameBufferInfo{};
        FrameBufferInfo.renderPass = RenderPass;
        FrameBufferInfo.attachmentCount = 1;
        FrameBufferInfo.pAttachments = &ImageViews[Index];
        FrameBufferInfo.width = ImageExtent.width;
        FrameBufferInfo.height = ImageExtent.height;
        FrameBufferInfo.layers = 1;

        Vk::Result Result{LogicalDevice.createFramebuffer(&FrameBufferInfo, nullptr, &FrameBuffers[Index])};
        ASSERT(!!Result, "{}", Vk::to_string(Result));
    }

    LOG(LogVulkan, "created frame buffers");

    return FrameBuffers;
}

Vk::CommandPool Render::CreateCommandPool(Vk::Device LogicalDevice, uint32 QueueIndex)
{
    Vk::CommandPoolCreateInfo CommandPoolInfo{};
    CommandPoolInfo.queueFamilyIndex = QueueIndex;
    CommandPoolInfo.flags = Vk::CommandPoolCreateFlagBits{};

    Vk::CommandPool CommandPool{NULL_HANDLE};

    Vk::Result Result{LogicalDevice.createCommandPool(&CommandPoolInfo, nullptr, &CommandPool)};
    ASSERT(!!Result, "{}", Vk::to_string(Result));
    LOG(LogVulkan, "created command pool");

    return CommandPool;
}

Vk::CommandPool Render::CreateCommandPoolTransient(Vk::Device LogicalDevice, uint32 QueueIndex)
{
    Vk::CommandPoolCreateInfo CommandPoolInfo{};
    CommandPoolInfo.queueFamilyIndex = QueueIndex;
    CommandPoolInfo.flags = Vk::CommandPoolCreateFlagBits::eTransient;

    Vk::CommandPool CommandPool{NULL_HANDLE};

    Vk::Result Result{LogicalDevice.createCommandPool(&CommandPoolInfo, nullptr, &CommandPool)};
    ASSERT(!!Result, "{}", Vk::to_string(Result));
    LOG(LogVulkan, "created command pool");

    return CommandPool;
}

Vk::Semaphore Render::CreateSemaphore(Vk::Device LogicalDevice)
{
    Vk::SemaphoreCreateInfo SemaphoreCreateInfo{};

    Vk::Semaphore Semaphore{NULL_HANDLE};

    Vk::Result Result{LogicalDevice.createSemaphore(&SemaphoreCreateInfo, nullptr, &Semaphore)};
    ASSERT(!!Result, "{}", Vk::to_string(Result));

    return Semaphore;
}

Vk::Fence Render::CreateFence(Vk::Device LogicalDevice)
{
    Vk::FenceCreateInfo FenceCreateInfo{};
    FenceCreateInfo.flags = Vk::FenceCreateFlagBits::eSignaled;

    Vk::Fence Fence{NULL_HANDLE};

    Vk::Result Result{LogicalDevice.createFence(&FenceCreateInfo, nullptr, &Fence)};
    ASSERT(!!Result, "{}", Vk::to_string(Result));

    return Fence;
}

Vk::Buffer Render::CreateBuffer(Vk::Device LogicalDevice, Vk::BufferUsageFlags BufferUsage, uint64 NumBytes)
{
    Vk::BufferCreateInfo BufferInfo{};
    BufferInfo.size = NumBytes;
    BufferInfo.usage = BufferUsage;
    BufferInfo.sharingMode = Vk::SharingMode::eExclusive;

    Vk::Buffer Buffer{NULL_HANDLE};

    Vk::Result Result{LogicalDevice.createBuffer(&BufferInfo, nullptr, &Buffer)};
    ASSERT(!!Result, "{}", Vk::to_string(Result));
    LOG(LogVulkan, "created buffer");

    return Buffer;
}

Vk::DeviceMemory Render::AllocateGPUMemory(Vk::PhysicalDevice PhysicalDevice, Vk::Device LogicalDevice, Vk::Buffer& Buffer, Vk::MemoryPropertyFlags MemoryFlags)
{
    Vk::MemoryRequirements MemoryRequirements{};
    LogicalDevice.getBufferMemoryRequirements(Buffer, &MemoryRequirements);

    Vk::MemoryAllocateInfo MemoryInfo{};
    MemoryInfo.allocationSize = MemoryRequirements.size;
    MemoryInfo.memoryTypeIndex = FindMemoryType(PhysicalDevice, MemoryRequirements.memoryTypeBits, MemoryFlags);

    Vk::DeviceMemory DeviceMemory{NULL_HANDLE};

    Vk::Result Result{LogicalDevice.allocateMemory(&MemoryInfo, nullptr, &DeviceMemory)};
    ASSERT(!!Result, "{}", Vk::to_string(Result));
    LOG(LogVulkan, "allocated {} bytes of gpu memory", MemoryRequirements.size);

    LogicalDevice.bindBufferMemory(Buffer, DeviceMemory, 0);

    return DeviceMemory;
}

void Render::CopyDataToGPU(Vk::Device LogicalDevice, Vk::Buffer Buffer, Vk::DeviceMemory DeviceMemory, const void* Data, uint64 NumBytes)
{
    void* DataStart{nullptr};

    Vk::Result Result{LogicalDevice.mapMemory(DeviceMemory, 0, NumBytes, Vk::MemoryMapFlags{}, &DataStart)};
    ASSERT(!!Result, "{}", Vk::to_string(Result));

    Memory::Copy(DataStart, Data, NumBytes);

    LogicalDevice.unmapMemory(DeviceMemory);
}

void Render::CopyBuffer(Vk::Buffer Source, Vk::Buffer Target, Vk::Device LogicalDevice, Vk::CommandPool CommandPool, Vk::Queue SubmitQueue, uint64 NumBytes)
{
    Vk::CommandBufferAllocateInfo AllocationInfo{};
    AllocationInfo.level = Vk::CommandBufferLevel::ePrimary;
    AllocationInfo.commandPool = CommandPool;
    AllocationInfo.commandBufferCount = 1;

    Vk::CommandBuffer CommandBuffer{NULL_HANDLE};
    Vk::Result Result{LogicalDevice.allocateCommandBuffers(&AllocationInfo, &CommandBuffer)};
    ASSERT(!!Result, "{}", Vk::to_string(Result));

    Vk::CommandBufferBeginInfo BeginInfo{};
    BeginInfo.flags = Vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    CommandBuffer.begin(BeginInfo);

    Vk::BufferCopy BufferCopy{};
    BufferCopy.size = NumBytes;
    BufferCopy.dstOffset = 0;
    BufferCopy.srcOffset = 0;

    CommandBuffer.copyBuffer(Source, Target, 1, &BufferCopy);
    CommandBuffer.end();

    Vk::SubmitInfo SubmitInfo{};
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffer;

    Result = SubmitQueue.submit(1, &SubmitInfo, NULL_HANDLE);
    ASSERT(!!Result, "{}", Vk::to_string(Result));

    SubmitQueue.waitIdle();

    LogicalDevice.freeCommandBuffers(CommandPool, 1, &CommandBuffer);
}

uint32 Render::FindMemoryType(Vk::PhysicalDevice PhysicalDevice, uint32 TypeFilter, Vk::MemoryPropertyFlags Flags)
{
    Vk::PhysicalDeviceMemoryProperties MemoryProperties{};
    PhysicalDevice.getMemoryProperties(&MemoryProperties);

    for(uint32 Index{0}; Index < MemoryProperties.memoryTypeCount; ++Index)
    {
        const bool CheckFilter{(TypeFilter & (1U << Index)) > 0};
        const bool CheckFlags{MemoryProperties.memoryTypes[Index].propertyFlags == Flags};

        if(CheckFilter && CheckFlags)
        {
            return Index;
        }
    }

    ASSERT(false, "failed to find suitable memory");
    UNREACHABLE;
}

TDynamicArray<Vk::CommandBuffer> Render::CreateCommandBuffers(Vk::Device LogicalDevice, Vk::CommandPool CommandPool, uint64 NumBuffers)
{
    TDynamicArray<Vk::CommandBuffer> CommandBuffers{};
    CommandBuffers.ResizeTo(NumBuffers);

    Vk::CommandBufferAllocateInfo CommandBufferInfo{};
    CommandBufferInfo.commandPool = CommandPool;
    CommandBufferInfo.level = Vk::CommandBufferLevel::ePrimary;
    CommandBufferInfo.commandBufferCount = CommandBuffers.Num();

    Vk::Result Result{LogicalDevice.allocateCommandBuffers(&CommandBufferInfo, CommandBuffers.Data())};
    ASSERT(!!Result, "{}", Vk::to_string(Result));

    return CommandBuffers;
}

Vk::CommandBuffer Render::CreateCommandBuffer(Vk::Device LogicalDevice, Vk::CommandPool CommandPool)
{
    Vk::CommandBuffer CommandBuffer{NULL_HANDLE};

    Vk::CommandBufferAllocateInfo CommandBufferInfo{};
    CommandBufferInfo.commandPool = CommandPool;
    CommandBufferInfo.level = Vk::CommandBufferLevel::ePrimary;
    CommandBufferInfo.commandBufferCount = 1;

    Vk::Result Result{LogicalDevice.allocateCommandBuffers(&CommandBufferInfo, &CommandBuffer)};
    ASSERT(!!Result, "{}", Vk::to_string(Result));

    return CommandBuffer;
}

void Render::BeginRenderPass(uint32 VertexCount, Vk::RenderPass RenderPass, Vk::Pipeline Pipeline, Vk::Buffer Buffer, Vk::Extent2D ImageExtent, const TDynamicArray<Vk::Framebuffer>& FrameBuffers, const TDynamicArray<Vk::CommandBuffer>& CommandBuffers)
{
    const Vk::ClearValue ClearColor{std::array<float32, 4>{0.f, 0.f, 0.f, 1.f}};

    Vk::CommandBufferBeginInfo CommandBufferBeginInfo{};
    CommandBufferBeginInfo.flags = Vk::CommandBufferUsageFlagBits{};
    CommandBufferBeginInfo.pInheritanceInfo = nullptr;

    Vk::RenderPassBeginInfo RenderPassBeginInfo{};
    RenderPassBeginInfo.renderPass = RenderPass;
    RenderPassBeginInfo.renderArea.offset = Vk::Offset2D{0, 0};
    RenderPassBeginInfo.renderArea.extent = ImageExtent;
    RenderPassBeginInfo.clearValueCount = 1;
    RenderPassBeginInfo.pClearValues = &ClearColor;

    TStaticArray<uint64_t, 1> Offsets{0ULL};

    for(int64 Index{0}; Index < CommandBuffers.Num(); ++Index)
    {
        RenderPassBeginInfo.framebuffer = FrameBuffers[Index];

        Vk::Result Result{CommandBuffers[Index].begin(&CommandBufferBeginInfo)};
        ASSERT(!!Result, "{}", Vk::to_string(Result));

        CommandBuffers[Index].beginRenderPass(&RenderPassBeginInfo, Vk::SubpassContents::eInline);
        CommandBuffers[Index].bindPipeline(Vk::PipelineBindPoint::eGraphics, Pipeline);
        CommandBuffers[Index].bindVertexBuffers(0, 1, &Buffer, Offsets.Data());
        CommandBuffers[Index].draw(VertexCount, 1, 0, 0);
        CommandBuffers[Index].endRenderPass();
        CommandBuffers[Index].end();
    }

    LOG(LogVulkan, "began render pass");
}

template TDynamicArray<uint8> Render::ReadShaderFile(FString<SS0>);
template TDynamicArray<uint8> Render::ReadShaderFile(FString<SS28>);
template TDynamicArray<uint8> Render::ReadShaderFile(FString<SS124>);
template TDynamicArray<uint8> Render::ReadShaderFile(FString<SS252>);

template Vk::ShaderModule Render::CreateShaderModule(Vk::Device, FString<SS0>);
template Vk::ShaderModule Render::CreateShaderModule(Vk::Device, FString<SS28>);
template Vk::ShaderModule Render::CreateShaderModule(Vk::Device, FString<SS124>);
template Vk::ShaderModule Render::CreateShaderModule(Vk::Device, FString<SS252>);
