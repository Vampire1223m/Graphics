#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>
#include "core.h"
#include "display.h"

typedef struct {
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkDescriptorSetLayout descriptorSetLayout;
    
} VulkanPipeline;

bool initVulkanPipeline(VulkanCore* core, VulkanDisplay* display, VulkanPipeline* pipeline);
void cleanupVulkanPipeline(VulkanCore* core, VulkanPipeline* pipeline);

#endif