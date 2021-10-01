#include "VulkanRenderer.hpp"
#include "RenderWindow.hpp"
#include "Interface/Tick.hpp"
#include "CoreFiles/Array.hpp"
#include "CoreFiles/Log.hpp"
#include "RenderDebug.hpp"
#include "RenderInstance.hpp"

#include <chrono>
#include <fmt/os.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifndef VK_KHR_VALIDATION_LAYER_NAME
#define VK_KHR_VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"
#endif

Render::FDebugManager GDebugManager{};
Render::FInstanceManager GVulkanInstance{};
Render::FRenderWindow GRenderWindow{};

using namespace Render;

bool operator!(const Vk::Result VulkanResult)
{
    return VulkanResult != Vk::Result::eSuccess;
}

uint32 Render::Data::VulkanVersion{VK_MAKE_VERSION(1,2,172)};

uint32 Render::Data::VulkanAPIVersion{[]() -> uint32
{
    uint32 ApiVersion{0};
    Vk::Result Result{Vk::enumerateInstanceVersion(&ApiVersion)};

    ENSURE(!!Result, "could not enumerate instance version [{}]", Vk::to_string(Result));

    return ApiVersion;
}()};

TDynamicArray<const char8*> Render::Data::ValidationLayers{};
TDynamicArray<const char8*> Render::Data::ExtensionLayers{};
TDynamicArray<const char8*> Render::Data::DeviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

FRenderConfigInfo::FRenderConfigInfo(Vk::Extent2D ImageExtent, Vk::RenderPass InRenderPass)
    : RenderPass{InRenderPass}
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

void OShaderCodeReader::Run()
{
    using namespace Render;

    VertexFilePath = Data::ShaderPath + FileName + Data::VertShaderPostfix;
    FragmentationFilePath = Data::ShaderPath + FileName + Data::FragShaderPostfix;

    fmt::file VertexInputFile{VertexFilePath.Data(), fmt::file::RDONLY};
    fmt::file FragInputFile{FragmentationFilePath.Data(), fmt::file::RDONLY};

    VertexCode.ResizeTo(VertexInputFile.size() / sizeof(uint32));
    FragmentationCode.ResizeTo(FragInputFile.size() / sizeof(uint32));

    VertexInputFile.read(VertexCode.Data(), VertexInputFile.size());
    FragInputFile.read(FragmentationCode.Data(), FragInputFile.size());

    VertexInputFile.close();
    FragInputFile.close();
}

Vk::Queue Render::Device::GetDeviceQueueHandle(Vk::Device LogicalDevice, uint32 QueueIndex)
{
    return LogicalDevice.getQueue(QueueIndex, 0);
}

TDynamicArray<Vk::Image> Render::SwapChain::FindSwapChainImages(Vk::Device LogicalDevice, Vk::SwapchainKHR SwapChain)
{
    TDynamicArray<Vk::Image> FoundImages{};

    uint32 ImageCount{0};
    Vk::Result Result{LogicalDevice.getSwapchainImagesKHR(SwapChain, &ImageCount, nullptr)};
    ASSERT(!!Result, "failed to get swapchain images [{}]", Vk::to_string(Result));

    if(ImageCount > 0)
    {
        FoundImages.ResizeTo(ImageCount);

        Result = LogicalDevice.getSwapchainImagesKHR(SwapChain, &ImageCount, FoundImages.GetData());
        ASSERT(!!Result, "failed to get swapchain images [{}]", Vk::to_string(Result));
    }

    return FoundImages;
}

TDynamicArray<Vk::ImageView> Render::SwapChain::CreateImageViews(Vk::Device LogicalDevice, const TDynamicArray<Vk::Image>& SourceImages)
{
    TDynamicArray<Vk::ImageView> ImageViews{SourceImages.Num(), EChooseConstructor{}};

    Vk::ImageViewCreateInfo ImageViewCreateInfo{};
    ImageViewCreateInfo.viewType = Vk::ImageViewType::e2D;
    ImageViewCreateInfo.format = Data::SurfaceFormat.format;

    ImageViewCreateInfo.components.r = Vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.g = Vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.b = Vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.a = Vk::ComponentSwizzle::eIdentity;

    ImageViewCreateInfo.subresourceRange.aspectMask = Vk::ImageAspectFlagBits::eColor;
    ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    ImageViewCreateInfo.subresourceRange.levelCount = 1;
    ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    ImageViewCreateInfo.subresourceRange.layerCount = 1;

    for(int64 Index{0}; Index < SourceImages.Num(); ++Index)
    {
        ImageViewCreateInfo.image = SourceImages[Index];

        ImageViews.Append(LogicalDevice.createImageView(ImageViewCreateInfo));
    }

    LOG(LogVulkan, "created image views");

    return ImageViews;
}

