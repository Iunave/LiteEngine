#include "VulkanRenderer.hpp"
#include "CoreFiles/Log.hpp"
#include "RenderWindow.hpp"
#include "VulkanLibrary.hpp"
#include "Vertex.hpp"
#include "CoreFiles/Time.hpp"
#include "Interface/Tick.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

inline bool operator!(const Vk::Result VulkanResult)
{
    return VulkanResult != Vk::Result::eSuccess;
}

struct FVertexBuffer
{
    Vk::Buffer StagingBuffer{NULL_HANDLE};
    Vk::DeviceMemory StagingMemory{NULL_HANDLE};

    Vk::Buffer DeviceBuffer{NULL_HANDLE};
    Vk::DeviceMemory DeviceMemory{NULL_HANDLE};
};

class FVulkanManager : public FWindow
{
public:

    static FVulkanManager& Instance()
    {
        static FVulkanManager Instance{};
        return Instance;
    }

    FVulkanManager();

    void Initialize();
    void DrawFrame();
    void Shutdown();

private:

    void RecreateSwapChain();
    void CleanupSwapChain();

    void InitializeFrameSyncData();
    void DestroyFrameSyncData();

    void CreateVertexBuffer();

    Vk::Instance InstanceHandle;
    Vk::DebugUtilsMessengerEXT DebugMessenger;
    Vk::SurfaceKHR Surface;
    Vk::PhysicalDevice PhysicalDevice;
    Vk::Device LogicalDevice;
    Vk::SwapchainKHR SwapChain;
    Vk::PipelineLayout GraphicsPipelineLayout;
    Vk::Pipeline GraphicsPipeline;
    Vk::RenderPass RenderPass;
    Vk::CommandPool DrawingCommandPool; //presentation / graphics
    Vk::CommandPool TransferCommandPool;
    Vk::ShaderModule VertShaderModule;
    Vk::ShaderModule FragShaderModule;

    FVertexBuffer VertexBuffer;

    TDynamicArray<uint8> VertShaderCode;
    TDynamicArray<uint8> FragShaderCode;

    TDynamicArray<Vk::CommandBuffer> DrawingCommandBuffers;
    TDynamicArray<Vk::Framebuffer> FrameBuffers;

    TStaticArray<Vk::Semaphore, Render::MaxFramesInFlight> ImageAvailableSemaphores;
    TStaticArray<Vk::Semaphore, Render::MaxFramesInFlight> RenderFinishedSemaphores;
    TStaticArray<Vk::Fence, Render::MaxFramesInFlight> ImageInFlightFences;
    TDynamicArray<Vk::Fence> ImageInFlightFencesCopy;

    Render::FQueueFamilies Queues;

    Vk::Queue GraphicsQueue;
    Vk::Queue PresentationQueue;
    Vk::Queue TransferQueue;

    Vk::SurfaceCapabilitiesKHR SwapChainSurfaceCapabilities;
    TDynamicArray<Vk::SurfaceFormatKHR> SwapChainSurfaceFormats;
    TDynamicArray<Vk::PresentModeKHR> SwapChainPresentModes;

    Vk::Extent2D SwapChainImageExtent;
    Vk::SurfaceFormatKHR SwapChainSurfaceFormat;
    Vk::PresentModeKHR SwapChainPresentMode;

    TDynamicArray<Vk::Image> SwapChainImages;
    TDynamicArray<Vk::ImageView> SwapChainImageViews;
    TDynamicArray<Vk::Framebuffer> SwapChainFrameBuffers;

    TDynamicArray<FVertex> VertexArray;

    int64 CurrentFrame;
};

void Render::Initialize()
{
    FVulkanManager::Instance().Initialize();
}

void Render::Loop()
{
    float64 StartTime{0};
    float64 EndTime{0};
    float64 DeltaTime{0};

    while(!FVulkanManager::Instance().ShouldClose())
    {
        StartTime = Time::Now();

        glfwPollEvents();

        FVulkanManager::Instance().DrawFrame();
        FTickManager::Instance().Tick(DeltaTime);

        EndTime = Time::Now();
        DeltaTime = (EndTime - StartTime);
    }
}

void Render::Shutdown()
{
    FVulkanManager::Instance().Shutdown();
}

