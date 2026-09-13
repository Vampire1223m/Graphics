#pragma once

#include "core.h"
#include <vulkan/vulkan.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat imageFormat;
    VkExtent2D extent;
    uint32_t imageCount;
    VkImage* images;
    VkImageView* imageViews;
} VulkanDisplay;

bool initVulkanSurface(VulkanCore* core, VulkanDisplay* display);
bool initVulkanSwapchain(VulkanCore* core, VulkanDisplay* display);
void cleanupVulkanDisplay(VulkanCore* core, VulkanDisplay* display);