void Render::SwapChain::DestroyImageViews(Vk::Device LogicalDevice, TDynamicArray<Vk::ImageView>& ImageViews)
{
    for(Vk::ImageView ImageView : ImageViews)
    {
        if(ImageView)
        {
            LogicalDevice.destroyImageView(ImageView);
        }
    }

    LOG(LogVulkan, "destroyed image views");
}

void Render::SwapChain::DestroyImages(Vk::Device LogicalDevice, TDynamicArray<Vk::Image>& Images)
{
    for(Vk::Image Image : Images)
    {
        if(Image)
        {
            LogicalDevice.destroyImage(Image);
        }
    }

    LOG(LogVulkan, "destroyed images");
}

Vk::Extent2D Render::SwapChain::ChooseImageExtent(const Vk::SurfaceCapabilitiesKHR SurfaceCapabilities)
{
    if(SurfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        return SurfaceCapabilities.currentExtent;
    }

    const uint32 ClampedWidth{Math::Clamp(static_cast<uint32>(Data::Window.GetPixelWidth()), SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width)};
    const uint32 ClampedHeight{Math::Clamp(static_cast<uint32>(Data::Window.GetPixelWidth()), SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height)};

    return Vk::Extent2D{ClampedWidth, ClampedHeight};
}

Vk::SurfaceFormatKHR Render::SwapChain::ChooseSurfaceFormat(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats)
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

Vk::PresentModeKHR Render::SwapChain::ChoosePresentationMode(const TDynamicArray<Vk::PresentModeKHR>& PresentModes)
{
    Vk::PresentModeKHR* FoundMode{nullptr};

    auto FindMode = [&PresentModes, &FoundMode](const Vk::PresentModeKHR ModeToFind) -> Vk::PresentModeKHR*
    {
        FoundMode = PresentModes.Find([ModeToFind](const Vk::PresentModeKHR Mode)
        {
            return Mode == ModeToFind;
        });

        return FoundMode;
    };

    if(!FindMode(Vk::PresentModeKHR::eMailbox))
    {
        if(!FindMode(Vk::PresentModeKHR::eImmediate))
        {
            FindMode(Vk::PresentModeKHR::eFifo);
        }
    }

    LOG(LogVulkan, "using presentation-mode: {}", Vk::to_string(*FoundMode));

    return *FoundMode;
}

