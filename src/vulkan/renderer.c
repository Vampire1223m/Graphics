#include "renderer.h"
#include "vertices.h"
#include <stdio.h>
#include <stdlib.h>

bool initVulkanRenderer(VulkanCore* core, VulkanRenderer* renderer) {

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

		printf("failed to create image available Semaphore: %d\n", s);
		return false;

	}

	s = vkCreateSemaphore(
			device,
			&semaphoreInfo,
			NULL,
			&renderer->renderFinished
		);
	if ( s != VK_SUCCESS){

		printf("failed to create render finished Semaphore: %d\n", s);
		return false;

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

		printf("failed to create fence: %d\n", s);
		return false;

	}

    VkBufferCreateInfo bufferInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = sizeof(vertices),
                .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE
            };
    s = vkCreateBuffer(
            device,
            &bufferInfo,
            NULL,
            &renderer->vertexBuffer
        );
    if ( s != VK_SUCCESS){

		printf("failed to vertex Buffer: %d\n", s);
		return false;

	}

    VkMemoryRequirements memRequirements;

    vkGetBufferMemoryRequirements(
                device,
                renderer->vertexBuffer,
                &memRequirements
            );

    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(
                Pdevice,
                &memoryProperties
            );

    uint32_t memoryTypeIndex;

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((memRequirements.memoryTypeBits & (1 << i)) && 
            (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && 
            (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)))
        {
            memoryTypeIndex = i;
            break;
        }
        
    }
    
    VkMemoryAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memRequirements.size,
                .memoryTypeIndex = memoryTypeIndex
            };

    s = vkAllocateMemory(
            device,
            &allocInfo,
            NULL,
            &renderer->vertexMemory
        );
    if ( s != VK_SUCCESS){

		printf("failed to allocate vertex memory: %d\n", s);
		return false;

	}

    s = vkBindBufferMemory(
        device,
        renderer->vertexBuffer,
        renderer->vertexMemory,
        0
    );
    if ( s != VK_SUCCESS){

		printf("failed to bind vertex memory: %d\n", s);
		return false;

	}

    void *data;

    s = vkMapMemory(
        device,
        renderer->vertexMemory,
        0,
        sizeof(vertices),
        0,
        &data
    );
    if ( s != VK_SUCCESS){

		printf("failed to map vertex memory: %d\n", s);
		return false;

	}

    memcpy(
        data,
        triangleVertices,
        sizeof(triangleVertices)
    );

    vkUnmapMemory(device, vertexMemory);

    VkCommandPoolCreateInfo poolInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = core->graphicsFamily
            };

    s = vkCreateCommandPool(
            device,
            &poolInfo,
            NULL,
            &renderer->cmdPool
        );
    
    if ( s != VK_SUCCESS){

        printf("failed to create command");
        return false;

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

        printf("failed to allocate command buffer");
        return false;

    }

    return true;

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
    vkResetFences(
            device,
            1,
            &renderer->inFlightFence
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

        printf("failed to acquire sc image: %d", s);
        return false;

    }

    vkResetCommandBuffer(cmdBuffer, 0);

    // printf("acquired image: %u\n", imageIndex);

    VkCommandBufferBeginInfo beginInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
            };

    s = vkBeginCommandBuffer(cmdBuffer,&beginInfo);

    if ( s != VK_SUCCESS){

        printf("failed to begin command buffer");
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

    vkCmdDraw(cmdBuffer, 3, 1, 0, 0);

    VkDeviceSize vertexOffset = 0;

    vkCmdBindVertexBuffers(
        cmdBuffer,
        0,
        1,
        &renderer->vertexBuffer,
        &vertexOffset
    );

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

        printf("failed to end command buffer");
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
                .pSignalSemaphores = &renderer->renderFinished
            };

    s = vkQueueSubmit(
            core->graphicsQueue,
            1,
            &submitInfo,
            renderer->inFlightFence
        );

    if ( s != VK_SUCCESS){

        printf("failed to submit command buffer");
        return false;

    }

    VkPresentInfoKHR presentInfo = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &renderer->renderFinished,
                .swapchainCount = 1,
                .pSwapchains = &display->swapchain,
                .pImageIndices = &imageIndex
            };

    s = vkQueuePresentKHR(core->graphicsQueue, &presentInfo);

    if ( s != VK_SUCCESS){

        printf("failed to present: %d", s);
        return false;

    }

    return true;

}

void cleanupVulkanRenderer(VulkanCore* core, VulkanRenderer* renderer) {
    VkDevice device = core->device;
    
    // Waiting for device to finish operations before destroying sync objects
    vkDeviceWaitIdle(device);

    if (renderer->imageAvailable) vkDestroySemaphore(device, renderer->imageAvailable, NULL);
    if (renderer->renderFinished) vkDestroySemaphore(device, renderer->renderFinished, NULL);
    if (renderer->inFlightFence) vkDestroyFence(device, renderer->inFlightFence, NULL);

    if (renderer->vertexBuffer) vkDestroyBuffer(device, renderer->vertexBuffer, NULL);
    if (renderer->vertexMemory) vkFreeMemory(device, renderer->vertexMemory, NULL);
    
    if (renderer->cmdPool) vkDestroyCommandPool(device, renderer->cmdPool, NULL);
}
