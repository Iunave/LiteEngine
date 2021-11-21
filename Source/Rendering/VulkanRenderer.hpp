#pragma once

#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/Array.hpp"
#include "CoreFiles/String.hpp"
#include "Object/Object.hpp"
#include "Interface/IniConfig.hpp"
#include "Thread/ThreadPool.hpp"
#include "RenderWindow.hpp"

#include <vulkan/vulkan.hpp>


OBJECT_CLASS(OShaderCodeReader)
class OShaderCodeReader : public ORunnable
{
    OBJECT_BASES(ORunnable)
public:

    OShaderCodeReader();
    OShaderCodeReader(const FString<SS124>& InFileName);

    ~OShaderCodeReader();

    virtual void Run() override;

    uint8* Allocation;

    uint8* VertexCode;
    int64 VertexCodeSize;

    uint8* FragCode;
    int64 FragCodeSize;

    FString<SS124> FileName;
};

namespace Render
{
    inline constexpr FString RelativeShaderPath{"/Shaders/Compiled/"};
    inline constexpr FString FragShaderPostfix{".frag.spv"};
    inline constexpr FString VertShaderPostfix{".vert.spv"};

    inline constexpr int64 MaxFramesInFlight{3};

    struct FQueueFamilyIndices
    {
        FQueueFamilyIndices();

        template<typename... Ts>
        static bool CheckValidity(Ts... Indices)
        {
            return ((Indices != UINT32_MAX) && ...);
        }

        uint32 Graphic;
        uint32 Compute;
        uint32 Protected;
        uint32 SparseBinding;
        uint32 Transfer;
        uint32 Presentation;
    };

    struct FQueueHandles
    {
        Vk::Queue Graphics;
        Vk::Queue Presentation;
        Vk::Queue Compute;
    };

    struct FSwapChainSupportDetails
    {
        Vk::SurfaceCapabilitiesKHR SurfaceCapabilities;
        TDynamicArray<Vk::SurfaceFormatKHR> SurfaceFormats;
        TDynamicArray<Vk::PresentModeKHR> PresentModes;
    };

    struct FSwapChainData
    {
        Vk::Extent2D ImageExtent;
        Vk::SurfaceFormatKHR SurfaceFormat;
        Vk::PresentModeKHR PresentMode;
    };

    struct FShaderModulePair
    {
        Vk::ShaderModule Vertex{NULL_HANDLE};
        Vk::ShaderModule Fragmentation{NULL_HANDLE};
    };

    struct FFrameData
    {
        Vk::Semaphore ImageAvailableSemaphore;
        Vk::Semaphore RenderFinishedSemaphore;
        Vk::Fence FlightFence;
    };

    struct FBuffer
    {
        Vk::Buffer BufferHandle;
        Vk::DeviceMemory MemoryHandle;
    };

    struct FRenderConfigInfo
    {
        FRenderConfigInfo(Vk::Extent2D ImageExtent);

        Vk::PipelineInputAssemblyStateCreateInfo AssemblyStateInfo;
        Vk::PipelineVertexInputStateCreateInfo VertexInputInfo;

        Vk::Viewport Viewport;
        Vk::Rect2D Scissor;
        Vk::Extent2D ImageExtent;

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

    void Initialize();
    attr(hot) void Loop();
    void ShutDown();
}

OBJECT_CLASS_NAMESPACED(Render, OVulkanManager)
class Render::OVulkanManager
{
    OBJECT_BASES()
public:

    OVulkanManager();

    static VKAPI_ATTR uint32 VKAPI_CALL VulkanDebugCallback(Vk::DebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, Vk::DebugUtilsMessageTypeFlagsEXT MessageType, const Vk::DebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData);
    static Vk::DebugUtilsMessengerCreateInfoEXT MakeDebugUtilsMessengerCreateInfo();

    void CreateMessenger();
    void DestroyMessenger();

    void CreateWindow();
    void DestroyWindow();

    void CreateInstance();
    void DestroyInstance();

    void PopulateValidationLayers();
    void PopulateInstanceExtensionLayers();
    void PopulateDeviceExtensionLayers();

