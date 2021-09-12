#pragma once

#include "Definitions.hpp"
#include "Log.hpp"
#include "SmartPointer.hpp"

#include <vulkan/vulkan.hpp>

#ifdef INCLUDE_ALL_VULKAN_FILES
#include "RenderDevice.hpp"
#include "RenderWindow.hpp"
#include "RenderPipeline.hpp"
#include "RenderSwapChain.hpp"
#else
class FRenderWindow;
class FRenderSwapChain;
class FRenderPipeline;
class FRenderDevice;
#endif

#ifndef VK_KHR_VALIDATION_LAYER_NAME
#define VK_KHR_VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"
#endif

namespace vk
{
    inline const constinit uint32 VulkanVersion{VK_MAKE_VERSION(1,2,172)};

    void InitializeVulkan();

    void ShutDownVulkan();
}

