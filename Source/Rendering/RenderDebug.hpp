#pragma once
#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/Array.hpp"

#include <vulkan/vulkan.hpp>
/*
class FRenderDebugManager final
{
public:

    FRenderDebugManager();
    ~FRenderDebugManager();

    static VKAPI_ATTR uint32 VKAPI_CALL VulkanDebugCallback(Vk::DebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, Vk::DebugUtilsMessageTypeFlagsEXT MessageType, const Vk::DebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData);

    static Vk::DebugUtilsMessengerCreateInfoEXT MakeDebugUtilsMessengerCreateInfo();

    void CreateMessenger(Vk::Instance VulkanInstance);
    void DestroyMessenger(Vk::Instance VulkanInstance);

    void PopulateValidationLayers();

    Vk::DebugUtilsMessengerEXT GetDebugMessengerHandle() const {return DebugMessengerHandle;}
    const TDynamicArray<const char8*>& GetValidationLayers() const {return ValidationLayers;}

private:

    Vk::DebugUtilsMessengerEXT DebugMessengerHandle;
    TDynamicArray<const char8*> ValidationLayers;
};
*/
