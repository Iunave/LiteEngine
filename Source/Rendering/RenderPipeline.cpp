#include "RenderPipeline.hpp"
#include "RenderDevice.hpp"
#include "Thread/ThreadPool.hpp"
#include "CoreFiles/Log.hpp"
#include <fmt/os.h>

inline constexpr FString<SS124> ShaderPath{"../Shaders/Compiled/"};
inline constexpr FString<SS124> FragShaderPostfix{".frag.spv"};
inline constexpr FString<SS124> VertShaderPostfix{".vert.spv"};
/*
inline bool operator!(const Vk::Result VulkanResult)
{
    return VulkanResult != Vk::Result::eSuccess;
}

FRenderConfigInfo::FRenderConfigInfo(Vk::Extent2D InImageExtent)
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

    FString<SS124> VertexFilePath{ShaderPath + FileName + VertShaderPostfix};
    FString<SS124> FragFilePath{ShaderPath + FileName + FragShaderPostfix};

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

FRenderPipelineManager::FRenderPipelineManager()
    : GraphicsPipelineHandle{NULL_HANDLE}
    , RenderPass{NULL_HANDLE}
{

}

FRenderPipelineManager::~FRenderPipelineManager()
{

}

void FRenderPipelineManager::CreateRenderPass(Vk::Device LogicalDevice, Vk::SurfaceFormatKHR SurfaceFormat)
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

    Vk::RenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.attachmentCount = 1;
    RenderPassInfo.pAttachments = &ColorAttachment;
    RenderPassInfo.subpassCount = 1;
    RenderPassInfo.pSubpasses = &SubpassDescription;

    RenderPass = LogicalDevice.createRenderPass(RenderPassInfo);

    ASSERT(RenderPass);
    LOG(LogVulkan, "created render pass");
}

void FRenderPipelineManager::CreateGraphicsPipeline(Vk::Device LogicalDevice, FShaderModulePair ShaderModulePair, const FRenderConfigInfo& RenderConfigInfo)
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

    GraphicsPipelineLayout = LogicalDevice.createPipelineLayout(RenderConfigInfo.PipelineLayoutInfo);

    ASSERT(GraphicsPipelineLayout);
    LOG(LogVulkan, "created pipeline layout");

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
    PipelineCreateInfo.renderPass = RenderPass;
    PipelineCreateInfo.layout = GraphicsPipelineLayout;
    PipelineCreateInfo.subpass = 0; //index to subpass
    PipelineCreateInfo.basePipelineIndex = -1;
    PipelineCreateInfo.basePipelineHandle = NULL_HANDLE;

    const Vk::Result Result{LogicalDevice.createGraphicsPipelines(NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &GraphicsPipelineHandle)};

    ASSERT(!!Result, "could not create graphics pipeline", Vk::to_string(Result));
    LOG(LogVulkan, "created graphics pipeline");

    LogicalDevice.destroyShaderModule(ShaderModulePair.Vertex);
    LogicalDevice.destroyShaderModule(ShaderModulePair.Fragmentation);
}

void FRenderPipelineManager::DestroyGraphicsPipeline(Vk::Device LogicalDevice)
{
    if(!LogicalDevice)
    {
        LOGW(LogVulkan, "failed to destroy graphics pipeline [logical device is invalid]");
        return;
    }

    if(!GraphicsPipelineHandle)
    {
        LOGW(LogVulkan, "failed to destroy graphics pipeline [its already invalid]");
        return;
    }

    LogicalDevice.destroyPipeline(GraphicsPipelineHandle);
    GraphicsPipelineHandle = NULL_HANDLE;

    LOG(LogVulkan, "destroyed graphics pipeline");
}

FShaderModulePair FRenderPipelineManager::CreateShaderModulePair(Vk::Device LogicalDevice, OShaderCodeReader* ShaderCodeReader)
{
    FShaderModulePair Result{};

    ShaderCodeReader->WaitForCompletion();

    Vk::ShaderModuleCreateInfo ShaderModuleCreateInfo{};
    ShaderModuleCreateInfo.codeSize = ShaderCodeReader->VertexCodeSize;
    ShaderModuleCreateInfo.pCode = reinterpret_cast<uint32*>(ShaderCodeReader->VertexCode);

    Result.Vertex = LogicalDevice.createShaderModule(ShaderModuleCreateInfo);

    ShaderModuleCreateInfo.codeSize = ShaderCodeReader->FragCodeSize;
    ShaderModuleCreateInfo.pCode = reinterpret_cast<uint32*>(ShaderCodeReader->FragCode);

    Result.Fragmentation = LogicalDevice.createShaderModule(ShaderModuleCreateInfo);

    return Result;
}

void FRenderPipelineManager::DestroyRenderPass(Vk::Device LogicalDevice)
{
    if(!LogicalDevice)
    {
        LOGW(LogVulkan, "failed to destroy renderpass [logical device is invalid]");
        return;
    }

    if(!GraphicsPipelineHandle)
    {
        LOGW(LogVulkan, "failed to destroy renderpass [its already invalid]");
        return;
    }

    LogicalDevice.destroyRenderPass(RenderPass);
    RenderPass = NULL_HANDLE;

    LOG(LogVulkan, "destroyed renderpass");
}

void FRenderPipelineManager::DestroyPipelineLayout(Vk::Device LogicalDevice)
{
    if(!LogicalDevice)
    {
        LOGW(LogVulkan, "failed to destroy graphics pipeline layout [logical device is invalid]");
        return;
    }

    if(!GraphicsPipelineLayout)
    {
        LOGW(LogVulkan, "failed to destroy graphics pipeline layout [its already invalid]");
        return;
    }

    LogicalDevice.destroyPipelineLayout(GraphicsPipelineLayout);
    GraphicsPipelineLayout = NULL_HANDLE;

    LOG(LogVulkan, "destroyed graphics pipeline layout");
}
*/