Vk::SwapchainKHR Render::SwapChain::CreateSwapChain(Vk::Device LogicalDevice, Vk::SurfaceKHR Surface, Vk::SwapchainKHR OldSwapChain)
{
    Data::ImageExtent = ChooseImageExtent(Data::SwapChainSupportDetails.SurfaceCapabilities);
    Data::SurfaceFormat = ChooseSurfaceFormat(Data::SwapChainSupportDetails.SurfaceFormats);
    Data::PresentMode = ChoosePresentationMode(Data::SwapChainSupportDetails.PresentModes);

    uint32 MinImageCount{Data::SwapChainSupportDetails.SurfaceCapabilities.minImageCount + 1};

    if(Data::SwapChainSupportDetails.SurfaceCapabilities.maxImageCount != 0 && MinImageCount > Data::SwapChainSupportDetails.SurfaceCapabilities.maxImageCount)
    {
        MinImageCount = Data::SwapChainSupportDetails.SurfaceCapabilities.maxImageCount;
    }

    const TStaticArray<uint32, 2> QueueFamilyIndicesArray{Data::QueueFamilyIndices.Graphic, Data::QueueFamilyIndices.Presentation};

    const Vk::SwapchainCreateFlagsKHR SwapChainCreateFlags{};
    const Vk::ImageUsageFlags ImageUsageFlags{Vk::ImageUsageFlagBits::eColorAttachment};

    Vk::SwapchainCreateInfoKHR SwapChainCreateInfo{};
    SwapChainCreateInfo.flags = SwapChainCreateFlags;
    SwapChainCreateInfo.surface = Surface;
    SwapChainCreateInfo.minImageCount = MinImageCount;
    SwapChainCreateInfo.imageColorSpace = Data::SurfaceFormat.colorSpace;
    SwapChainCreateInfo.imageFormat = Data::SurfaceFormat.format;
    SwapChainCreateInfo.imageExtent = Data::ImageExtent;
    SwapChainCreateInfo.imageArrayLayers = 1;
    SwapChainCreateInfo.imageUsage = ImageUsageFlags;
    SwapChainCreateInfo.preTransform = Data::SwapChainSupportDetails.SurfaceCapabilities.currentTransform;
    SwapChainCreateInfo.compositeAlpha = Vk::CompositeAlphaFlagBitsKHR::eOpaque;
    SwapChainCreateInfo.presentMode = Data::PresentMode;
    SwapChainCreateInfo.clipped = true;
    SwapChainCreateInfo.oldSwapchain = OldSwapChain;

    if(Data::QueueFamilyIndices.Graphic == Data::QueueFamilyIndices.Presentation)
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


    Vk::SwapchainKHR NewSwapChain{LogicalDevice.createSwapchainKHR(SwapChainCreateInfo)};
    LOG(LogVulkan, "created swapchain {}", (void*)NewSwapChain);

    return NewSwapChain;
}

void Render::SwapChain::DestroySwapChain(Vk::SwapchainKHR SwapChain, Vk::Device LogicalDevice)
{
    LogicalDevice.destroySwapchainKHR(SwapChain);
    LOG(LogVulkan, "destroyed swapchain {}", (void*)SwapChain);
}

Vk::Pipeline Render::Pipeline::CreateGraphicsPipeline(Vk::Device LogicalDevice, FShaderModulePair ShaderModulePair, const FRenderConfigInfo& RenderConfigInfo)
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

    LOG(LogVulkan, "created pipeline-layout");

    Vk::GraphicsPipelineCreateInfo PipelineCreateInfo{};
    PipelineCreateInfo.stageCount = ShaderStages.Num();
    PipelineCreateInfo.pStages = ShaderStages.GetData();
    PipelineCreateInfo.pVertexInputState = &RenderConfigInfo.VertexInputInfo;
    PipelineCreateInfo.pInputAssemblyState = &RenderConfigInfo.AssemblyStateInfo;
    PipelineCreateInfo.pViewportState = &RenderConfigInfo.ViewportInfo;
    PipelineCreateInfo.pRasterizationState = &RenderConfigInfo.RasterizerInfo;
    PipelineCreateInfo.pMultisampleState = &RenderConfigInfo.MultisamplerInfo;
    PipelineCreateInfo.pDepthStencilState = &RenderConfigInfo.DepthStencilInfo;
    PipelineCreateInfo.pColorBlendState = &RenderConfigInfo.ColorBlendInfo;
    PipelineCreateInfo.pDynamicState = nullptr;//&RenderConfig.DynamicStateInfo;
    PipelineCreateInfo.renderPass = RenderConfigInfo.RenderPass;
    PipelineCreateInfo.layout = Handle::PipelineLayout;
    PipelineCreateInfo.subpass = 0; //index to subpass
    PipelineCreateInfo.basePipelineIndex = -1;
    PipelineCreateInfo.basePipelineHandle = NULL_HANDLE;

    Vk::Pipeline NewPipeline{NULL_HANDLE};

    const Vk::Result Result{LogicalDevice.createGraphicsPipelines(NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &NewPipeline)};
    ASSERT(!!Result, "could not create graphics pipelines", Vk::to_string(Result));

    LOG(LogVulkan, "created graphics pipelines");

    return NewPipeline;
}

