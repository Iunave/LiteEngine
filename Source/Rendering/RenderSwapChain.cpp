#include "RenderSwapChain.hpp"
#include "RenderDevice.hpp"
#include "RenderWindow.hpp"
#include "EngineGlobals.hpp"

FRenderSwapChain::FRenderSwapChain(TSharedPtr<FRenderSwapChain> InOldSwapChain)
    : OldSwapChain{Move(InOldSwapChain)}
    , SwapChain{NULL_HANDLE}
    , RenderPass{NULL_HANDLE}
{
    if(OldSwapChain)
    {
        OldSwapChain->OldSwapChain.Reset();
    }
}

void FRenderSwapChain::Initialize()
{
    CreateSwapChain();

    CreateImageViews();

    CreateRenderPass();

    CreateDepthResources();

    CreateFrameBuffers();

    CreateSyncObjects();
}

void FRenderSwapChain::ShutDown()
{
    const Vk::Device LogicalDevice{GRenderDevice.GetLogicalDevice()};

    if(LogicalDevice)
    {
        for(Vk::ImageView ImageView : ImageViews)
        {
            if(ImageView)
            {
                LogicalDevice.destroyImageView(ImageView);
            }
        }

        if(SwapChain)
        {
            LogicalDevice.destroySwapchainKHR(SwapChain);
        }

        for(Vk::Framebuffer Framebuffer : FrameBuffers)
        {
            if(Framebuffer)
            {
                LogicalDevice.destroyFramebuffer(Framebuffer);
            }
        }

        if(RenderPass)
        {
            LogicalDevice.destroyRenderPass(RenderPass);
        }
    }
}

FRenderSwapChain::~FRenderSwapChain()
{
}

Vk::SwapchainKHR FRenderSwapChain::GetSwapChain() const
{
    return SwapChain;
}

Vk::RenderPass FRenderSwapChain::GetRenderPass() const
{
    return RenderPass;
}

Vk::Extent2D FRenderSwapChain::GetImageExtent() const
{
    return ImageExtent;
}

TDynamicArray<Vk::Image> FRenderSwapChain::FindSwapChainImages() const
{
    const Vk::Device LogicalDevice{GRenderDevice.GetLogicalDevice()};
    ASSERT(LogicalDevice && SwapChain);

    TDynamicArray<Vk::Image> FoundImages{};

    uint32 ImageCount{0};
    Vk::Result Result{LogicalDevice.getSwapchainImagesKHR(SwapChain, &ImageCount, nullptr)};
    ASSERT(Result == Vk::Result::eSuccess, "failed to get swapchain images {}", Vk::to_string(Result));

    if(ImageCount > 0)
    {
        FoundImages.ResizeTo(ImageCount);

        Result = LogicalDevice.getSwapchainImagesKHR(SwapChain, &ImageCount, FoundImages.GetData());
        ASSERT(Result == Vk::Result::eSuccess, "failed to get swapchain images {}", Vk::to_string(Result));
    }

    return FoundImages;
}

Vk::Extent2D FRenderSwapChain::ChooseImageExtent(const Vk::SurfaceCapabilitiesKHR SurfaceCapabilities)
{
    if(SurfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        return SurfaceCapabilities.currentExtent;
    }

    const uint32 ClampedWidth{Math::Clamp(static_cast<uint32>(GRenderWindow.GetPixelWidth()), SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width)};
    const uint32 ClampedHeight{Math::Clamp(static_cast<uint32>(GRenderWindow.GetPixelWidth()), SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height)};

    return Vk::Extent2D{ClampedWidth, ClampedHeight};
}

Vk::SurfaceFormatKHR FRenderSwapChain::ChooseSurfaceFormat(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats)
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

