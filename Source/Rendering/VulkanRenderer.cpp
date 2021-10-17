#include "VulkanRenderer.hpp"
#include "RenderWindow.hpp"
#include "Interface/Tick.hpp"
#include "CoreFiles/Array.hpp"
#include "CoreFiles/Log.hpp"

#include <chrono>
#include <fmt/os.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifndef VK_KHR_VALIDATION_LAYER_NAME
#define VK_KHR_VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"
#endif

Render::OVulkanManager VulkanManager{};

inline bool operator!(const Vk::Result VulkanResult)
{
    return VulkanResult != Vk::Result::eSuccess;
}

OShaderCodeReader::OShaderCodeReader()
    : Allocation{nullptr}
    , VertexCode{nullptr}
    , VertexCodeSize{0}
    , FragCode{nullptr}
    , FragCodeSize{0}
{
}

OShaderCodeReader::OShaderCodeReader(const FString<SS124>& InFileName)
    : Allocation{nullptr}
    , VertexCode{nullptr}
    , VertexCodeSize{0}
    , FragCode{nullptr}
    , FragCodeSize{0}
    , FileName{Move(InFileName)}
{
}

OShaderCodeReader::~OShaderCodeReader()
{
    Memory::Free(Allocation);
}

void OShaderCodeReader::Run()
{
    if(Allocation != nullptr)
    {
        Memory::Free(Allocation);
    }

    FileName.PushBack_Assign(Render::ShaderPath);

    FString<SS124> VertexFilePath{FileName + Render::VertShaderPostfix};
    FString<SS124> FragFilePath{FileName + Render::FragShaderPostfix};

    fmt::file VertexInputFile{VertexFilePath.Data(), fmt::file::RDONLY};
    fmt::file FragInputFile{FragFilePath.Data(), fmt::file::RDONLY};

    VertexCodeSize = VertexInputFile.size();
    FragCodeSize =  FragInputFile.size();

    Allocation = Memory::Allocate<uint8>(VertexCodeSize + FragCodeSize);
    VertexCode = Allocation;
    FragCode = Allocation + VertexCodeSize;

    LOG(LogVulkan, "reading {}", VertexFilePath);
    VertexInputFile.read(VertexCode, VertexCodeSize);

    LOG(LogVulkan, "reading {}", FragFilePath);
    FragInputFile.read(FragCode, FragCodeSize);

    VertexInputFile.close();
    FragInputFile.close();
}

Render::FRenderConfigInfo::FRenderConfigInfo(Vk::Extent2D InImageExtent)
    : ImageExtent{InImageExtent}
    , DynamicStates{Vk::DynamicState::eViewport, Vk::DynamicState::eLineWidth}
{
    AssemblyStateInfo.topology = Vk::PrimitiveTopology::eTriangleList;
    AssemblyStateInfo.primitiveRestartEnable = false;

    VertexInputInfo.vertexBindingDescriptionCount = 0;
    VertexInputInfo.pVertexBindingDescriptions = nullptr;
    VertexInputInfo.vertexAttributeDescriptionCount = 0;
    VertexInputInfo.pVertexAttributeDescriptions = nullptr;

    Viewport.x = 0.f;
    Viewport.y = 0.f;
    Viewport.width = static_cast<float32>(ImageExtent.width);
    Viewport.height = static_cast<float32>(ImageExtent.height);
    Viewport.minDepth = 0.f;
    Viewport.maxDepth = 1.f;

    Scissor.offset = Vk::Offset2D{0, 0};
    Scissor.extent = ImageExtent;

    ViewportInfo.scissorCount = 1;
    ViewportInfo.pScissors = &Scissor;
    ViewportInfo.viewportCount = 1;
    ViewportInfo.pViewports = &Viewport;

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

    MultisamplerInfo.sampleShadingEnable = false;
    MultisamplerInfo.rasterizationSamples = Vk::SampleCountFlagBits::e1;
    MultisamplerInfo.minSampleShading = 1.0f;
    MultisamplerInfo.pSampleMask = nullptr;
    MultisamplerInfo.alphaToCoverageEnable = false;
    MultisamplerInfo.alphaToOneEnable = false;

    DepthStencilInfo.depthTestEnable = true;
    DepthStencilInfo.depthWriteEnable = true;
    DepthStencilInfo.depthCompareOp = Vk::CompareOp::eLess;
    DepthStencilInfo.depthBoundsTestEnable = false;
    DepthStencilInfo.minDepthBounds = 0.f;
    DepthStencilInfo.maxDepthBounds = 1.f;
    DepthStencilInfo.stencilTestEnable = false;
    DepthStencilInfo.front = Vk::StencilOp::eZero;
    DepthStencilInfo.back = Vk::StencilOp::eZero;

    ColorBlendAttachmentState.blendEnable = false;
    ColorBlendAttachmentState.colorWriteMask = Vk::ColorComponentFlagBits::eR | Vk::ColorComponentFlagBits::eG | Vk::ColorComponentFlagBits::eB | Vk::ColorComponentFlagBits::eA;
    ColorBlendAttachmentState.srcColorBlendFactor = Vk::BlendFactor::eSrcAlpha;
    ColorBlendAttachmentState.dstColorBlendFactor = Vk::BlendFactor::eOneMinusSrcAlpha;
    ColorBlendAttachmentState.colorBlendOp = Vk::BlendOp::eAdd;
    ColorBlendAttachmentState.srcAlphaBlendFactor = Vk::BlendFactor::eOne;
    ColorBlendAttachmentState.dstAlphaBlendFactor = Vk::BlendFactor::eZero;
    ColorBlendAttachmentState.alphaBlendOp = Vk::BlendOp::eAdd;

    ColorBlendInfo.attachmentCount = 1;
    ColorBlendInfo.pAttachments = &ColorBlendAttachmentState;
    ColorBlendInfo.logicOpEnable = false;
    ColorBlendInfo.logicOp = Vk::LogicOp::eCopy;
    ColorBlendInfo.blendConstants = std::array<float32, 4>{0.F, 0.F, 0.F, 0.F};

    DynamicStateInfo.dynamicStateCount = DynamicStates.Num();
    DynamicStateInfo.pDynamicStates = DynamicStates.GetData();

    PipelineLayoutInfo.setLayoutCount = 0;
    PipelineLayoutInfo.pSetLayouts = nullptr;
    PipelineLayoutInfo.pushConstantRangeCount = 0;
    PipelineLayoutInfo.pPushConstantRanges = nullptr;
}

