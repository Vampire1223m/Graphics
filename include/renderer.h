#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include "core.h"
#include "display.h"
#include "pipeline.h"

typedef struct {
    VkCommandPool cmdPool;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkCommandBuffer cmdBuffer;

    VkSemaphore imageAvailable;
    VkSemaphore *renderFinished;
    uint32_t renderFinishedCount;

    VkFence inFlightFence;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;
    void *vertexMapped;
    VkBuffer indexBuffer;
    VkDeviceMemory indexMemory;
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformMemory;
    void *uniformMapped;
} VulkanRenderer;

bool initVulkanRenderer(VulkanCore* core, VulkanDisplay* display, VulkanRenderer* renderer, VulkanPipeline* pipeline);

bool drawFrame(VulkanCore* core, VulkanDisplay* display, VulkanPipeline* pipeline, VulkanRenderer* renderer);

void cleanupVulkanRenderer(VulkanCore* core, VulkanRenderer* renderer);

#endif