Vk::PresentModeKHR FRenderSwapChain::ChoosePresentationMode(const TDynamicArray<Vk::PresentModeKHR>& PresentModes)
{
    Vk::PresentModeKHR* FoundMode{nullptr};

    auto FindMode = [&PresentModes, &FoundMode](const Vk::PresentModeKHR ModeToFind) -> Vk::PresentModeKHR*
    {
        FoundMode = PresentModes.FindPointer(ModeToFind);
#if DEBUG
        if(FoundMode)
        {
            LOG(LogVulkan, "using presentation-mode: {}", Vk::to_string(*FoundMode));
        }
#endif
        return FoundMode;
    };

    if(!FindMode(Vk::PresentModeKHR::eMailbox))
    {
        if(!FindMode(Vk::PresentModeKHR::eImmediate))
        {
            FindMode(Vk::PresentModeKHR::eFifo);
        }
    }

    return *FoundMode;
}

void FRenderSwapChain::CreateSwapChain()
{
    ASSERT(GRenderDevice.GetLogicalDevice() && GRenderDevice.GetSurface());

    const FSwapChainSupportDetails& SwapChainSupportDetails{GRenderDevice.GetSwapChainSupportDetails()};

    ImageExtent = ChooseImageExtent(SwapChainSupportDetails.SurfaceCapabilities);
    SurfaceFormat = ChooseSurfaceFormat(SwapChainSupportDetails.SurfaceFormats);
    PresentMode = ChoosePresentationMode(SwapChainSupportDetails.PresentModes);

    uint32 MinImageCount{SwapChainSupportDetails.SurfaceCapabilities.minImageCount + 1};

    if(SwapChainSupportDetails.SurfaceCapabilities.maxImageCount != 0 && MinImageCount > SwapChainSupportDetails.SurfaceCapabilities.maxImageCount)
    {
        MinImageCount = SwapChainSupportDetails.SurfaceCapabilities.maxImageCount;
    }

    const FQueueFamilyIndices QueueFamilyIndices{GRenderDevice.GetQueueFamilyIndices()};
    const TStaticArray<uint32, 2> QueueFamilyIndicesArray{QueueFamilyIndices.Graphic, QueueFamilyIndices.Presentation};

    const Vk::SwapchainCreateFlagsKHR SwapChainCreateFlags{};
    const Vk::ImageUsageFlags ImageUsageFlags{Vk::ImageUsageFlagBits::eColorAttachment};

    Vk::SwapchainCreateInfoKHR SwapChainCreateInfo{};
    SwapChainCreateInfo.flags = SwapChainCreateFlags;
    SwapChainCreateInfo.surface = GRenderDevice.GetSurface();
    SwapChainCreateInfo.minImageCount = MinImageCount;
    SwapChainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    SwapChainCreateInfo.imageFormat = SurfaceFormat.format;
    SwapChainCreateInfo.imageExtent = ImageExtent;
    SwapChainCreateInfo.imageArrayLayers = 1;
    SwapChainCreateInfo.imageUsage = ImageUsageFlags;
    SwapChainCreateInfo.preTransform = SwapChainSupportDetails.SurfaceCapabilities.currentTransform;
    SwapChainCreateInfo.compositeAlpha = Vk::CompositeAlphaFlagBitsKHR::eOpaque;
    SwapChainCreateInfo.presentMode = PresentMode;
    SwapChainCreateInfo.clipped = true;
    SwapChainCreateInfo.oldSwapchain = OldSwapChain.IsValid() ? OldSwapChain->SwapChain : NULL_HANDLE;
    SwapChainCreateInfo.imageSharingMode = Vk::SharingMode::eExclusive;
    SwapChainCreateInfo.queueFamilyIndexCount = QueueFamilyIndicesArray.Num() * (QueueFamilyIndicesArray[0] != QueueFamilyIndicesArray[1]); //0 if queue indices match
    SwapChainCreateInfo.pQueueFamilyIndices = QueueFamilyIndicesArray.GetData();

    SwapChain = GRenderDevice.GetLogicalDevice().createSwapchainKHR(SwapChainCreateInfo);
    ASSERT(SwapChain);
    LOG(LogVulkan, "created swapchain");

    Images = FindSwapChainImages();
}