Render::FQueueFamilyIndices::FQueueFamilyIndices()
    : Graphic{UINT32_MAX}
    , Compute{UINT32_MAX}
    , Protected{UINT32_MAX}
    , SparseBinding{UINT32_MAX}
    , Transfer{UINT32_MAX}
    , Presentation{UINT32_MAX}
{
}

Render::OVulkanManager::OVulkanManager()
    : InstanceHandle{NULL_HANDLE}
    , DebugMessengerHandle{NULL_HANDLE}
    , SurfaceHandle{NULL_HANDLE}
    , PhysicalDeviceHandle{NULL_HANDLE}
    , LogicalDeviceHandle{NULL_HANDLE}
    , SwapChainHandle{NULL_HANDLE}
    , GraphicsPipelineHandle{NULL_HANDLE}
    , RenderPassHandle{NULL_HANDLE}
    , CommandPool{NULL_HANDLE}
    , CurrentFrame{0}
    , VulkanVersion{VK_MAKE_VERSION(1,2,172)}
{
    Vk::Result Result{Vk::enumerateInstanceVersion(&VulkanAPIVersion)};
    ENSURE(!!Result, "could not enumerate instance version [{}]", Vk::to_string(Result));
}

void Render::Initialize()
{
    LOG(LogVulkan, "initializing vulkan");

    VulkanManager.ShaderCodeReader.FileName = "TriangleShader";
    Thread::AsyncTask(&VulkanManager.ShaderCodeReader);

    VulkanManager.CreateWindow();
    VulkanManager.CreateInstance();
    VulkanManager.CreateMessenger();
    VulkanManager.CreateSurface();
    VulkanManager.PickPhysicalDevice();
    VulkanManager.CreateLogicalDevice();
    VulkanManager.InitializeQueues();
    VulkanManager.CreateSwapChain();
    VulkanManager.PopulateImages();
    VulkanManager.CreateImageViews();
    VulkanManager.CreateRenderPass();

    FRenderConfigInfo RenderConfigInfo{VulkanManager.RenderWindow.GetImageExtent()};
    FShaderModulePair Shaders{VulkanManager.CreateShaderModulePair()};

    VulkanManager.CreateGraphicsPipeline(Shaders, RenderConfigInfo);
    VulkanManager.CreateFrameBuffers();
    VulkanManager.CreateDrawingCommandPool();
    VulkanManager.CreateCommandBuffers();
    VulkanManager.CreateFrameSyncData();
    VulkanManager.BeginRenderPass();
}

void Render::Loop()
{
    while(EXPECT(!VulkanManager.RenderWindow.ShouldClose(), true))
    {
        const auto StartTime{std::chrono::high_resolution_clock::now()};

        glfwPollEvents();

        VulkanManager.DrawFrame();

        const auto EndTime{std::chrono::high_resolution_clock::now()};
        std::chrono::duration<float64> DeltaTime{EndTime - StartTime};

        FTickManager::Instance().Tick(DeltaTime.count());
    }

    VulkanManager.LogicalDeviceHandle.waitIdle();
}

void Render::ShutDown()
{
    LOG(LogVulkan, "shutting down vulkan");

    VulkanManager.FreeCommandBuffers();
    VulkanManager.DestroyDrawingCommandPool();
    VulkanManager.DestroyFrameBuffers();
    VulkanManager.DestroyImageViews();
    VulkanManager.DestroySwapChain();
    VulkanManager.DestroySurface();
    VulkanManager.DestroyRenderPass();
    VulkanManager.DestroyPipelineLayout();
    VulkanManager.DestroyGraphicsPipeline();
    VulkanManager.DestroyFrameSyncData();
    VulkanManager.DestroyLogicalDevice();
    VulkanManager.DestroyPhysicalDevice();
    VulkanManager.DestroyMessenger();
    VulkanManager.DestroyInstance();
    VulkanManager.DestroyWindow();
}

uint32 Render::OVulkanManager::VulkanDebugCallback(Vk::DebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, Vk::DebugUtilsMessageTypeFlagsEXT MessageType, const Vk::DebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
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
            LOGW(LogVulkan, "{}", CallbackData->pMessage);
            break;
        }
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
        {
            throw FRuntimeError{CallbackData->pMessage};
        }
    }
    return false;
}

Vk::DebugUtilsMessengerCreateInfoEXT Render::OVulkanManager::MakeDebugUtilsMessengerCreateInfo()
{
    Vk::DebugUtilsMessengerCreateInfoEXT CreateInfo{};
    CreateInfo.messageSeverity = Vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    CreateInfo.messageType = Vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | Vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | Vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    CreateInfo.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(&VulkanDebugCallback);
    CreateInfo.pUserData = nullptr;
    return CreateInfo;
}

void Render::OVulkanManager::CreateMessenger()
{
#if DEBUG
    const Vk::DebugUtilsMessengerCreateInfoEXT CreateInfo{MakeDebugUtilsMessengerCreateInfo()};

    PFN_vkCreateDebugUtilsMessengerEXT Function{reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(InstanceHandle, "vkCreateDebugUtilsMessengerEXT"))};

    if(Function == nullptr)
    {
        throw FRuntimeError{"failed to create debug messenger [{}]", "vkGetInstanceProcAddr returned nullptr"};
    }

    Vk::Result Result{Function(InstanceHandle, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&CreateInfo), nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT_T**>(&DebugMessengerHandle))};

    if(!Result)
    {
        throw FRuntimeError{"failed to create debug messenger [{}]", Vk::to_string(Result)};
    }

    LOG(LogVulkan, "created debug messenger");
#endif
}

void Render::OVulkanManager::DestroyMessenger()
{
#if DEBUG
    if(!ENSURE(DebugMessengerHandle))
    {
        return;
    }

    PFN_vkDestroyDebugUtilsMessengerEXT Function{reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(InstanceHandle, "vkDestroyDebugUtilsMessengerEXT"))};

    if(!ENSURE(Function != nullptr))
    {
        return;
    }

    Function(InstanceHandle, DebugMessengerHandle.operator VkDebugUtilsMessengerEXT_T*(), nullptr);
    DebugMessengerHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed debug messenger");
