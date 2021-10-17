#include "RenderSwapChain.hpp"
#include "RenderWindow.hpp"
#include "RenderDevice.hpp"
#include "CoreFiles/Log.hpp"
#include "CoreFiles/Math.hpp"

inline bool operator!(const Vk::Result VulkanResult)
{
    return VulkanResult != Vk::Result::eSuccess;
}
/*
FRenderSwapChainManager::FRenderSwapChainManager()
    : SwapChainHandle{NULL_HANDLE}
{
}

FRenderSwapChainManager::~FRenderSwapChainManager()
{
    ASSERT(!SwapChainHandle, "swap chain was not destroyed");
}

void FRenderSwapChainManager::PopulateImages(Vk::Device LogicalDevice)
{
    uint32 ImageCount{0};
    Vk::Result Result{LogicalDevice.getSwapchainImagesKHR(SwapChainHandle, &ImageCount, nullptr)};
    ASSERT(!!Result, "failed to get swapchain images {}", Vk::to_string(Result));

    if(ImageCount > 0)
    {
        Images.ResizeTo(ImageCount);

        Result = LogicalDevice.getSwapchainImagesKHR(SwapChainHandle, &ImageCount, Images.Data());
        ASSERT(!!Result, "failed to get swapchain images {}", Vk::to_string(Result));
    }

    LOG(LogVulkan, "populated swapchain images");
}

void FRenderSwapChainManager::DestroyImages(Vk::Device LogicalDevice)
{
    if(!LogicalDevice)
    {
        LOGW(LogVulkan, "could not destroy images [logical device is invalid]");
        return;
    }

    for(Vk::Image& Image: Images)
    {
        if(Image)
        {
            LogicalDevice.destroyImage(Image);
            Image = NULL_HANDLE;
        }
    }

    LOGW(LogVulkan, "destroyed images");
}

void FRenderSwapChainManager::CreateImageViews(Vk::Device LogicalDevice)
{
    ImageViews.ResizeTo(Images.Num());

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

    for(int64 Index{0}; Index < Images.Num(); ++Index)
    {
        ImageViewCreateInfo.image = Images[Index];
        ImageViews[Index] = LogicalDevice.createImageView(ImageViewCreateInfo);

        ASSERT(ImageViews[Index]);
    }

    LOG(LogVulkan, "created image views");
}

void FRenderSwapChainManager::DestroyImageViews(Vk::Device LogicalDevice)
{
    if(!LogicalDevice)
    {
        LOGW(LogVulkan, "could not destroy image views [logical device is invalid]");
        return;
    }

    for(Vk::ImageView& ImageView : ImageViews)
    {
        if(ImageView)
        {
            LogicalDevice.destroyImageView(ImageView);
            ImageView = NULL_HANDLE;
        }
    }

    LOG(LogVulkan, "destroyed image-views");
}

Vk::Extent2D FRenderSwapChainManager::ChooseImageExtent(const Vk::SurfaceCapabilitiesKHR& SurfaceCapabilities, FWindowDimensions Dimensions)
{
    if(SurfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        return SurfaceCapabilities.currentExtent;
    }

    const uint32 ClampedWidth{Math::Clamp(static_cast<uint32>(Dimensions.PixelWidth), SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width)};
    const uint32 ClampedHeight{Math::Clamp(static_cast<uint32>(Dimensions.PixelHeight), SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height)};

    return Vk::Extent2D{ClampedWidth, ClampedHeight};
}

Vk::SurfaceFormatKHR FRenderSwapChainManager::ChooseSurfaceFormat(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats)
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

Vk::PresentModeKHR FRenderSwapChainManager::ChoosePresentationMode(const TDynamicArray<Vk::PresentModeKHR>& PresentModes)
{
    uint32 MailboxFound{0};
    uint32 ImmediateFound{0};

    for(const Vk::PresentModeKHR Mode : PresentModes)
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
    else
    {
        LOG(LogVulkan, "using presentation-mode: {}", Vk::to_string(Vk::PresentModeKHR::eFifo));
        return Vk::PresentModeKHR::eFifo; //is guaranteed to exist
    }
}

void FRenderSwapChainManager::PopulateSwapChainData(const FSwapChainSupportDetails& SwapChainSupportDetails, FWindowDimensions WindowDimensions)
{
    SwapChainData.ImageExtent = ChooseImageExtent(SwapChainSupportDetails.SurfaceCapabilities, WindowDimensions);
    SwapChainData.SurfaceFormat = ChooseSurfaceFormat(SwapChainSupportDetails.SurfaceFormats);
    SwapChainData.PresentMode = ChoosePresentationMode(SwapChainSupportDetails.PresentModes);
}

void FRenderSwapChainManager::CreateSwapChain(Vk::Device LogicalDevice, Vk::SurfaceKHR Surface, const FSwapChainSupportDetails& SwapChainSupportDetails, const FQueueFamilyIndices& QueueIndices, Vk::SwapchainKHR OldSwapChain)
{
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
    SwapChainCreateInfo.surface = Surface;
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


    SwapChainHandle = LogicalDevice.createSwapchainKHR(SwapChainCreateInfo);

    ASSERT(SwapChainHandle);
    LOG(LogVulkan, "created swapchain");
}

void FRenderSwapChainManager::DestroySwapChain(Vk::Device LogicalDevice)
{
    if(!LogicalDevice)
    {
        LOGW(LogVulkan, "could not destroy swap chain [logical device is invalid]");
        return;
    }

    if(!SwapChainHandle)
    {
        LOGW(LogVulkan, "could not destroy swap chain [swap chain is already invalid]");
        return;
    }

    LogicalDevice.destroySwapchainKHR(SwapChainHandle);
    SwapChainHandle = NULL_HANDLE;
}



FRenderSwapChainManager::FRenderSwapChainManager(TSharedPtr<FRenderSwapChainManager> InOldSwapChain)
    : OldSwapChain{Move(InOldSwapChain)}
    , SwapChain{NULL_HANDLE}
    , RenderPass{NULL_HANDLE}
{
    if(OldSwapChain)
    {
        OldSwapChain->OldSwapChain.Reset();
    }
}

void FRenderSwapChainManager::Initialize()
{
    CreateSwapChain();

    CreateImageViews();

    CreateRenderPass();

    CreateDepthResources();

    CreateFrameBuffers();

    CreateSyncObjects();
}

void FRenderSwapChainManager::ShutDown()
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

FRenderSwapChainManager::~FRenderSwapChainManager()
{
}

Vk::SwapchainKHR FRenderSwapChainManager::GetSwapChain() const
{
    return SwapChain;
}

Vk::RenderPass FRenderSwapChainManager::GetRenderPass() const
{
    return RenderPass;
}

Vk::Extent2D FRenderSwapChainManager::GetImageExtent() const
{
    return ImageExtent;
}

TDynamicArray<Vk::Image> FRenderSwapChainManager::FindSwapChainImages() const
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

Vk::Extent2D FRenderSwapChainManager::ChooseImageExtent(const Vk::SurfaceCapabilitiesKHR SurfaceCapabilities)
{
    if(SurfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        return SurfaceCapabilities.currentExtent;
    }

    const uint32 ClampedWidth{Math::Clamp(static_cast<uint32>(GRenderWindow.GetPixelWidth()), SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width)};
    const uint32 ClampedHeight{Math::Clamp(static_cast<uint32>(GRenderWindow.GetPixelWidth()), SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height)};

    return Vk::Extent2D{ClampedWidth, ClampedHeight};
}

Vk::SurfaceFormatKHR FRenderSwapChainManager::ChooseSurfaceFormat(const TDynamicArray<Vk::SurfaceFormatKHR>& SurfaceFormats)
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

Vk::PresentModeKHR FRenderSwapChainManager::ChoosePresentationMode(const TDynamicArray<Vk::PresentModeKHR>& PresentModes)
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

void FRenderSwapChainManager::CreateSwapChain()
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

void FRenderSwapChainManager::CreateImageViews()
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

void FRenderSwapChainManager::CreateDepthResources()
{

}

void FRenderSwapChainManager::CreateRenderPass()
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

void FRenderSwapChainManager::CreateFrameBuffers()
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

void FRenderSwapChainManager::CreateCommandBuffers()
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

void FRenderSwapChainManager::CreateSyncObjects()
{

}

*/
