#include "RenderPipeline.hpp"
#include "RenderDevice.hpp"
#include "EngineGlobals.hpp"
#include "Thread/ThreadPool.hpp"
#include <fmt/os.h>

constexpr FString ShaderPath{"../Shaders/Compiled/"};
constexpr FString FragShaderPostfix{".frag.spv"};
constexpr FString VertShaderPostfix{".vert.spv"};


void FRenderPipeline::OShaderCodeReader::Run()
{
    FileName <<= ShaderPath;
    FileName += VertShaderPostfix;

    fmt::file VertexInputFile{FileName.RawString(), fmt::file::RDONLY};

    FileName.Replace_Assign(FileName.Length() - VertShaderPostfix.Length(), FragShaderPostfix);

    fmt::file FragInputFile{FileName.RawString(), fmt::file::RDONLY};

    VertexCode.ResizeTo((VertexInputFile.size() + FragInputFile.size()) / 4);
    FragCodeStartIndex = FragInputFile.size() / 4;

    VertexInputFile.read(VertexCode.GetData(), VertexInputFile.size());
    VertexInputFile.close();

    FragInputFile.read(&VertexCode[FragCodeStartIndex], FragInputFile.size());
    FragInputFile.close();
}

FRenderPipeline::FRenderPipeline()
    : GraphicsPipeline{NULL_HANDLE}
    , PipelineLayout{NULL_HANDLE}
    , ShaderCode{nullptr}
    , VertexShaderModule{NULL_HANDLE}
    , FragmentationShaderModule{NULL_HANDLE}
{
}

FRenderPipeline::~FRenderPipeline()
{
}

void FRenderPipeline::ShutDown()
{
    const Vk::Device LogicalDevice{GRenderDevice.GetLogicalDevice()};

    if(LogicalDevice)
    {
        auto DestroyShaderModule = [LogicalDevice](Vk::ShaderModule Module) -> void
        {
            if(Module)
            {
                LogicalDevice.destroyShaderModule(Module);
            }
        };

        if(LogicalDevice)
        {
            DestroyShaderModule(VertexShaderModule);
            DestroyShaderModule(FragmentationShaderModule);

            if(PipelineLayout)
            {
                LogicalDevice.destroyPipelineLayout(PipelineLayout);
            }

            if(GraphicsPipeline)
            {
                LogicalDevice.destroyPipeline(GraphicsPipeline);
            }
        }
    }
}

void FRenderPipeline::ReadShaderFilesAsync(FString ShaderFileName)
{
    Thread::AsyncTask(&ReadShaderCode);
}

Vk::ShaderModule FRenderPipeline::CreateShaderModule(const Vk::ShaderModuleCreateInfo ShaderCreateInfo)
{
    const Vk::Device LogicalDevice{GRenderDevice.GetLogicalDevice()};
    ASSERT(LogicalDevice);

    Vk::ShaderModule ShaderModule{LogicalDevice.createShaderModule(ShaderCreateInfo)};

    return ShaderModule;
}

void FRenderPipeline::CreateGraphicsPipeline(const FRenderConfigInfo& RenderConfigInfo)
{
    const Vk::Device LogicalDevice{GRenderDevice.GetLogicalDevice()};
    ASSERT(LogicalDevice);

    ShaderCodeReader.WaitForCompletion();

    Vk::ShaderModuleCreateInfo ShaderModuleCreateInfo{};
    ShaderModuleCreateInfo.codeSize = (ShaderCodeReader.FragCodeStartIndex + 1) * 4;
    ShaderModuleCreateInfo.pCode = ShaderCodeReader.VertexCode.GetData();

    VertexShaderModule = CreateShaderModule(ShaderModuleCreateInfo);

    ShaderModuleCreateInfo.codeSize = ((ShaderCode->FragCodeStartIndex + 1) - ShaderCode->VertexCode.Num()) * 4;
    ShaderModuleCreateInfo.pCode = &ShaderCode->VertexCode[ShaderCode->FragCodeStartIndex];

    FragmentationShaderModule = CreateShaderModule(ShaderModuleCreateInfo);

    ShaderCode.Reset();

    Vk::PipelineShaderStageCreateInfo VertexShaderStageInfo{};
    VertexShaderStageInfo.stage = Vk::ShaderStageFlagBits::eVertex;
    VertexShaderStageInfo.module = VertexShaderModule;
    VertexShaderStageInfo.pName = "main";
    VertexShaderStageInfo.pNext = nullptr;
    VertexShaderStageInfo.pSpecializationInfo = nullptr;

    Vk::PipelineShaderStageCreateInfo FragmentationShaderStageInfo{};
    FragmentationShaderStageInfo.stage = Vk::ShaderStageFlagBits::eFragment;
    FragmentationShaderStageInfo.module = FragmentationShaderModule;
    FragmentationShaderStageInfo.pName = "main";
    FragmentationShaderStageInfo.pNext = nullptr;
    FragmentationShaderStageInfo.pSpecializationInfo = nullptr;

    const TStaticArray<Vk::PipelineShaderStageCreateInfo, 2> ShaderStages{VertexShaderStageInfo, FragmentationShaderStageInfo};

    PipelineLayout = LogicalDevice.createPipelineLayout(RenderConfigInfo.PipelineLayoutInfo);

    ASSERT(PipelineLayout);
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
    PipelineCreateInfo.layout = PipelineLayout;
    PipelineCreateInfo.subpass = 0; //index to subpass
    PipelineCreateInfo.basePipelineIndex = -1;
    PipelineCreateInfo.basePipelineHandle = NULL_HANDLE;

    const Vk::Result Result{LogicalDevice.createGraphicsPipelines(NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &GraphicsPipeline)};
    ASSERT(Result == Vk::Result::eSuccess, "could not create graphics pipelines", Vk::to_string(Result));

    LOG(LogVulkan, "created graphics pipelines");
}

void FRenderPipeline::Bind(Vk::CommandBuffer CommandBuffer)
{
    ASSERT(GraphicsPipeline);

    CommandBuffer.bindPipeline(Vk::PipelineBindPoint::eGraphics, GraphicsPipeline);
}