FVulkanManager::FVulkanManager()
    : FWindow{}
    , InstanceHandle{NULL_HANDLE}
    , DebugMessenger{NULL_HANDLE}
    , Surface{NULL_HANDLE}
    , PhysicalDevice{NULL_HANDLE}
    , LogicalDevice{NULL_HANDLE}
    , SwapChain{NULL_HANDLE}
    , GraphicsPipeline{NULL_HANDLE}
    , RenderPass{NULL_HANDLE}
    , DrawingCommandPool{NULL_HANDLE}
    , VertShaderModule{NULL_HANDLE}
    , FragShaderModule{NULL_HANDLE}
    , CurrentFrame{0}
{
    VertexArray.Append(FVector2D{0.0f, -0.5f}, RGBA32F{1.0f, 0.0f, 1.0f, 1.0f});
    VertexArray.Append(FVector2D{0.5f, 0.5f}, RGBA32F{0.0f, 1.0f, 0.0f, 1.0f});
    VertexArray.Append(FVector2D{-0.5f, 0.5f}, RGBA32F{0.0f, 0.0f, 1.0f, 1.0f});
}

void FVulkanManager::Initialize()
{
    LOG(LogVulkan, "starting initializing vulkan");

    CreateWindow(500, 500, "my amazing window");

    Render::ValidationLayers = Render::FindValidationLayers();
    Render::InstanceExtensionLayers = Render::FindInstanceExtensionLayers();
    Render::DeviceExtensionLayers = Render::FindDeviceExtensionLayers();

    InstanceHandle = Render::CreateInstance(ValidationLayers, InstanceExtensionLayers);
    DebugMessenger = Render::CreateDebugMessenger(InstanceHandle);
    Surface = Render::CreateSurface(Vk::Instance(), InstanceHandle);
    PhysicalDevice = Render::PickPhysicalDevice(Vk::Instance(), Vk::SurfaceKHR(), <#initializer#>);

    Queues = Render::PopulateQueueFamilyIndices(PhysicalDevice, Surface);

    SwapChainPresentModes = Render::FindSurfacePresentModes(PhysicalDevice, Surface);
    SwapChainSurfaceFormats = Render::FindSurfaceFormats(PhysicalDevice, Surface);
    SwapChainSurfaceCapabilities = Render::FindSurfaceCapabilities(PhysicalDevice, Surface);

    SwapChainImageExtent = Render::ChooseImageExtent(WindowDimensions, SwapChainSurfaceCapabilities);
    SwapChainSurfaceFormat = Render::ChooseSurfaceFormat(SwapChainSurfaceFormats);
    SwapChainPresentMode = Render::ChoosePresentationMode(SwapChainPresentModes);

    LogicalDevice = Render::CreateLogicalDevice(Vk::PhysicalDevice(), Render::FQueueFamilies(), <#initializer#>, <#initializer#>);

    Queues.Graphic.Handle = LogicalDevice.getQueue(Queues.Graphic.Index, 0);
    Queues.Presentation.Handle = LogicalDevice.getQueue(Queues.Presentation.Index, 0);
    Queues.Transfer.Handle = LogicalDevice.getQueue(Queues.Transfer.Index, 0);

    SwapChain = Render::CreateSwapChain(NULL_HANDLE, Vk::Device(), Vk::SurfaceKHR(), Vk::SurfaceCapabilitiesKHR(), Render::FQueueFamilies(), Vk::Extent2D(), Vk::PresentModeKHR::eImmediate,
                                        Vk::SurfaceFormatKHR());

    SwapChainImages = Render::FindSwapChainImages(Vk::Device(), Vk::SwapchainKHR());
    SwapChainImageViews = Render::CreateSwapChainImageViews(Vk::Device(), <#initializer#>, Vk::SurfaceFormatKHR());

    RenderPass = Render::CreateRenderPass(LogicalDevice, SwapChainSurfaceFormat);

    VertShaderCode = Render::ReadShaderFile(FString<SS124>{"TriangleShader.vert.spv"});
    FragShaderCode = Render::ReadShaderFile(FString<SS124>{"TriangleShader.frag.spv"});

    VertShaderModule = Render::CreateShaderModule(Vk::Device(), LogicalDevice);
    FragShaderModule = Render::CreateShaderModule(Vk::Device(), LogicalDevice);

    GraphicsPipelineLayout = Render::CreateGraphicsPipelineLayout(LogicalDevice);
    GraphicsPipeline = Render::CreateGraphicsPipeline(LogicalDevice, RenderPass, GraphicsPipelineLayout, FragShaderModule, VertShaderModule, SwapChainImageExtent);

    LogicalDevice.destroyShaderModule(VertShaderModule);
    LogicalDevice.destroyShaderModule(FragShaderModule);

    FrameBuffers = Render::CreateFrameBuffers(LogicalDevice, RenderPass, SwapChainImageExtent, SwapChainImageViews);
    DrawingCommandPool = Render::CreateCommandPool(LogicalDevice, Queues.Presentation.Index);
    DrawingCommandBuffers = Render::CreateCommandBuffers(LogicalDevice, DrawingCommandPool, FrameBuffers.Num());

    TransferCommandPool = Render::CreateCommandPoolTransient(LogicalDevice, Queues.Transfer.Index);

    CreateVertexBuffer();

    Render::BeginRenderPass(VertexArray.Num(), RenderPass, GraphicsPipeline, VertexBuffer.DeviceBuffer, SwapChainImageExtent, FrameBuffers, DrawingCommandBuffers);

    InitializeFrameSyncData();

    LOG(LogVulkan, "finished initializing vulkan");
}

void FVulkanManager::DrawFrame()
{
    CHECK(!!LogicalDevice.waitForFences(1, &ImageInFlightFences[CurrentFrame], true, UINT64_MAX));

    uint32 ImageIndex;
    Vk::Result Result{LogicalDevice.acquireNextImageKHR(SwapChain, UINT64_MAX, ImageAvailableSemaphores[CurrentFrame], NULL_HANDLE, &ImageIndex)};

    if EXPECT(Render::SwapChainNeedsRecreation(Result) || HasBeenResized, false)
    {
        HasBeenResized = false;
        RecreateSwapChain();
        return;
    }

    if(!!ImageInFlightFencesCopy[ImageIndex])
    {
        CHECK(!!LogicalDevice.waitForFences(1, &ImageInFlightFencesCopy[ImageIndex], true, UINT64_MAX));
    }

    ImageInFlightFencesCopy[ImageIndex] = ImageInFlightFences[CurrentFrame];

    TStaticArray<Vk::Semaphore, 1> WaitSemaphores{ImageAvailableSemaphores[CurrentFrame]};
    TStaticArray<Vk::Semaphore, 1> SignalSemaphores{RenderFinishedSemaphores[CurrentFrame]};
    TStaticArray<Vk::PipelineStageFlags, 1> WaitStages{Vk::PipelineStageFlagBits::eColorAttachmentOutput};

    Vk::SubmitInfo SubmitInfo{};
    SubmitInfo.waitSemaphoreCount = WaitSemaphores.Num();
    SubmitInfo.pWaitSemaphores = WaitSemaphores.Data();
    SubmitInfo.pWaitDstStageMask = WaitStages.Data();
    SubmitInfo.signalSemaphoreCount = SignalSemaphores.Num();
    SubmitInfo.pSignalSemaphores = SignalSemaphores.Data();
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &DrawingCommandBuffers[ImageIndex];

    CHECK(!!LogicalDevice.resetFences(1, &ImageInFlightFences[CurrentFrame]));

    CHECK(!!GraphicsQueue.submit(1, &SubmitInfo, ImageInFlightFences[CurrentFrame]));

    TStaticArray<Vk::SwapchainKHR, 1> SwapChains{SwapChain};

    Vk::PresentInfoKHR PresentInfo{};
    PresentInfo.waitSemaphoreCount = SignalSemaphores.Num();
    PresentInfo.pWaitSemaphores = SignalSemaphores.Data();
    PresentInfo.swapchainCount = SwapChains.Num();
    PresentInfo.pSwapchains = SwapChains.Data();
    PresentInfo.pImageIndices = &ImageIndex;
    PresentInfo.pResults = nullptr;

    Result = PresentationQueue.presentKHR(&PresentInfo);

    if EXPECT(Render::SwapChainNeedsRecreation(Result) || HasBeenResized, false)
    {
        HasBeenResized = false;
        RecreateSwapChain();
        return;
    }

    ++CurrentFrame;
    CurrentFrame %= Render::MaxFramesInFlight;
}

void FVulkanManager::Shutdown()
{
    LogicalDevice.waitIdle();

    LOG(LogVulkan, "starting shutting down vulkan");

    CleanupSwapChain();

    LogicalDevice.destroySwapchainKHR(SwapChain);

    InstanceHandle.destroySurfaceKHR(Surface);

    LogicalDevice.destroyBuffer(VertexBuffer.StagingBuffer);
    LogicalDevice.freeMemory(VertexBuffer.StagingMemory);

    LogicalDevice.destroyBuffer(VertexBuffer.DeviceBuffer);
    LogicalDevice.freeMemory(VertexBuffer.DeviceMemory);

    LogicalDevice.destroy();

    Render::DestroyDebugMessenger(InstanceHandle, DebugMessenger);

    InstanceHandle.destroy();

    CloseWindow();

    LOG(LogVulkan, "finished shutting down vulkan");
}

void FVulkanManager::CleanupSwapChain()
{
    LogicalDevice.freeCommandBuffers(DrawingCommandPool, DrawingCommandBuffers.Num(), DrawingCommandBuffers.Data());

    for(Vk::Framebuffer FrameBuffer : FrameBuffers)
    {
        LogicalDevice.destroyFramebuffer(FrameBuffer);
    }

    LogicalDevice.destroyCommandPool(DrawingCommandPool);
    LogicalDevice.destroyCommandPool(TransferCommandPool);
    LogicalDevice.destroyPipeline(GraphicsPipeline);
    LogicalDevice.destroyPipelineLayout(GraphicsPipelineLayout);
    LogicalDevice.destroyRenderPass(RenderPass);

    DestroyFrameSyncData();
}

void FVulkanManager::RecreateSwapChain()
{
    while EXPECT(IsMinimized(), false)
    {
        UpdateWindowDimensions();
        glfwWaitEvents();
    }

    LogicalDevice.waitIdle();

    LOG(LogVulkan, "started recreating swapchain");

    CleanupSwapChain();

    SwapChainPresentModes = Render::FindSurfacePresentModes(PhysicalDevice, Surface);
    SwapChainSurfaceFormats = Render::FindSurfaceFormats(PhysicalDevice, Surface);
    SwapChainSurfaceCapabilities = Render::FindSurfaceCapabilities(PhysicalDevice, Surface);

    SwapChainImageExtent = Render::ChooseImageExtent(WindowDimensions, SwapChainSurfaceCapabilities);
    SwapChainSurfaceFormat = Render::ChooseSurfaceFormat(SwapChainSurfaceFormats);
    SwapChainPresentMode = Render::ChoosePresentationMode(SwapChainPresentModes);

    SwapChain = Render::CreateSwapChain(SwapChain, Vk::Device(), Vk::SurfaceKHR(), Vk::SurfaceCapabilitiesKHR(), Render::FQueueFamilies(), Vk::Extent2D(), Vk::PresentModeKHR::eImmediate,
                                        Vk::SurfaceFormatKHR());

    SwapChainImages = Render::FindSwapChainImages(Vk::Device(), Vk::SwapchainKHR());
    SwapChainImageViews = Render::CreateSwapChainImageViews(Vk::Device(), <#initializer#>, Vk::SurfaceFormatKHR());

    RenderPass = Render::CreateRenderPass(LogicalDevice, SwapChainSurfaceFormat);

    VertShaderModule = Render::CreateShaderModule(Vk::Device(), LogicalDevice);
    FragShaderModule = Render::CreateShaderModule(Vk::Device(), LogicalDevice);

    GraphicsPipelineLayout = Render::CreateGraphicsPipelineLayout(LogicalDevice);
    GraphicsPipeline = Render::CreateGraphicsPipeline(LogicalDevice, RenderPass, GraphicsPipelineLayout, FragShaderModule, VertShaderModule, SwapChainImageExtent);

    LogicalDevice.destroyShaderModule(VertShaderModule);
    LogicalDevice.destroyShaderModule(FragShaderModule);

    FrameBuffers = Render::CreateFrameBuffers(LogicalDevice, RenderPass, SwapChainImageExtent, SwapChainImageViews);
    DrawingCommandPool = Render::CreateCommandPool(LogicalDevice, Queues.Presentation.Index);
    DrawingCommandBuffers = Render::CreateCommandBuffers(LogicalDevice, DrawingCommandPool, FrameBuffers.Num());

    TransferCommandPool = Render::CreateCommandPoolTransient(LogicalDevice, Queues.Transfer.Index);

    InitializeFrameSyncData();

    Render::BeginRenderPass(VertexArray.Num(), RenderPass, GraphicsPipeline, VertexBuffer.DeviceBuffer, SwapChainImageExtent, FrameBuffers, DrawingCommandBuffers);

    LOG(LogVulkan, "finished recreating swapchain");
}

void FVulkanManager::CreateVertexBuffer()
{
    constexpr Vk::BufferUsageFlags StagingBufferUsageFlags{Vk::BufferUsageFlagBits::eTransferSrc};
    constexpr Vk::MemoryPropertyFlags StagingMemoryFlags{Vk::MemoryPropertyFlagBits::eHostVisible | Vk::MemoryPropertyFlagBits::eHostCoherent};

    constexpr Vk::BufferUsageFlags DeviceBufferUsageFlags{Vk::BufferUsageFlagBits::eTransferDst | Vk::BufferUsageFlagBits::eVertexBuffer};
    constexpr Vk::MemoryPropertyFlags DeviceMemoryFlags{Vk::MemoryPropertyFlagBits::eDeviceLocal};

    VertexBuffer.StagingBuffer = Render::CreateBuffer(LogicalDevice, StagingBufferUsageFlags, VertexArray.UsedSize());
    VertexBuffer.StagingMemory = Render::AllocateGPUMemory(PhysicalDevice, LogicalDevice, VertexBuffer.StagingBuffer, StagingMemoryFlags);

    VertexBuffer.DeviceBuffer = Render::CreateBuffer(LogicalDevice, DeviceBufferUsageFlags, VertexArray.UsedSize());
    VertexBuffer.DeviceMemory = Render::AllocateGPUMemory(PhysicalDevice, LogicalDevice, VertexBuffer.DeviceBuffer, DeviceMemoryFlags);

    Render::CopyDataToGPU(LogicalDevice, VertexBuffer.StagingBuffer, VertexBuffer.StagingMemory, VertexArray.Data(), VertexArray.Num());
    Render::CopyBuffer(VertexBuffer.StagingBuffer, VertexBuffer.DeviceBuffer, LogicalDevice, TransferCommandPool, TransferQueue, VertexArray.Num());

    LogicalDevice.destroyBuffer(VertexBuffer.StagingBuffer);
    LogicalDevice.freeMemory(VertexBuffer.StagingMemory);
}

void FVulkanManager::InitializeFrameSyncData()
{
    ImageInFlightFencesCopy.ResizeTo(SwapChainImageViews.Num());

    for(Vk::Fence& Fence : ImageInFlightFencesCopy)
    {
        Fence = NULL_HANDLE;
    }

    for(Vk::Semaphore& Semaphore : ImageAvailableSemaphores)
    {
        Semaphore = Render::CreateSemaphore(LogicalDevice);
    }

    for(Vk::Semaphore& Semaphore : RenderFinishedSemaphores)
    {
        Semaphore = Render::CreateSemaphore(LogicalDevice);
    }

    for(Vk::Fence& Fence : ImageInFlightFences)
    {
        Fence = Render::CreateFence(LogicalDevice);
    }

    LOG(LogVulkan, "created frame sync data");
}

void FVulkanManager::DestroyFrameSyncData()
{
    for(Vk::ImageView ImageView : SwapChainImageViews)
    {
        LogicalDevice.destroyImageView(ImageView);
    }

    for(Vk::Semaphore& Semaphore : ImageAvailableSemaphores)
    {
        LogicalDevice.destroySemaphore(Semaphore);
    }

    for(Vk::Semaphore& Semaphore : RenderFinishedSemaphores)
    {
        LogicalDevice.destroySemaphore(Semaphore);
    }

    for(Vk::Fence& Fence : ImageInFlightFences)
    {
        LogicalDevice.destroyFence(Fence);
    }

    LOG(LogVulkan, "destroyed frame sync data");
}
