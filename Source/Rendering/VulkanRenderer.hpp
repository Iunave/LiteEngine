#pragma once

#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/Array.hpp"
#include "CoreFiles/String.hpp"
#include "Object/Object.hpp"
#include "Thread/ThreadPool.hpp"
#include "RenderWindow.hpp"

#include <vulkan/vulkan.hpp>

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

struct FSwapChainSupportDetails
{
    Vk::SurfaceCapabilitiesKHR SurfaceCapabilities;
    TDynamicArray<Vk::SurfaceFormatKHR> SurfaceFormats;
    TDynamicArray<Vk::PresentModeKHR> PresentModes;
};

struct FQueueFamilyIndices
{
    FQueueFamilyIndices()
        : Graphic{UINT32_MAX}
        , Compute{UINT32_MAX}
        , Protected{UINT32_MAX}
        , SparseBinding{UINT32_MAX}
        , Transfer{UINT32_MAX}
        , Presentation{UINT32_MAX}
    {
    }

    template<typename... Ts>
    static bool CheckValidity(Ts... Queues)
    {
        return ((Queues != UINT32_MAX) && ...);
    }

    uint32 Graphic;
    uint32 Compute;
    uint32 Protected;
    uint32 SparseBinding;
    uint32 Transfer;
    uint32 Presentation;
};

struct FShaderModulePair
{
    Vk::ShaderModule Vertex{NULL_HANDLE};
    Vk::ShaderModule Fragmentation{NULL_HANDLE};
};

OBJECT_CLASS(OShaderCodeReader) : public ORunnable
{
    OBJECT_BASES(ORunnable)
public:

    virtual void Run() override;

    TDynamicArray<uint32> VertexCode;
    TDynamicArray<uint32> FragmentationCode;

    FString<ss124> VertexFilePath;
    FString<ss124> FragmentationFilePath;
    FString<ss124> FileName;
};

namespace Handle
{
    inline Vk::DebugUtilsMessengerEXT DebugMessenger{NULL_HANDLE};
    inline Vk::Instance VulkanInstance{NULL_HANDLE};
    inline Vk::PhysicalDevice PhysicalDevice{NULL_HANDLE};
    inline Vk::Device LogicalDevice{NULL_HANDLE};
    inline Vk::SurfaceKHR Surface{NULL_HANDLE};
    inline Vk::Queue GraphicsQueue{NULL_HANDLE};
    inline Vk::Queue PresentationQueue{NULL_HANDLE};
    inline Vk::Queue ComputeQueue{NULL_HANDLE};
    inline Vk::SwapchainKHR SwapChain{NULL_HANDLE};

    inline TDynamicArray<Vk::Image> Images{};
    inline TDynamicArray<Vk::ImageView> ImageViews{};
    inline TDynamicArray<Vk::Framebuffer> FrameBuffers{};
    inline TDynamicArray<Vk::CommandBuffer> CommandBuffers{};

    inline Vk::Pipeline GraphicsPipeline{NULL_HANDLE};
    inline Vk::PipelineLayout PipelineLayout{NULL_HANDLE};

    inline FShaderModulePair ShaderModulePair{};
}

namespace Render
{
    void Initialize();
    void Loop();
    void ShutDown();

    namespace Data
    {
        inline constexpr FString<ss124> ShaderPath{"../Shaders/Compiled/"};
        inline constexpr FString<ss124> FragShaderPostfix{".frag.spv"};
        inline constexpr FString<ss124> VertShaderPostfix{".vert.spv"};

        extern uint32 VulkanVersion;
        extern uint32 VulkanAPIVersion;

        inline FRenderWindow Window{};

        extern TDynamicArray<const char8*> ValidationLayers;
        extern TDynamicArray<const char8*> ExtensionLayers;
        extern TDynamicArray<const char8*> DeviceExtensions;

        inline Vk::Extent2D ImageExtent{};
        inline Vk::SurfaceFormatKHR SurfaceFormat{};
        inline Vk::PresentModeKHR PresentMode{};

        inline FQueueFamilyIndices QueueFamilyIndices{};
        inline FSwapChainSupportDetails SwapChainSupportDetails{};

        inline OShaderCodeReader ShaderCodeReader{};
    }