#endif
}

void Render::OVulkanManager::CreateWindow()
{
    RenderWindow.CreateWindow(500, 500, "my amazing window", false);
}

void Render::OVulkanManager::DestroyWindow()
{
    RenderWindow.CloseWindow();
}

void Render::OVulkanManager::CreateInstance()
{
    PopulateValidationLayers();
    PopulateInstanceExtensionLayers();
    PopulateDeviceExtensionLayers();

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
    const Vk::DebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo{MakeDebugUtilsMessengerCreateInfo()};
    InstanceInfo.pNext = &DebugMessengerCreateInfo;
#endif

    InstanceHandle = Vk::createInstance(InstanceInfo);

    if(!InstanceHandle)
    {
        throw FRuntimeError{"failed to create vulkan instance"};
    }

    LOG(LogVulkan, "created vulkan instance");
}

void Render::OVulkanManager::DestroyInstance()
{
    if(!ENSURE(InstanceHandle))
    {
        return;
    }

    InstanceHandle.destroy();
    InstanceHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed vulkan instance");
}

void Render::OVulkanManager::PopulateValidationLayers()
{
#if DEBUG
    ValidationLayers.Append(VK_KHR_VALIDATION_LAYER_NAME);

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

    auto IsLayerAvailable = [&FoundValidationLayerProperties, this](const int64 Index) -> bool
    {
        for(const Vk::LayerProperties& LayerProperty : FoundValidationLayerProperties)
        {
            if(Memory::StringCompare(ValidationLayers[Index], LayerProperty.layerName))
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
            throw FRuntimeError{"validation layer {} was requested but is not available", ValidationLayers[Index]};
        }

        LOG(LogVulkan, "using validation-layer: \"{}\"", ValidationLayers[Index]);
    }
#endif
}

void Render::OVulkanManager::PopulateInstanceExtensionLayers()
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