void FRenderSwapChain::CreateImageViews()
{
    const Vk::Device LogicalDevice{GRenderDevice.GetLogicalDevice()};
    ASSERT(LogicalDevice);

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
}

void FRenderSwapChain::CreateDepthResources()
{

}

void FRenderSwapChain::CreateRenderPass()
{
    Vk::AttachmentDescription ColorAttachment{};
    ColorAttachment.format = SurfaceFormat.format;
    ColorAttachment.loadOp = Vk::AttachmentLoadOp::eClear;
    ColorAttachment.storeOp = Vk::AttachmentStoreOp::eStore;
    ColorAttachment.stencilLoadOp = Vk::AttachmentLoadOp::eDontCare;
    ColorAttachment.stencilStoreOp = Vk::AttachmentStoreOp::eDontCare;
    ColorAttachment.initialLayout = Vk::ImageLayout::eUndefined;
    ColorAttachment.finalLayout = Vk::ImageLayout::ePresentSrcKHR;

    Vk::AttachmentReference AttachmentReference{0, Vk::ImageLayout::eColorAttachmentOptimal};

    Vk::SubpassDescription SubPass{};
    SubPass.pipelineBindPoint = Vk::PipelineBindPoint::eGraphics;
    SubPass.colorAttachmentCount = 1;
    SubPass.pColorAttachments = &AttachmentReference;
    SubPass.inputAttachmentCount = 0;
    SubPass.pInputAttachments = nullptr;
    SubPass.pDepthStencilAttachment = nullptr;
    SubPass.preserveAttachmentCount = 0;
    SubPass.pPreserveAttachments = nullptr;

    Vk::RenderPassCreateInfo RenderPassInfo{};
    RenderPassInfo.attachmentCount = 1;
    RenderPassInfo.pAttachments = &ColorAttachment;
    RenderPassInfo.subpassCount = 1;
    RenderPassInfo.pSubpasses = &SubPass;

    RenderPass = GRenderDevice.GetLogicalDevice().createRenderPass(RenderPassInfo);

    ASSERT(RenderPass);
    LOG(LogVulkan, "created render-pass");
}

void FRenderSwapChain::CreateFrameBuffers()
{
    const Vk::Device LogicalDevice{GRenderDevice.GetLogicalDevice()};
    ASSERT(LogicalDevice);

    FrameBuffers.ResizeTo(ImageViews.Num());

    Vk::FramebufferCreateInfo FramebufferInfo{};
    FramebufferInfo.renderPass = RenderPass;
    FramebufferInfo.attachmentCount = 1;
    FramebufferInfo.width = ImageExtent.width;
    FramebufferInfo.height = ImageExtent.height;
    FramebufferInfo.layers = 1;

    for(int64 Index{0}; Index < ImageViews.Num(); ++Index)
    {
        FramebufferInfo.pAttachments = &ImageViews[Index];

        FrameBuffers[Index] = LogicalDevice.createFramebuffer(FramebufferInfo);
        ASSERT(FrameBuffers[Index]);
    }

    LOG(LogVulkan, "created frame-buffers");
}

void FRenderSwapChain::CreateCommandBuffers()
{
    CommandBuffers.ResizeTo(FrameBuffers.Num());

    Vk::CommandBufferAllocateInfo AllocateInfo{};
    AllocateInfo.commandPool = GRenderDevice.GetCommandPool();
    AllocateInfo.commandBufferCount = CommandBuffers.Num();
    AllocateInfo.level = Vk::CommandBufferLevel::ePrimary;

    const Vk::Result Result{GRenderDevice.GetLogicalDevice().allocateCommandBuffers(&AllocateInfo, CommandBuffers.GetData())};

    ASSERT(Result == Vk::Result::eSuccess, "could not allocate command-buffers {}", Vk::to_string(Result));
    LOG(LogVulkan, "allocated command-buffers");
}

void FRenderSwapChain::CreateSyncObjects()
{

}

