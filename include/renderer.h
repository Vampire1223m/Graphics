#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include "core.h"
#include "display.h"
#include "pipeline.h"

typedef struct {
    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffer;
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence inFlightFence;
} VulkanRenderer;

bool initVulkanRenderer(VulkanCore* core, VulkanRenderer* renderer);

bool drawFrame(VulkanCore* core, VulkanDisplay* display, VulkanPipeline* pipeline, VulkanRenderer* renderer);

void cleanupVulkanRenderer(VulkanCore* core, VulkanRenderer* renderer);

#endif