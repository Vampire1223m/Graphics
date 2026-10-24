#include "renderer.h"
#include "mat.h"
#include "vertices.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool findMemoryType( VkPhysicalDevice physicalDevice, uint32_t allowedTypeBits, VkMemoryPropertyFlags requiredProperties, uint32_t *memoryTypeIndex ){

    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(
                physicalDevice,
                &memoryProperties
            );

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((allowedTypeBits & (1u << i)) && 
            ((memoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties)){

            *memoryTypeIndex = i;
            return true;

        }
        
    }

    return false;
   
}

static void destroyRendererResources(VulkanCore *core, VulkanRenderer *renderer);
static void destroyVulkanBuffer(VkDevice device, VulkanBuffer *buffer);
static bool createVulkanBuffer(
    VulkanCore *core,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    bool persistentMap,
    VulkanBuffer *buffer
);

bool initVulkanRenderer(VulkanCore* core, VulkanDisplay* display, VulkanRenderer* renderer, VulkanPipeline* pipeline) {

    VkDevice device = core->device;

    VkSemaphoreCreateInfo semaphoreInfo = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
			};

	VkResult s = vkCreateSemaphore(
			device,
			&semaphoreInfo,
			NULL,
			&renderer->imageAvailable
		);
	if ( s != VK_SUCCESS){

		fprintf(stderr, "failed to create image available Semaphore: %d\n", s);
		goto fail;

	}

    renderer->renderFinishedCount = display->imageCount;

    if (display->imageCount == 0){
        fprintf(stderr, "cant create semaphores, 0 swapchain images\n");
        goto fail;
    }

    renderer->renderFinished = calloc(renderer->renderFinishedCount, sizeof(*renderer->renderFinished));
    
    if (renderer->renderFinished == NULL) {
        fprintf(stderr, "Failed to allocate render-finished semaphore array\n");
        renderer->renderFinishedCount = 0;
        goto fail;
    }

    for (uint32_t i = 0; i < renderer->renderFinishedCount; ++i)
    {
        s = vkCreateSemaphore(
                device,
                &semaphoreInfo,
                NULL,
                &renderer->renderFinished[i]
            );
        if ( s != VK_SUCCESS){

            fprintf(stderr, "failed to create render finished Semaphore: %d\n", s);
            goto fail;

        }
    }
    

    VkFenceCreateInfo fenceInfo = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

	s = vkCreateFence(
			device,
			&fenceInfo,
			NULL,
			&renderer->inFlightFence
		);
	if ( s != VK_SUCCESS){

		fprintf(stderr, "failed to create fence: %d\n", s);
        goto fail;

	}

    if (!createVulkanBuffer(core, 
                            sizeof(Vertex)*verticesCount, 
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                            true, 
                            &renderer->vertexBuffer)) {
    
        fprintf(stderr, "failed to create vertex buffer");
        goto fail;
    }
    

    memcpy(
        renderer->vertexBuffer.mapped,
        vertices,
        renderer->vertexBuffer.size
    );

    memcpy(
        frameVertices,
        vertices,
        renderer->vertexBuffer.size
    );

    if (!createVulkanBuffer(core, 
                            sizeof(uint32_t)*indicesCount, 
                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                            true, 
                            &renderer->indexBuffer)) {

        fprintf(stderr, "failed to create index buffer");
        goto fail;
    }

    memcpy(
        renderer->indexBuffer.mapped,
        indices,
        renderer->indexBuffer.size
    );

    renderer->indexCount = indicesCount;

    if (!createVulkanBuffer(core, 
                            sizeof(uniformBufferObj), 
                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                            true, 
                            &renderer->uniformBuffer)) {

        fprintf(stderr, "failed to create uniform buffer");
        goto fail;
    }

    memcpy(
        renderer->uniformBuffer.mapped,
        &ubo,
        renderer->uniformBuffer.size
    );

    VkDescriptorPoolSize dPoolSize = {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1
            };

    VkDescriptorPoolCreateInfo dPoolInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .poolSizeCount = 1,
                .pPoolSizes = &dPoolSize,
                .maxSets = 1
            };

    s = vkCreateDescriptorPool(
            device,
            &dPoolInfo,
            NULL,
            &renderer->descriptorPool
        );
    if (s != VK_SUCCESS) {

        fprintf(stderr, "Failed to create descriptor pool: %d\n", s);
        goto fail;

    }

    VkDescriptorSetAllocateInfo dAllocInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = renderer->descriptorPool,
                .descriptorSetCount = 1,
                .pSetLayouts = &pipeline->descriptorSetLayout
            };

    s = vkAllocateDescriptorSets(
            device,
            &dAllocInfo,
            &renderer->descriptorSet
        );
    if (s != VK_SUCCESS) {

        fprintf(stderr, "Failed to allocate descriptor set: %d\n", s);
        goto fail;
        
    }

    VkDescriptorBufferInfo dBufferInfo = {
                .buffer = renderer->uniformBuffer.buffer,
                .offset = 0,
                .range = sizeof(uniformBufferObj)
            };
    
    VkWriteDescriptorSet descriptorWrite = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = renderer->descriptorSet,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &dBufferInfo
            };

    vkUpdateDescriptorSets(
            device,
            1,
            &descriptorWrite,
            0,
            NULL
        );

    VkCommandPoolCreateInfo poolInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = core->graphicsFamily
            };

    s = vkCreateCommandPool(
            device,
            &poolInfo,
            NULL,
            &renderer->cmdPool
        );
    
    if ( s != VK_SUCCESS){

        fprintf(stderr, "failed to create command");
        goto fail;

    }

    VkCommandBufferAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = renderer->cmdPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };

    s = vkAllocateCommandBuffers(
            device,
            &allocInfo,
            &renderer->cmdBuffer
        );

    if ( s != VK_SUCCESS){

        fprintf(stderr, "failed to allocate command buffer");
        goto fail;

    }

    return true;

    fail:
    destroyRendererResources(core, renderer);
    return false;

}

