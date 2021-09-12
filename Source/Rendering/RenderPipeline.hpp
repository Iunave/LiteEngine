#pragma once

#include "Definitions.hpp"
#include "VulkanCommon.hpp"
#include "SmartPointer.hpp"
#include "Thread/ThreadPool.hpp"
#include "Array.hpp"

class FRenderDevice;
class FRenderSwapChain;

struct FRenderConfigInfo : private FNonCopyable
{
    FRenderConfigInfo(Vk::Extent2D ImageExtent, Vk::RenderPass InRenderPass);

    Vk::RenderPass RenderPass;

    Vk::PipelineInputAssemblyStateCreateInfo AssemblyStateInfo;

    Vk::PipelineVertexInputStateCreateInfo VertexInputInfo;

    Vk::Viewport Viewport;
    Vk::Rect2D Scissor;

    Vk::PipelineViewportStateCreateInfo ViewportInfo;

    Vk::PipelineRasterizationStateCreateInfo RasterizerInfo;

    Vk::PipelineMultisampleStateCreateInfo MultisamplerInfo;

    Vk::PipelineDepthStencilStateCreateInfo DepthStencilInfo;

    Vk::PipelineColorBlendAttachmentState ColorBlendAttachmentState;

    Vk::PipelineColorBlendStateCreateInfo ColorBlendInfo;

    TStaticArray<Vk::DynamicState, 2> DynamicStates;

    Vk::PipelineDynamicStateCreateInfo DynamicStateInfo;

    Vk::PipelineLayoutCreateInfo PipelineLayoutInfo;
};

class FRenderPipeline final : private FNonCopyable
{
public:

    FRenderPipeline();

    virtual ~FRenderPipeline() override;

    void ShutDown();

    Vk::ShaderModule CreateShaderModule(const Vk::ShaderModuleCreateInfo ShaderCreateInfo);

    void ReadShaderFilesAsync(FString ShaderFileName);

    void CreateGraphicsPipeline(const FRenderConfigInfo& RenderConfigInfo);

    void Bind(Vk::CommandBuffer CommandBuffer);

    OBJECT_CLASS(OShaderCodeReader) : public ORunnable
    {
        OBJECT_BASES(ORunnable)
    public:

        INLINE explicit OShaderCodeReader(const FString& InFileName)
            : FileName{InFileName}
        {
        }

        virtual void Run() override;

        TDynamicArray<uint32> VertexCode;
        uint64 FragCodeStartIndex;

        FString FileName;
    };

    OShaderCodeReader ShaderCodeReader;

protected:

    Vk::Pipeline GraphicsPipeline;

    Vk::PipelineLayout PipelineLayout;

    Vk::ShaderModule VertexShaderModule;
    Vk::ShaderModule FragmentationShaderModule;
};