FShaderModulePair Render::Pipeline::CreateShaderModulePair(Vk::Device LogicalDevice, OShaderCodeReader* ShaderCodeReader)
{
    FShaderModulePair Result{};

    ShaderCodeReader->WaitForCompletion();

    Vk::ShaderModuleCreateInfo ShaderModuleCreateInfo{};
    ShaderModuleCreateInfo.codeSize = ShaderCodeReader->VertexCode.Num() * sizeof(uint32);
    ShaderModuleCreateInfo.pCode = ShaderCodeReader->VertexCode.Data();

    Result.Vertex = LogicalDevice.createShaderModule(ShaderModuleCreateInfo);

    ShaderModuleCreateInfo.codeSize = ShaderCodeReader->FragmentationCode.Num() * sizeof(uint32);
    ShaderModuleCreateInfo.pCode = ShaderCodeReader->FragmentationCode.Data();

    Result.Fragmentation = LogicalDevice.createShaderModule(ShaderModuleCreateInfo);

    return Result;
}

Vk::PipelineLayout Render::Pipeline::CreatePipelineLayout(Vk::Device LogicalDevice, const FRenderConfigInfo& RenderConfigInfo)
{
    return LogicalDevice.createPipelineLayout(RenderConfigInfo.PipelineLayoutInfo);
}

void Render::Initialize()
{
    Thread::AsyncTask(&Data::ShaderCodeReader);

    GRenderWindow.CreateWindow(1000, 1000, "my amazing window", false);

    GVulkanInstance.PopulateExtensionLayers();
    GDebugManager.PopulateValidationLayers();

    GVulkanInstance.CreateInstance(GDebugManager.GetValidationLayers());
    GDebugManager.CreateMessenger(GVulkanInstance.GetVulkanInstanceHandle());



    Handle::Surface = SwapChain::CreateSurface(Handle::VulkanInstance, Data::Window.GetWindow());
    Handle::PhysicalDevice = Device::PickPhysicalDevice(Handle::VulkanInstance, Handle::Surface, Data::DeviceExtensions);
    Handle::LogicalDevice = Device::CreateLogicalDevice(Handle::PhysicalDevice, Data::QueueFamilyIndices);
    Handle::PresentationQueue = Device::GetDeviceQueueHandle(Handle::LogicalDevice, Data::QueueFamilyIndices.Presentation);
    Handle::GraphicsQueue = Device::GetDeviceQueueHandle(Handle::LogicalDevice, Data::QueueFamilyIndices.Graphic);
    Handle::ComputeQueue = Device::GetDeviceQueueHandle(Handle::LogicalDevice, Data::QueueFamilyIndices.Compute);
    Handle::SwapChain = SwapChain::CreateSwapChain(Handle::LogicalDevice, Handle::Surface);
    Handle::Images = SwapChain::FindSwapChainImages(Handle::LogicalDevice, Handle::SwapChain);
    Handle::ImageViews = SwapChain::CreateImageViews(Handle::LogicalDevice, Handle::Images);

    FRenderConfigInfo RenderConfigInfo{Data::Window.GetImageExtent(), Handle::RenderPass};

    Handle::PipelineLayout = Pipeline::CreatePipelineLayout(Handle::LogicalDevice,)
}

void Render::Loop()
{
    while(EXPECT(!Data::Window.ShouldClose(), true))
    {
        const auto StartTime{std::chrono::high_resolution_clock::now()};

        glfwPollEvents();

        const auto EndTime{std::chrono::high_resolution_clock::now()};
        std::chrono::duration<float64> DeltaTime{EndTime - StartTime};

        FTickManager::Instance().Tick(DeltaTime.count());
    }
}

void Render::ShutDown()
{
    Data::Window.CloseWindow();

    if(Handle::VulkanInstance)
    {
        if(Handle::DebugMessenger)
        {
            Debug::DestroyMessenger(Handle::DebugMessenger, Handle::VulkanInstance);
        }

        if(Handle::LogicalDevice)
        {
            SwapChain::DestroyImageViews(Handle::LogicalDevice, Handle::ImageViews);
            //SwapChain::DestroyImages(Handle::LogicalDevice, Data::Images);

            if(Handle::SwapChain)
            {
                SwapChain::DestroySwapChain(Handle::SwapChain, Handle::LogicalDevice);
            }

            Handle::LogicalDevice.destroy();
        }

        if(Handle::Surface)
        {
            SwapChain::DestroySurface(Handle::VulkanInstance, Handle::Surface);
        }

        Instance::DestroyInstance(Handle::VulkanInstance);
    }
}