bool drawFrame(VulkanCore* core, VulkanDisplay* display, VulkanPipeline* pipeline, VulkanRenderer* renderer) {
    VkDevice device = core->device;
    VkCommandBuffer cmdBuffer = renderer->cmdBuffer;

    vkWaitForFences(
            device,
            1,
            &renderer->inFlightFence,
            VK_TRUE,
            UINT64_MAX
        );

    uint32_t imageIndex;

    VkResult s = vkAcquireNextImageKHR(
            device,
            display->swapchain,
            UINT64_MAX,
            renderer->imageAvailable,
            VK_NULL_HANDLE,
            &imageIndex
        );

    if ( s != VK_SUCCESS){

        fprintf(stderr, "failed to acquire sc image: %d", s);
        return false;

    }

    // printf("acquired image: %u\n", imageIndex);

    // frameVertices[0].position[0] += 0.01f

    memcpy(
        renderer->vertexBuffer.mapped,
        frameVertices,
        renderer->vertexBuffer.size
    );

    float transformR[16];
    float transformS[16];
    mat4Identity(transformR);
    mat4Identity(transformS);
    mat4Rotate(transformR, 67.0f);
    mat4Scale(transformS, 0.5f, 0.5f);
    mat4Multiply(ubo.transform, transformR, transformS);

    memcpy(
        renderer->uniformBuffer.mapped,
        &ubo,
        renderer->uniformBuffer.size
    );

    vkResetCommandBuffer(cmdBuffer, 0);

    VkCommandBufferBeginInfo beginInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
            };

    s = vkBeginCommandBuffer(cmdBuffer,&beginInfo);

    if ( s != VK_SUCCESS){

        fprintf(stderr, "failed to begin command buffer");
        return false;

    }

    //
    // cmd start
    //

    VkImageMemoryBarrier barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = display->images[imageIndex],
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                    }
            };

    vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            0,
            NULL,
            0,
            NULL,
            1,
            &barrier
        );

    VkRenderingAttachmentInfo colorAttachment = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = display->imageViews[imageIndex],
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                        .color = {
                            .float32 = {0.0f, 0.0f, 0.0f, 1.0f}
                        }
                    }
            };

    VkRenderingInfo renderingInfo = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea = {
                        .offset = {0, 0},
                        .extent = display->extent
                    },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachment
            };

    vkCmdBeginRendering(cmdBuffer, &renderingInfo);

    //
    // Cmd Rendering Start
    //

    vkCmdBindPipeline(
            cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->graphicsPipeline
        );

    vkCmdBindDescriptorSets(
            cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->pipelineLayout,
            0,
            1,
            &renderer->descriptorSet,
            0,
            NULL
        );

    VkDeviceSize vertexOffset = 0;

    vkCmdBindVertexBuffers(
        cmdBuffer,
        0,
        1,
        &renderer->vertexBuffer.buffer,
        &vertexOffset
    );

    vkCmdBindIndexBuffer(
        renderer->cmdBuffer,
        renderer->indexBuffer.buffer,
        0,
        VK_INDEX_TYPE_UINT32
    );

    VkViewport viewport = {
            .x = 0.0f, .y = 0.0f,
            .width = (float)display->extent.width,
            .height = (float)display->extent.height,
            .minDepth = 0.0f, .maxDepth = 1.0f
        };
    
    VkRect2D scissor = {
            .offset = {0, 0},
            .extent = display->extent
        };

    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    vkCmdDrawIndexed(cmdBuffer, renderer->indexCount, 1, 0, 0, 0);

    //
    // Cmd Rendering End
    //

    vkCmdEndRendering(cmdBuffer);

    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0,
            NULL,
            0,
            NULL,
            1,
            &barrier
        );

    //
    // cmd end
    //

    s = vkEndCommandBuffer(cmdBuffer);

    if ( s != VK_SUCCESS){

        fprintf(stderr, "failed to end command buffer");
        return false;

    }

    s = vkResetFences(
            device,
            1,
            &renderer->inFlightFence
        );
    if ( s != VK_SUCCESS){

        fprintf(stderr, "failed to reset fence buffer");
        return false;

    }

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &renderer->imageAvailable,
                .pWaitDstStageMask = &waitStage,

                .commandBufferCount = 1,
                .pCommandBuffers = &cmdBuffer,

                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &renderer->renderFinished[imageIndex]
            };

    s = vkQueueSubmit(
            core->graphicsQueue,
            1,
            &submitInfo,
            renderer->inFlightFence
        );

    if ( s != VK_SUCCESS){

        fprintf(stderr, "failed to submit command buffer");
        return false;

    }

    VkPresentInfoKHR presentInfo = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &renderer->renderFinished[imageIndex],
                .swapchainCount = 1,
                .pSwapchains = &display->swapchain,
                .pImageIndices = &imageIndex
            };

    s = vkQueuePresentKHR(core->graphicsQueue, &presentInfo);

    if ( s != VK_SUCCESS){

        fprintf(stderr, "failed to present: %d", s);
        return false;

    }

    return true;

}