void Render::OVulkanManager::PopulateDeviceExtensionLayers()
{
    DeviceExtensions.Append(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

TDynamicArray<Vk::ExtensionProperties> Render::OVulkanManager::FindPhysicalDeviceProperties()
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

TDynamicArray<Vk::QueueFamilyProperties> Render::OVulkanManager::FindQueueFamilyProperties()
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

void Render::OVulkanManager::PopulateQueueFamilyIndices()
{
    TDynamicArray<Vk::QueueFamilyProperties> QueueProperties{FindQueueFamilyProperties()};

    Vk::ArrayWrapper1D<char8, 256u> DeviceName{PhysicalDeviceHandle.getProperties().deviceName};

    LOG(LogVulkan, "searching queue indices for: {}", DeviceName);

    for(uint32 Index{0}; Index < QueueProperties.Num<uint32>(); ++Index)
    {
        auto WantsBreak = [this]() -> bool
        {
            const bool AreIndicesValid{Render::FQueueFamilyIndices::CheckValidity(QueueIndices.Presentation, QueueIndices.Graphic, QueueIndices.Compute)};
            const bool AreIndicesEqual{QueueIndices.Presentation == QueueIndices.Graphic}; //todo how many should be equal?

            return AreIndicesValid && AreIndicesEqual;
        };

        const Vk::Flags<Vk::QueueFlagBits> QueueFlags{QueueProperties[Index].queueFlags};

        if(QueueFlags & Vk::QueueFlagBits::eCompute)
        {
            QueueIndices.Compute = Index;
        }

        if(WantsBreak())
        {
            break;
        }

        if(PhysicalDeviceHandle.getSurfaceSupportKHR(Index, SurfaceHandle))
        {
            QueueIndices.Presentation = Index;
        }

        if(WantsBreak())
        {
            break;
        }

        if(QueueFlags & Vk::QueueFlagBits::eGraphics)
        {
            QueueIndices.Graphic = Index;
        }

        if(WantsBreak())
        {
            break;
        }
    }

    LOG(LogVulkan, "{} has graphic queue {}", DeviceName, QueueIndices.Graphic);
    LOG(LogVulkan, "{} has presentation queue {}", DeviceName, QueueIndices.Presentation);
    LOG(LogVulkan, "{} has compute queue {}", DeviceName, QueueIndices.Compute);
}

bool Render::OVulkanManager::DoesPhysicalDeviceSupportExtensions()
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

bool Render::OVulkanManager::DoesPhysicalDeviceSupportFeatures()
{
    const Vk::PhysicalDeviceFeatures& DeviceFeatures{PhysicalDeviceHandle.getFeatures()};
    return DeviceFeatures.shaderFloat64 && DeviceFeatures.geometryShader;
}

bool Render::OVulkanManager::DoesPhysicalDeviceSupportQueues()
{
    return Render::FQueueFamilyIndices::CheckValidity(QueueIndices.Graphic, QueueIndices.Presentation, QueueIndices.Compute);
}

bool Render::OVulkanManager::DoesPhysicalDeviceSupportSwapChain()
{
    return !SwapChainSupportDetails.SurfaceFormats.IsEmpty() && !SwapChainSupportDetails.PresentModes.IsEmpty();
}

Vk::SurfaceCapabilitiesKHR Render::OVulkanManager::FindSurfaceCapabilities()
{
    return PhysicalDeviceHandle.getSurfaceCapabilitiesKHR(SurfaceHandle);
}

TDynamicArray<Vk::SurfaceFormatKHR> Render::OVulkanManager::FindSurfaceFormats()
{
    TDynamicArray<Vk::SurfaceFormatKHR> FoundFormats{};

    uint32 SurfaceFormatCount{0};
    Vk::Result Result{PhysicalDeviceHandle.getSurfaceFormatsKHR(SurfaceHandle, &SurfaceFormatCount, nullptr)};
    ASSERT(!!Result, "failed to get surface formats: {}", Vk::to_string(Result));

    if(SurfaceFormatCount >= 0)
    {
        FoundFormats.ResizeTo(SurfaceFormatCount);

        Result = PhysicalDeviceHandle.getSurfaceFormatsKHR(SurfaceHandle, &SurfaceFormatCount, FoundFormats.GetData());
        ASSERT(!!Result, "failed to get surface formats: {}", Vk::to_string(Result));
    }

    return FoundFormats;
}

TDynamicArray<Vk::PresentModeKHR> Render::OVulkanManager::FindSurfacePresentModes()
{
    TDynamicArray<Vk::PresentModeKHR> FoundModes{};

    uint32 PresentModeCount{0};
    Vk::Result Result{PhysicalDeviceHandle.getSurfacePresentModesKHR(SurfaceHandle, &PresentModeCount, nullptr)};
    ASSERT(!!Result, "failed to get present modes: {}", Vk::to_string(Result));

    if(PresentModeCount >= 0)
    {
        FoundModes.ResizeTo(PresentModeCount);

        Result = PhysicalDeviceHandle.getSurfacePresentModesKHR(SurfaceHandle, &PresentModeCount, FoundModes.GetData());
        ASSERT(!!Result, "failed to get present modes: {}", Vk::to_string(Result));
    }

    return FoundModes;
}

TDynamicArray<Vk::PhysicalDevice> Render::OVulkanManager::FindAvailablePhysicalDevices()
{
    TDynamicArray<Vk::PhysicalDevice> PhysicalDeviceArray{};

    uint32 PhysicalDeviceCount{0};
    Vk::Result EnumerateResult{InstanceHandle.enumeratePhysicalDevices(&PhysicalDeviceCount, nullptr)};
    ASSERT(!!EnumerateResult, "failed to enumerate physical devices: {}", Vk::to_string(EnumerateResult));

    if(PhysicalDeviceCount > 0)
    {
        PhysicalDeviceArray.ResizeTo(PhysicalDeviceCount);

        EnumerateResult = InstanceHandle.enumeratePhysicalDevices(&PhysicalDeviceCount, PhysicalDeviceArray.GetData());
        ASSERT(!!EnumerateResult, "failed to enumerate physical devices: {}", Vk::to_string(EnumerateResult));
    }

    return PhysicalDeviceArray;
}

void Render::OVulkanManager::PopulateSwapChainSupportDetails()
{
    SwapChainSupportDetails.PresentModes = FindSurfacePresentModes();
    SwapChainSupportDetails.SurfaceCapabilities = FindSurfaceCapabilities();
    SwapChainSupportDetails.SurfaceFormats = FindSurfaceFormats();
}

void Render::OVulkanManager::CreateSurface()
{
    VkSurfaceKHR SurfaceHandleResult{NULL_HANDLE};

    Vk::Result Result{static_cast<Vk::Result>(glfwCreateWindowSurface(InstanceHandle, RenderWindow.GetWindowHandle(), nullptr, &SurfaceHandleResult))};

    if(!Result)
    {
        throw FRuntimeError{"could not create surface [{}]", Vk::to_string(Result)};
    }

    LOG(LogVulkan, "created surface");

    SurfaceHandle = SurfaceHandleResult;
}

void Render::OVulkanManager::DestroySurface()
{
    if(!ENSURE(InstanceHandle && SurfaceHandle))
    {
        return;
    }

    InstanceHandle.destroySurfaceKHR(SurfaceHandle);
    SurfaceHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed surface");
}

void Render::OVulkanManager::PickPhysicalDevice()
{
    TDynamicArray<Vk::PhysicalDevice> AvailablePhysicalDevices{FindAvailablePhysicalDevices()};

    auto FindPhysicalDevice = [this, &AvailablePhysicalDevices]<bool(*Check)(Vk::PhysicalDevice)>() -> bool
    {
        for(int64 Index{0}; Index < AvailablePhysicalDevices.Num(); ++Index)
        {
            PhysicalDeviceHandle = AvailablePhysicalDevices[Index];

            if((Check == nullptr) || (*Check)(PhysicalDeviceHandle))
            {
                if(DoesPhysicalDeviceSupportExtensions() && DoesPhysicalDeviceSupportFeatures())
                {
                    PopulateQueueFamilyIndices();

                    if(DoesPhysicalDeviceSupportQueues())
                    {
                        PopulateSwapChainSupportDetails();

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
        if(!FindPhysicalDevice.operator()<nullptr>())
        {
            throw FRuntimeError{"failed to find a physical GPU"};
        }
    }

    LOG(LogVulkan, "using physical-device: {}", PhysicalDeviceHandle.getProperties().deviceName);
}

void Render::OVulkanManager::DestroyPhysicalDevice()
{
    PhysicalDeviceHandle = NULL_HANDLE;
    LOG(LogVulkan, "destroyed physical device");
}

void Render::OVulkanManager::CreateLogicalDevice()
{
    float32 QueuePriority{1.f};
    TCountedArray<Vk::DeviceQueueCreateInfo, 3> DeviceQueueInfos
    {
        Vk::DeviceQueueCreateInfo{{}, QueueIndices.Graphic, 1u, &QueuePriority},
        Vk::DeviceQueueCreateInfo{{}, QueueIndices.Compute, 1u, &QueuePriority},
        Vk::DeviceQueueCreateInfo{{}, QueueIndices.Presentation, 1u, &QueuePriority}
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

    if(!LogicalDeviceHandle)
    {
        throw FRuntimeError{"failed to create logical device"};
    }

    LOG(LogVulkan, "created logical device");
}

void Render::OVulkanManager::DestroyLogicalDevice()
{
    if(!ENSURE(LogicalDeviceHandle))
    {
        return;
    }

    LogicalDeviceHandle.destroy();
    LogicalDeviceHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed logical device");
}

void Render::OVulkanManager::InitializeQueues()
{
    QueueHandles.Graphics = LogicalDeviceHandle.getQueue(QueueIndices.Graphic, 0);
    QueueHandles.Presentation = LogicalDeviceHandle.getQueue(QueueIndices.Presentation, 0);
    QueueHandles.Compute = LogicalDeviceHandle.getQueue(QueueIndices.Compute, 0);

    ASSERT(QueueHandles.Graphics && QueueHandles.Presentation && QueueHandles.Compute);
    LOG(LogVulkan, "created graphics, presentation and compute queues");
}

void Render::OVulkanManager::PopulateImages()
{
    uint32 ImageCount{0};
    Vk::Result Result{LogicalDeviceHandle.getSwapchainImagesKHR(SwapChainHandle, &ImageCount, nullptr)};
    ASSERT(!!Result, "failed to get swapchain images {}", Vk::to_string(Result));

    if(ImageCount > 0)
    {
        SwapChainImages.ResizeTo(ImageCount);

        Result = LogicalDeviceHandle.getSwapchainImagesKHR(SwapChainHandle, &ImageCount, SwapChainImages.Data());
        ASSERT(!!Result, "failed to get swapchain images {}", Vk::to_string(Result));
    }

    LOG(LogVulkan, "populated swapchain images");
}

void Render::OVulkanManager::DestroyImages()
{
    if(!ENSURE(LogicalDeviceHandle))
    {
        return;
    }

    for(Vk::Image& Image : SwapChainImages)
    {
        if(Image)
        {
            LogicalDeviceHandle.destroyImage(Image);
            Image = NULL_HANDLE;
        }
    }

    LOGW(LogVulkan, "destroyed images");
}

void Render::OVulkanManager::CreateImageViews()
{
    SwapChainImageViews.ResizeTo(SwapChainImages.Num());

    Vk::ImageViewCreateInfo ImageViewCreateInfo{};
    ImageViewCreateInfo.viewType = Vk::ImageViewType::e2D;
    ImageViewCreateInfo.format = SwapChainData.SurfaceFormat.format;

    ImageViewCreateInfo.components.r = Vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.g = Vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.b = Vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.a = Vk::ComponentSwizzle::eIdentity;

    ImageViewCreateInfo.subresourceRange.aspectMask = Vk::ImageAspectFlagBits::eColor;
    ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    ImageViewCreateInfo.subresourceRange.levelCount = 1;
    ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    ImageViewCreateInfo.subresourceRange.layerCount = 1;

    for(int64 Index{0}; Index < SwapChainImages.Num(); ++Index)
    {
        ImageViewCreateInfo.image = SwapChainImages[Index];
        SwapChainImageViews[Index] = LogicalDeviceHandle.createImageView(ImageViewCreateInfo);

        ASSERT(SwapChainImageViews[Index]);
    }

    LOG(LogVulkan, "created image views");
}

void Render::OVulkanManager::DestroyImageViews()
{
    if(!ENSURE(LogicalDeviceHandle))
    {
        return;
    }

    for(Vk::ImageView& ImageView : SwapChainImageViews)
    {
        if(ImageView)
        {
            LogicalDeviceHandle.destroyImageView(ImageView);
            ImageView = NULL_HANDLE;
        }
    }

    LOG(LogVulkan, "destroyed image-views");
}

Vk::Extent2D Render::OVulkanManager::ChooseImageExtent()
{
    Vk::SurfaceCapabilitiesKHR& SurfaceCapabilities{SwapChainSupportDetails.SurfaceCapabilities};

    FWindowDimensions Dimensions{RenderWindow.GetWindowDimensions()};

    const uint32 ClampedWidth{Math::Clamp(static_cast<uint32>(Dimensions.PixelWidth), SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width)};
    const uint32 ClampedHeight{Math::Clamp(static_cast<uint32>(Dimensions.PixelHeight), SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height)};

    LOG(LogVulkan, "using image width: {} height: {}", ClampedWidth, ClampedHeight);

    return Vk::Extent2D{ClampedWidth, ClampedHeight};
}

Vk::SurfaceFormatKHR Render::OVulkanManager::ChooseSurfaceFormat()
{
    Vk::SurfaceFormatKHR FoundFormat{SwapChainSupportDetails.SurfaceFormats[0]};

    for(int32 Index{1}; Index < SwapChainSupportDetails.SurfaceFormats.Num(); ++Index)
    {
        if(FoundFormat.format == Vk::Format::eR8G8B8A8Srgb && FoundFormat.colorSpace == Vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            break;
        }

        FoundFormat = SwapChainSupportDetails.SurfaceFormats[Index];
    }

    LOG(LogVulkan, "using color-format: {}", Vk::to_string(FoundFormat.format));
    LOG(LogVulkan, "using color-space: {}", Vk::to_string(FoundFormat.colorSpace));

    return FoundFormat;
}

Vk::PresentModeKHR Render::OVulkanManager::ChoosePresentationMode()
{
    uint32 MailboxFound{0};
    uint32 ImmediateFound{0};

    for(const Vk::PresentModeKHR Mode : SwapChainSupportDetails.PresentModes)
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

void Render::OVulkanManager::PopulateSwapChainData()
{
    SwapChainData.ImageExtent = ChooseImageExtent();
    SwapChainData.SurfaceFormat = ChooseSurfaceFormat();
    SwapChainData.PresentMode = ChoosePresentationMode();
}

void Render::OVulkanManager::CreateSwapChain(Vk::SwapchainKHR OldSwapChain)
{
    PopulateSwapChainData();

    uint32 MinImageCount{SwapChainSupportDetails.SurfaceCapabilities.minImageCount + 1};

    if(SwapChainSupportDetails.SurfaceCapabilities.maxImageCount != 0 && MinImageCount > SwapChainSupportDetails.SurfaceCapabilities.maxImageCount)
    {
        MinImageCount = SwapChainSupportDetails.SurfaceCapabilities.maxImageCount;
    }

    const TStaticArray<uint32, 2> QueueFamilyIndicesArray{QueueIndices.Graphic, QueueIndices.Presentation};

    const Vk::SwapchainCreateFlagsKHR SwapChainCreateFlags{};
    const Vk::ImageUsageFlags ImageUsageFlags{Vk::ImageUsageFlagBits::eColorAttachment};

    Vk::SwapchainCreateInfoKHR SwapChainCreateInfo{};
    SwapChainCreateInfo.flags = SwapChainCreateFlags;
    SwapChainCreateInfo.surface = SurfaceHandle;
    SwapChainCreateInfo.minImageCount = MinImageCount;
    SwapChainCreateInfo.imageColorSpace = SwapChainData.SurfaceFormat.colorSpace;
    SwapChainCreateInfo.imageFormat = SwapChainData.SurfaceFormat.format;
    SwapChainCreateInfo.imageExtent = SwapChainData.ImageExtent;
    SwapChainCreateInfo.imageArrayLayers = 1;
    SwapChainCreateInfo.imageUsage = ImageUsageFlags;
    SwapChainCreateInfo.preTransform = SwapChainSupportDetails.SurfaceCapabilities.currentTransform;
    SwapChainCreateInfo.compositeAlpha = Vk::CompositeAlphaFlagBitsKHR::eOpaque;
    SwapChainCreateInfo.presentMode = SwapChainData.PresentMode;
    SwapChainCreateInfo.clipped = true;
    SwapChainCreateInfo.oldSwapchain = OldSwapChain;

    if(QueueIndices.Graphic == QueueIndices.Presentation)
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


    SwapChainHandle = LogicalDeviceHandle.createSwapchainKHR(SwapChainCreateInfo);

    if(!SwapChainHandle)
    {
        throw FRuntimeError{"failed to create swapchain"};
    }

    LOG(LogVulkan, "created swapchain");
}

void Render::OVulkanManager::RecreateSwapChain()
{
    while(RenderWindow.IsMinimized())
    {
        RenderWindow.UpdateWindowDimensions();
        glfwWaitEvents();
    }

    LogicalDeviceHandle.waitIdle();

    Vk::SwapchainKHR OldSwapChain = SwapChainHandle;

    FreeCommandBuffers();
    DestroyDrawingCommandPool();
    DestroyFrameBuffers();
    DestroyGraphicsPipeline();
    DestroyPipelineLayout();
    DestroyRenderPass();
    DestroyImageViews();
    DestroyFrameSyncData();
    DestroySwapChain();

    PopulateSwapChainSupportDetails();
    CreateSwapChain();
    PopulateImages();
    CreateImageViews();
    CreateRenderPass();

    Render::FRenderConfigInfo RenderConfigInfo{RenderWindow.GetImageExtent()};
    Render::FShaderModulePair Shaders{CreateShaderModulePair()};

    CreateGraphicsPipeline(Shaders, RenderConfigInfo);
    CreateFrameBuffers();
    CreateDrawingCommandPool();
    CreateCommandBuffers();
    CreateFrameSyncData();
    BeginRenderPass();
}

void Render::OVulkanManager::DestroySwapChain()
{
    if(!ENSURE(LogicalDeviceHandle && SwapChainHandle))
    {
        return;
    }

    LogicalDeviceHandle.destroySwapchainKHR(SwapChainHandle);
    SwapChainHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed swapchain");
}

void Render::OVulkanManager::CreateRenderPass()
{
    Vk::AttachmentDescription ColorAttachment{};
    ColorAttachment.format = SwapChainData.SurfaceFormat.format;
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

    RenderPassHandle = LogicalDeviceHandle.createRenderPass(RenderPassInfo);

    if(!RenderPassHandle)
    {
        throw FRuntimeError{"failed to create render pass"};
    }

    LOG(LogVulkan, "created render pass");
}

void Render::OVulkanManager::DestroyRenderPass()
{
    if(!ENSURE(LogicalDeviceHandle && RenderPassHandle))
    {
        return;
    }

    LogicalDeviceHandle.destroyRenderPass(RenderPassHandle);
    RenderPassHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed renderpass");
}

void Render::OVulkanManager::CreateGraphicsPipeline(Render::FShaderModulePair ShaderModulePair, const Render::FRenderConfigInfo& RenderConfigInfo)
{
    Vk::PipelineShaderStageCreateInfo VertexShaderStageInfo{};
    VertexShaderStageInfo.stage = Vk::ShaderStageFlagBits::eVertex;
    VertexShaderStageInfo.module = ShaderModulePair.Vertex;
    VertexShaderStageInfo.pName = "main";
    VertexShaderStageInfo.pNext = nullptr;
    VertexShaderStageInfo.pSpecializationInfo = nullptr;

    Vk::PipelineShaderStageCreateInfo FragmentationShaderStageInfo{};
    FragmentationShaderStageInfo.stage = Vk::ShaderStageFlagBits::eFragment;
    FragmentationShaderStageInfo.module = ShaderModulePair.Fragmentation;
    FragmentationShaderStageInfo.pName = "main";
    FragmentationShaderStageInfo.pNext = nullptr;
    FragmentationShaderStageInfo.pSpecializationInfo = nullptr;

    const TStaticArray<Vk::PipelineShaderStageCreateInfo, 2> ShaderStages{VertexShaderStageInfo, FragmentationShaderStageInfo};

    GraphicsPipelineLayout = LogicalDeviceHandle.createPipelineLayout(RenderConfigInfo.PipelineLayoutInfo);

    if(!GraphicsPipelineLayout)
    {
        throw FRuntimeError{"failed to create graphics pipeline layout"};
    }

    LOG(LogVulkan, "created graphics pipeline layout");

    Vk::GraphicsPipelineCreateInfo PipelineCreateInfo{};
    PipelineCreateInfo.stageCount = ShaderStages.Num();
    PipelineCreateInfo.pStages = ShaderStages.Data();
    PipelineCreateInfo.pVertexInputState = &RenderConfigInfo.VertexInputInfo;
    PipelineCreateInfo.pInputAssemblyState = &RenderConfigInfo.AssemblyStateInfo;
    PipelineCreateInfo.pViewportState = &RenderConfigInfo.ViewportInfo;
    PipelineCreateInfo.pRasterizationState = &RenderConfigInfo.RasterizerInfo;
    PipelineCreateInfo.pMultisampleState = &RenderConfigInfo.MultisamplerInfo;
    PipelineCreateInfo.pDepthStencilState = &RenderConfigInfo.DepthStencilInfo;
    PipelineCreateInfo.pColorBlendState = &RenderConfigInfo.ColorBlendInfo;
    PipelineCreateInfo.pDynamicState = nullptr;//&RenderConfig.DynamicStateInfo;
    PipelineCreateInfo.renderPass = RenderPassHandle;
    PipelineCreateInfo.layout = GraphicsPipelineLayout;
    PipelineCreateInfo.subpass = 0; //index to subpass
    PipelineCreateInfo.basePipelineIndex = -1;
    PipelineCreateInfo.basePipelineHandle = NULL_HANDLE;

   Vk::Result Result{LogicalDeviceHandle.createGraphicsPipelines(NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &GraphicsPipelineHandle)};

    if(!Result)
    {
        throw FRuntimeError{"could not create graphics pipeline [{}]", Vk::to_string(Result)};
    }

    LOG(LogVulkan, "created graphics pipeline");

    LogicalDeviceHandle.destroyShaderModule(ShaderModulePair.Vertex);
    LogicalDeviceHandle.destroyShaderModule(ShaderModulePair.Fragmentation);
}

void Render::OVulkanManager::DestroyGraphicsPipeline()
{
    if(!ENSURE(LogicalDeviceHandle && GraphicsPipelineHandle))
    {
        return;
    }

    LogicalDeviceHandle.destroyPipeline(GraphicsPipelineHandle);
    GraphicsPipelineHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed graphics pipeline");
}

void Render::OVulkanManager::DestroyPipelineLayout()
{
    if(!ENSURE(LogicalDeviceHandle && GraphicsPipelineLayout))
    {
        return;
    }

    LogicalDeviceHandle.destroyPipelineLayout(GraphicsPipelineLayout);
    GraphicsPipelineLayout = NULL_HANDLE;

    LOG(LogVulkan, "destroyed graphics pipeline layout");
}

Render::FShaderModulePair Render::OVulkanManager::CreateShaderModulePair()
{
    Render::FShaderModulePair Result{};

    ShaderCodeReader.WaitForCompletion();

    Vk::ShaderModuleCreateInfo ShaderModuleCreateInfo{};
    ShaderModuleCreateInfo.codeSize = ShaderCodeReader.VertexCodeSize;
    ShaderModuleCreateInfo.pCode = reinterpret_cast<uint32*>(ShaderCodeReader.VertexCode);

    Result.Vertex = LogicalDeviceHandle.createShaderModule(ShaderModuleCreateInfo);

    ShaderModuleCreateInfo.codeSize = ShaderCodeReader.FragCodeSize;
    ShaderModuleCreateInfo.pCode = reinterpret_cast<uint32*>(ShaderCodeReader.FragCode);

    Result.Fragmentation = LogicalDeviceHandle.createShaderModule(ShaderModuleCreateInfo);

    return Result;
}

void Render::OVulkanManager::CreateFrameBuffers()
{
    SwapChainFrameBuffers.ResizeTo(SwapChainImageViews.Num());

    for(int64 Index{0}; Index < SwapChainFrameBuffers.Num(); ++Index)
    {
        Vk::FramebufferCreateInfo FrameBufferInfo{};
        FrameBufferInfo.renderPass = RenderPassHandle;
        FrameBufferInfo.attachmentCount = 1;
        FrameBufferInfo.pAttachments = &SwapChainImageViews[Index];
        FrameBufferInfo.width = SwapChainData.ImageExtent.width;
        FrameBufferInfo.height = SwapChainData.ImageExtent.height;
        FrameBufferInfo.layers = 1;

        Vk::Result Result{LogicalDeviceHandle.createFramebuffer(&FrameBufferInfo, nullptr, &SwapChainFrameBuffers[Index])};

        if(!Result)
        {
            throw FRuntimeError{"could not create framebuffer [{}]", Vk::to_string(Result)};
        }
    }
}

void Render::OVulkanManager::DestroyFrameBuffers()
{
    if(!ENSURE(LogicalDeviceHandle))
    {
        return;
    }

    for(Vk::Framebuffer& FrameBuffer : SwapChainFrameBuffers)
    {
        LogicalDeviceHandle.destroyFramebuffer(FrameBuffer);
        FrameBuffer = NULL_HANDLE;
    }

    SwapChainFrameBuffers.Empty<false>();

    LOG(LogVulkan, "destroyed framebuffers");
}

void Render::OVulkanManager::CreateDrawingCommandPool()
{
    Vk::CommandPoolCreateInfo CommandPoolInfo{};
    CommandPoolInfo.queueFamilyIndex = QueueIndices.Graphic;
    CommandPoolInfo.flags = Vk::CommandPoolCreateFlagBits{};

    Vk::Result Result{LogicalDeviceHandle.createCommandPool(&CommandPoolInfo, nullptr, &CommandPool)};

    if(!Result)
    {
        throw FRuntimeError{"could not create command pool [{}]", Vk::to_string(Result)};
    }

    LOG(LogVulkan, "created drawing command pool");
}

void Render::OVulkanManager::DestroyDrawingCommandPool()
{
    if(!ENSURE(LogicalDeviceHandle && CommandPool))
    {
        return;
    }

    LogicalDeviceHandle.destroyCommandPool(CommandPool);
    CommandPool = NULL_HANDLE;

    LOG(LogVulkan, "destroyed drawing command pool");
}

void Render::OVulkanManager::CreateCommandBuffers()
{
    CommandBuffers.ResizeTo(SwapChainFrameBuffers.Num());

    Vk::CommandBufferAllocateInfo CommandBufferInfo{};
    CommandBufferInfo.commandPool = CommandPool;
    CommandBufferInfo.level = Vk::CommandBufferLevel::ePrimary;
    CommandBufferInfo.commandBufferCount = CommandBuffers.Num();

    Vk::Result Result{LogicalDeviceHandle.allocateCommandBuffers(&CommandBufferInfo, CommandBuffers.Data())};

    if(!Result)
    {
        throw FRuntimeError{"could not allocate command buffers [{}]", Vk::to_string(Result)};
    }

    LOG(LogVulkan, "allocated command buffers");
}

void Render::OVulkanManager::FreeCommandBuffers()
{
    if(!ENSURE(LogicalDeviceHandle && CommandPool))
    {
        return;
    }

    LogicalDeviceHandle.freeCommandBuffers(CommandPool, CommandBuffers.Num(), CommandBuffers.Data());

    LOG(LogVulkan, "freed command buffers");
}

void Render::OVulkanManager::BeginRenderPass()
{
    const Vk::ClearValue ClearColor{std::array<float32, 4>{0.f, 0.f, 0.f, 1.f}};

    Vk::CommandBufferBeginInfo CommandBufferBeginInfo{};
    CommandBufferBeginInfo.flags = Vk::CommandBufferUsageFlagBits{};
    CommandBufferBeginInfo.pInheritanceInfo = nullptr;

    Vk::RenderPassBeginInfo RenderPassBeginInfo{};
    RenderPassBeginInfo.renderPass = RenderPassHandle;
    RenderPassBeginInfo.renderArea.offset = Vk::Offset2D{0, 0};
    RenderPassBeginInfo.renderArea.extent = SwapChainData.ImageExtent;
    RenderPassBeginInfo.clearValueCount = 1;
    RenderPassBeginInfo.pClearValues = &ClearColor;

    for(int64 Index{0}; Index < CommandBuffers.Num(); ++Index)
    {
        RenderPassBeginInfo.framebuffer = SwapChainFrameBuffers[Index];

        if(Vk::Result Result{CommandBuffers[Index].begin(&CommandBufferBeginInfo)}; !Result)
        {
            throw FRuntimeError{"failed to begin command buffer {} [{}]", Index, Vk::to_string(Result)};
        }

        CommandBuffers[Index].beginRenderPass(&RenderPassBeginInfo, Vk::SubpassContents::eInline);
        CommandBuffers[Index].bindPipeline(Vk::PipelineBindPoint::eGraphics, GraphicsPipelineHandle);
        CommandBuffers[Index].draw(6, 1, 0, 0);
        CommandBuffers[Index].endRenderPass();
        CommandBuffers[Index].end();
    }

    LOG(LogVulkan, "began render pass");
}

void Render::OVulkanManager::CreateFrameSyncData()
{
    Vk::SemaphoreCreateInfo SemaphoreCreateInfo{};

    Vk::FenceCreateInfo FenceCreateInfo{};
    FenceCreateInfo.flags = Vk::FenceCreateFlagBits::eSignaled;

    ImagesInFlightFence.ResizeTo(SwapChainImageViews.Num());

    for(Vk::Fence& Fence : ImagesInFlightFence)
    {
        Fence = NULL_HANDLE;
    }

    for(Render::FFrameData& Data : FrameData)
    {
        if(!LogicalDeviceHandle.createSemaphore(&SemaphoreCreateInfo, nullptr, &Data.ImageAvailableSemaphore)
        || !LogicalDeviceHandle.createSemaphore(&SemaphoreCreateInfo, nullptr, &Data.RenderFinishedSemaphore)
        || !LogicalDeviceHandle.createFence(&FenceCreateInfo, nullptr, &Data.FlightFence))
        {
            throw FRuntimeError{"failed to create frame sync data"};
        }
    }

    LOG(LogVulkan, "created frame sync data");
}

void Render::OVulkanManager::DestroyFrameSyncData()
{
    if(!ENSURE(LogicalDeviceHandle))
    {
        return;
    }

    for(Render::FFrameData& Data : FrameData)
    {
        LogicalDeviceHandle.destroySemaphore(Data.ImageAvailableSemaphore);
        LogicalDeviceHandle.destroySemaphore(Data.RenderFinishedSemaphore);
        LogicalDeviceHandle.destroyFence(Data.FlightFence);
    }

    LOG(LogVulkan, "destroyed frame sync data");
}

void Render::OVulkanManager::DrawFrame()
{
    CHECK(!!LogicalDeviceHandle.waitForFences(1, &FrameData[CurrentFrame].FlightFence, true, UINT64_MAX));

    uint32 ImageIndex;
    Vk::Result Result{LogicalDeviceHandle.acquireNextImageKHR(SwapChainHandle, UINT64_MAX, FrameData[CurrentFrame].ImageAvailableSemaphore, NULL_HANDLE, &ImageIndex)};

    if EXPECT(Result == Vk::Result::eErrorOutOfDateKHR || Result == Vk::Result::eSuboptimalKHR || RenderWindow.HasBeenResized, false)
    {
        RenderWindow.HasBeenResized = false;
        RecreateSwapChain();
        return;
    }
    else if EXPECT(Result != Vk::Result::eSuccess, false)
    {
        throw FRuntimeError{"failed to acquire next image [{}]", Vk::to_string(Result)};
    }

    if(ImagesInFlightFence[ImageIndex] != Vk::Fence{NULL_HANDLE})
    {
        CHECK(!!LogicalDeviceHandle.waitForFences(1, &ImagesInFlightFence[ImageIndex], true, UINT64_MAX));
    }

    ImagesInFlightFence[ImageIndex] = FrameData[CurrentFrame].FlightFence;

    TStaticArray<Vk::Semaphore*, 1> WaitSemaphores{&FrameData[CurrentFrame].ImageAvailableSemaphore};
    TStaticArray<Vk::Semaphore*, 1> SignalSemaphores{&FrameData[CurrentFrame].RenderFinishedSemaphore};
    TStaticArray<Vk::PipelineStageFlags, 1> WaitStages{Vk::PipelineStageFlagBits::eColorAttachmentOutput};

    Vk::SubmitInfo SubmitInfo{};
    SubmitInfo.waitSemaphoreCount = WaitSemaphores.Num();
    SubmitInfo.pWaitSemaphores = *WaitSemaphores.Data();
    SubmitInfo.pWaitDstStageMask = WaitStages.Data();
    SubmitInfo.signalSemaphoreCount = SignalSemaphores.Num();
    SubmitInfo.pSignalSemaphores = *SignalSemaphores.Data();
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffers[ImageIndex];

    CHECK(!!LogicalDeviceHandle.resetFences(1, &FrameData[CurrentFrame].FlightFence));

    CHECK(!!QueueHandles.Graphics.submit(1, &SubmitInfo, FrameData[CurrentFrame].FlightFence));

    TStaticArray<Vk::SwapchainKHR*, 1> SwapChains{&SwapChainHandle};

    Vk::PresentInfoKHR PresentInfo{};
    PresentInfo.waitSemaphoreCount = SignalSemaphores.Num();
    PresentInfo.pWaitSemaphores = *SignalSemaphores.Data();
    PresentInfo.swapchainCount = SwapChains.Num();
    PresentInfo.pSwapchains = *SwapChains.Data();
    PresentInfo.pImageIndices = &ImageIndex;
    PresentInfo.pResults = nullptr;

    Result = QueueHandles.Presentation.presentKHR(&PresentInfo);

    if EXPECT(Result == Vk::Result::eErrorOutOfDateKHR || Result == Vk::Result::eSuboptimalKHR || RenderWindow.HasBeenResized, false)
    {
        RenderWindow.HasBeenResized = false;
        RecreateSwapChain();
        return;
    }
    else if EXPECT(Result != Vk::Result::eSuccess, false)
    {
        throw FRuntimeError{"failed to present [{}]", Vk::to_string(Result)};
    }

    ++CurrentFrame;
    CurrentFrame %= Render::MaxFramesInFlight;
}