    TDynamicArray<Vk::ExtensionProperties> FindPhysicalDeviceProperties();
    TDynamicArray<Vk::QueueFamilyProperties> FindQueueFamilyProperties();

    void PopulateQueueFamilyIndices();

    bool DoesPhysicalDeviceSupportExtensions();
    bool DoesPhysicalDeviceSupportFeatures();
    bool DoesPhysicalDeviceSupportQueues();
    bool DoesPhysicalDeviceSupportSwapChain();

    Vk::SurfaceCapabilitiesKHR FindSurfaceCapabilities();
    TDynamicArray<Vk::SurfaceFormatKHR> FindSurfaceFormats();
    TDynamicArray<Vk::PresentModeKHR> FindSurfacePresentModes();

    TDynamicArray<Vk::PhysicalDevice> FindAvailablePhysicalDevices();

    void PopulateSwapChainSupportDetails();

    void CreateSurface();
    void DestroySurface();

    void PickPhysicalDevice();
    void DestroyPhysicalDevice();

    void CreateLogicalDevice();
    void DestroyLogicalDevice();

    void InitializeQueues();

    void PopulateImages();
    void DestroyImages();

    void CreateImageViews();
    void DestroyImageViews();

    Vk::Extent2D ChooseImageExtent();
    Vk::SurfaceFormatKHR ChooseSurfaceFormat();
    Vk::PresentModeKHR ChoosePresentationMode();

    void PopulateSwapChainData();

    void CreateSwapChain(Vk::SwapchainKHR OldSwapChain = NULL_HANDLE);
    void RecreateSwapChain();

    bool SwapChainNeedsRecreation(Vk::Result Result) const;

    void DestroySwapChain();

    void CreateRenderPass();
    void DestroyRenderPass();

    void CreateGraphicsPipeline(FShaderModulePair ShaderModulePair, const FRenderConfigInfo& RenderConfigInfo);
    void DestroyGraphicsPipeline();

    void DestroyPipelineLayout();

    FShaderModulePair CreateShaderModulePair();

    void CreateFrameBuffers();
    void DestroyFrameBuffers();

    void CreateDrawingCommandPool();
    void DestroyDrawingCommandPool();

    void CreateVertexBuffer();
    void DestroyVertexBuffer();

    uint32 FindMemoryType(uint32 TypeFilter, Vk::MemoryPropertyFlags Flags) const;

    void CreateCommandBuffers();
    void FreeCommandBuffers();

    void BeginRenderPass();

    void CreateFrameSyncData();
    void DestroyFrameSyncData();

    void DrawFrame();

public:

    OShaderCodeReader ShaderCodeReader;

    OWindow RenderWindow;

    Vk::Instance InstanceHandle;
    Vk::DebugUtilsMessengerEXT DebugMessengerHandle;
    Vk::SurfaceKHR SurfaceHandle;
    Vk::PhysicalDevice PhysicalDeviceHandle;
    Vk::Device LogicalDeviceHandle;
    Vk::SwapchainKHR SwapChainHandle;
    Vk::Pipeline GraphicsPipelineHandle;
    Vk::RenderPass RenderPassHandle;
    Vk::CommandPool CommandPool;

    Vk::PipelineLayout GraphicsPipelineLayout;

    TStaticArray<FFrameData, MaxFramesInFlight> FrameData;
    TDynamicArray<Vk::Fence> ImagesInFlightFence;

    TDynamicArray<const char8*> InstanceExtensionLayers;
    TDynamicArray<const char8*> ValidationLayers;
    TDynamicArray<const char8*> DeviceExtensions;

    FQueueFamilyIndices QueueIndices;
    FQueueHandles QueueHandles;

    FSwapChainSupportDetails SwapChainSupportDetails;
    FSwapChainData SwapChainData;

    TDynamicArray<Vk::Image> SwapChainImages;
    TDynamicArray<Vk::ImageView> SwapChainImageViews;
    TDynamicArray<Vk::Framebuffer> SwapChainFrameBuffers;

    Vk::Buffer VertexBuffer;
    Vk::DeviceMemory VertexMemoryHandle;

    TDynamicArray<Vk::CommandBuffer> CommandBuffers;

    int64 CurrentFrame;

    uint32 VulkanVersion;
    uint32 VulkanAPIVersion;
};
