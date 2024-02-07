#ifndef __VE_VULKAN_CONTEXT_H__
#define __VE_VULKAN_CONTEXT_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <optional>
#include "Singleton.h"
#include "ComputeBuffer.h"
#include "ComputeShader.h"


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
    std::optional<uint32_t> computeFamily;

    bool isComplete()
    {
        return computeFamily.has_value();
    }
};

class VulkanContext : public Singleton<VulkanContext>
{
public:
    VkDevice device;

public:
    void initialize();

    void reset();

    void compute();

    void release();

    inline VkCommandBuffer getCommandBuffer()
    {
        return _commandBuffer;
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    QueueFamilyIndices findQueueFamilies();

private:
    void setupDebugMessenger();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createInstance();

    bool checkValidationLayerSupport();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    std::vector<const char *> getRequiredExtensions();

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    bool isDeviceSuitable(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void createCommandBuffer();

private:
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;

    VkQueue _computeQueue;
    VkFence _computeInFlightFence;
    VkSemaphore _computeFinishedSemaphore;

    VkCommandBuffer _commandBuffer;
    VkCommandPool _commandPool;
};

#endif