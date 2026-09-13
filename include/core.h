#pragma once

#include <vulkan/vulkan.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t graphicsFamily;
    VkQueue graphicsQueue;
} VulkanCore;

bool initVulkanInstance(VulkanCore* core);
bool initVulkanDevice(VulkanCore* core, VkSurfaceKHR surface);
void cleanupVulkanCore(VulkanCore* core);