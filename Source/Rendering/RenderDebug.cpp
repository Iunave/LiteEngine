#include "RenderDebug.hpp"

using namespace Render;

FDebugManager::FDebugManager()
    : DebugMessengerHandle{NULL_HANDLE}
{
}

FDebugManager::~FDebugManager()
{
    ASSERT(!DebugMessengerHandle, "debug messenger handle was not destroyed");
}

VKAPI_ATTR uint32 VKAPI_CALL FDebugManager::VulkanDebugCallback(Vk::DebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, Vk::DebugUtilsMessageTypeFlagsEXT MessageType, const Vk::DebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
{
    switch(MessageSeverity)
    {
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
        {
            break;
        }
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
        {
            break;
        }
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
        {
            LOGW(LogVulkan, "{}", CallbackData->pMessage);
            break;
        }
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
        {
            LOGE(LogVulkan, "{}", CallbackData->pMessage);
            break;
        }
    }
    return false;
}

Vk::DebugUtilsMessengerCreateInfoEXT FDebugManager::MakeDebugUtilsMessengerCreateInfo()
{
    Vk::DebugUtilsMessengerCreateInfoEXT CreateInfo{};
    CreateInfo.messageSeverity = Vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    CreateInfo.messageType = Vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | Vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | Vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    CreateInfo.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(&VulkanDebugCallback);
    CreateInfo.pUserData = nullptr;
    return CreateInfo;
}

void FDebugManager::CreateMessenger(Vk::Instance VulkanInstance)
{
#if DEBUG
    const Vk::DebugUtilsMessengerCreateInfoEXT CreateInfo{MakeDebugUtilsMessengerCreateInfo()};

    PFN_vkCreateDebugUtilsMessengerEXT Function{reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(VulkanInstance, "vkCreateDebugUtilsMessengerEXT"))};

    if(Function == nullptr)
    {
        LOGW(LogVulkan, "failed to create debug messenger [vkGetInstanceProcAddr returned nullptr]");
        return;
    }

    Vk::Result Result{Function(VulkanInstance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&CreateInfo), nullptr, reinterpret_cast<VkDebugUtilsMessengerEXT_T**>(&DebugMessengerHandle))};

    if(!Result)
    {
        LOGW(LogVulkan, "failed to create debug messenger [{}]", Vk::to_string(Result));
        return;
    }

    LOG(LogVulkan, "created debug messenger");
#endif
}

void FDebugManager::DestroyMessenger(Vk::Instance VulkanInstance)
{
#if DEBUG
    if(!DebugMessengerHandle)
    {
        LOGW(LogVulkan, "failed to destroy debug messenger [its already invalid]");
        return;
    }

    PFN_vkDestroyDebugUtilsMessengerEXT Function{reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(VulkanInstance, "vkDestroyDebugUtilsMessengerEXT"))};

    if(Function == nullptr)
    {
        LOGW(LogVulkan, "failed to destroy debug messenger [vkGetInstanceProcAddr returned nullptr]");
        return;
    }

    Function(VulkanInstance, DebugMessengerHandle.operator VkDebugUtilsMessengerEXT_T*(), nullptr);

    LOG(LogVulkan, "destroyed debug messenger");
#endif
}

void FDebugManager::PopulateValidationLayers()
{
#if DEBUG
    ValidationLayers.Append(VK_KHR_VALIDATION_LAYER_NAME);

    auto FindAvailableValidationLayerProperties = []() -> TDynamicArray<Vk::LayerProperties>
    {
        TDynamicArray<Vk::LayerProperties> Layers{};

        uint32 LayerPropertyCount{0};
        Vk::Result Result{Vk::enumerateInstanceLayerProperties(&LayerPropertyCount, nullptr)};

        ASSERT(!!Result, "failed to enumerate layers: [{}]", Vk::to_string(Result));

        if(LayerPropertyCount > 0)
        {
            Layers.ResizeTo(LayerPropertyCount);

            Result = Vk::enumerateInstanceLayerProperties(&LayerPropertyCount, Layers.GetData());

            ASSERT(!!Result, "failed to enumerate layers: {}", Vk::to_string(Result));
        }

        return Layers;
    };

    const TDynamicArray<Vk::LayerProperties> FoundValidationLayerProperties{FindAvailableValidationLayerProperties()};

    auto IsLayerAvailable = [&FoundValidationLayerProperties, this](const int64 Index) -> bool
    {
        for(const Vk::LayerProperties& LayerProperty : FoundValidationLayerProperties)
        {
            if(Memory::StringCompare(ValidationLayers[Index], LayerProperty.layerName))
            {
                return true;
            }
        }
        return false;
    };

    for(int64 Index{0}; Index < ValidationLayers.Num(); ++Index)
    {
        if(!IsLayerAvailable(Index))
        {
            LOGW(LogVulkan, "removed validation-layer: \"{}\" as it was not available", ValidationLayers[Index]);

            ValidationLayers.RemoveAtSwap(Index);
        }
        else
        {
            LOG(LogVulkan, "using validation-layer: \"{}\"", ValidationLayers[Index]);
        }
    }
#endif
}
