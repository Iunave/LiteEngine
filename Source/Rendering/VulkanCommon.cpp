#define INCLUDE_ALL_VULKAN_FILES
#include "VulkanCommon.hpp"
#undef INCLUDE_ALL_VULKAN_FILES

FRenderWindow GRenderWindow{}
(0, 0,

FString(),

false);
TSharedPtr<FRenderSwapChain> GRenderSwapChain{MakeShared<FRenderSwapChain>(nullptr)};
FRenderPipeline GRenderPipeline{};
#if USE_VULKAN_VALIDATION_LAYERS
FRenderDevice GRenderDevice{{VK_KHR_SWAPCHAIN_EXTENSION_NAME}, {VK_KHR_VALIDATION_LAYER_NAME}};
#else
FRenderDevice GRenderDevice{{VK_KHR_SWAPCHAIN_EXTENSION_NAME}};
#endif

void vk::InitializeVulkan()
{
    GRenderPipeline.ReadShaderFilesAsync("TriangleShader");

    GRenderWindow.CreateWindow(600, 800, "My Amazing Window", false);

    GRenderDevice.Initialize();

    GRenderSwapChain->Initialize();

    FRenderConfigInfo RenderConfigInfo{GRenderSwapChain->GetImageExtent(), GRenderSwapChain->GetRenderPass()};
    GRenderPipeline.CreateGraphicsPipeline(RenderConfigInfo);
}

void vk::ShutDownVulkan()
{
    GRenderWindow.CloseWindow();

    GRenderPipeline.ShutDown();

    GRenderSwapChain->ShutDown();
    GRenderSwapChain.Reset();

    GRenderDevice.ShutDown();
}