    namespace Device
    {
        TDynamicArray<Vk::PhysicalDevice> FindAvailablePhysicalDevices(Vk::Instance VulkanInstance);
        TDynamicArray<Vk::ExtensionProperties> FindPhysicalDeviceProperties(Vk::PhysicalDevice PhysicalDevice);
        TDynamicArray<Vk::QueueFamilyProperties> FindQueueFamilyProperties(Vk::PhysicalDevice PhysicalDevice);

        FQueueFamilyIndices FindQueueFamilyIndices(Vk::PhysicalDevice PhysicalDevice, Vk::SurfaceKHR Surface);

        bool DoesPhysicalDeviceSupportExtensions(Vk::PhysicalDevice PhysicalDevice, const TDynamicArray<const char8*>& NeededDeviceExtensions);
        bool DoesPhysicalDeviceSupportFeatures(Vk::PhysicalDevice PhysicalDevice);

        Vk::SurfaceCapabilitiesKHR FindSurfaceCapabilities(Vk::PhysicalDevice PhysicalDevice, Vk::SurfaceKHR Surface);
        TDynamicArray<Vk::SurfaceFormatKHR> FindSurfaceFormats(Vk::PhysicalDevice PhysicalDevice, Vk::SurfaceKHR Surface);
        TDynamicArray<Vk::PresentModeKHR> FindSurfacePresentModes(Vk::PhysicalDevice PhysicalDevice, Vk::SurfaceKHR Surface);

        FSwapChainSupportDetails FillSwapChainSupportDetails(Vk::PhysicalDevice PhysicalDevice, Vk::SurfaceKHR Surface);

        Vk::PhysicalDevice PickPhysicalDevice(Vk::Instance VulkanInstance, Vk::SurfaceKHR Surface, const TDynamicArray<const char8*>& NeededDeviceExtensions);

        Vk::Device CreateLogicalDevice(Vk::PhysicalDevice PhysicalDevice, const FQueueFamilyIndices& QueueFamilyIndices);
        void DestroyLogicalDevice(Vk::Device);

        Vk::Queue GetDeviceQueueHandle(Vk::Device LogicalDevice, uint32 QueueIndex);
    }

    namespace SwapChain
    {
        TDynamicArray<Vk::Image> FindSwapChainImages(Vk::Device LogicalDevice, Vk::SwapchainKHR SwapChain);
        TDynamicArray<Vk::ImageView> CreateImageViews(Vk::Device LogicalDevice, const TDynamicArray<Vk::Image>& SourceImages);

        void DestroyImageViews(Vk::Device LogicalDevice, TDynamicArray<Vk::ImageView>& ImageViews);
        void DestroyImages(Vk::Device LogicalDevice, TDynamicArray<Vk::Image>& Images);

        Vk::Extent2D ChooseImageExtent(const Vk::SurfaceCapabilitiesKHR SurfaceCapabilities);
        Vk::SurfaceFormatKHR ChooseSurfaceFormat(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats);
        Vk::PresentModeKHR ChoosePresentationMode(const TDynamicArray<Vk::PresentModeKHR>& PresentModes);

        Vk::SwapchainKHR CreateSwapChain(Vk::Device LogicalDevice, Vk::SurfaceKHR Surface, Vk::SwapchainKHR OldSwapChain = NULL_HANDLE);
        void DestroySwapChain(Vk::SwapchainKHR SwapChain, Vk::Device LogicalDevice);

        Vk::SurfaceKHR CreateSurface(Vk::Instance VulkanInstance, GLFWwindow* RenderWindow);
        void DestroySurface(Vk::Instance VulkanInstance, Vk::SurfaceKHR Surface);
    }

    namespace Pipeline
    {
        Vk::Pipeline CreateGraphicsPipeline(Vk::Device LogicalDevice, FShaderModulePair ShaderModulePair, const FRenderConfigInfo& RenderConfigInfo);

        Vk::PipelineLayout CreatePipelineLayout(Vk::Device LogicalDevice, const FRenderConfigInfo& RenderConfigInfo);

        FShaderModulePair CreateShaderModulePair(Vk::Device LogicalDevice, OShaderCodeReader* ShaderCodeReader);
    }
}