static void destroyVulkanBuffer(VkDevice device, VulkanBuffer *buffer){

    if (buffer->buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, buffer->buffer, NULL);
        buffer->buffer = VK_NULL_HANDLE;
    }

    if (buffer->mapped != NULL) {
        vkUnmapMemory(device, buffer->memory);
        buffer->mapped = NULL;
    }

    if (buffer->memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, buffer->memory, NULL);
        buffer->memory = VK_NULL_HANDLE;
    }

    buffer->size = 0;

}

static bool createVulkanBuffer(
    VulkanCore *core,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    bool persistentMap,
    VulkanBuffer *buffer
){

    *buffer = (VulkanBuffer){0};
    buffer->size = size;

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkResult s = vkCreateBuffer(
        core->device,
        &bufferInfo,
        NULL,
        &buffer->buffer
    );

    if (s != VK_SUCCESS) {
        fprintf(stderr, "Failed to create buffer: %d\n", s);
        return false;
    }

    VkMemoryRequirements requirements;

    vkGetBufferMemoryRequirements(
        core->device,
        buffer->buffer,
        &requirements
    );

    uint32_t memoryTypeIndex;

    if (!findMemoryType(
            core->physicalDevice,
            requirements.memoryTypeBits,
            properties,
            &memoryTypeIndex)) {

        fprintf(stderr, "No compatible memory type found\n");
        destroyVulkanBuffer(core->device, buffer);
        return false;
    }

    VkMemoryAllocateInfo allocationInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    s = vkAllocateMemory(
        core->device,
        &allocationInfo,
        NULL,
        &buffer->memory
    );

    if (s != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate buffer memory: %d\n", s);
        destroyVulkanBuffer(core->device, buffer);
        return false;
    }

    s = vkBindBufferMemory(
        core->device,
        buffer->buffer,
        buffer->memory,
        0
    );

    if (s != VK_SUCCESS) {
        fprintf(stderr, "Failed to bind buffer memory: %d\n", s);
        destroyVulkanBuffer(core->device, buffer);
        return false;
    }

    if (persistentMap) {
        s = vkMapMemory(
            core->device,
            buffer->memory,
            0,
            size,
            0,
            &buffer->mapped
        );

        if (s != VK_SUCCESS) {
            fprintf(stderr, "Failed to map buffer memory: %d\n", s);
            destroyVulkanBuffer(core->device, buffer);
            return false;
        }
    }

    return true;

}

static void destroyRendererResources(VulkanCore* core, VulkanRenderer* renderer) {

    VkDevice device = core->device;

    if (renderer->imageAvailable != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, renderer->imageAvailable, NULL);
        renderer->imageAvailable = VK_NULL_HANDLE;
    }
    if (renderer->renderFinished != NULL) {
        for (uint32_t i = 0; i < renderer->renderFinishedCount; ++i) {

            if (renderer->renderFinished[i] != VK_NULL_HANDLE) {

                vkDestroySemaphore(
                    device,
                    renderer->renderFinished[i],
                    NULL
                );

            }

        }

        free(renderer->renderFinished);
        renderer->renderFinished = NULL;
        renderer->renderFinishedCount = 0;
        
    }
    if (renderer->inFlightFence != VK_NULL_HANDLE) {
        vkDestroyFence(device, renderer->inFlightFence, NULL);
        renderer->inFlightFence = VK_NULL_HANDLE;
    }

    if (renderer->descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, renderer->descriptorPool, NULL);
        renderer->descriptorPool = VK_NULL_HANDLE;
        renderer->descriptorSet = VK_NULL_HANDLE;
    }

    renderer->indexCount = 0;
    
    destroyVulkanBuffer(device, &renderer->vertexBuffer);
    destroyVulkanBuffer(device, &renderer->indexBuffer);
    destroyVulkanBuffer(device, &renderer->uniformBuffer);

    if (renderer->cmdPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, renderer->cmdPool, NULL);
        renderer->cmdPool = VK_NULL_HANDLE;
        renderer->cmdBuffer = VK_NULL_HANDLE;
    }

}

void cleanupVulkanRenderer(VulkanCore* core, VulkanRenderer* renderer) {
    
    vkDeviceWaitIdle(core->device);

    destroyRendererResources(core, renderer);

}
