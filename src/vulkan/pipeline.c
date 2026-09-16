#include "pipeline.h"
#include "vertex.h"
#include <stdio.h>
#include <stdlib.h>

static VkShaderModule createShaderModule(VkDevice device, const char *filename){

	FILE *file = fopen(filename, "rb");

	if (!file){

		printf("Failed to open shader");
		exit(1);

	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	uint32_t *code = malloc(fileSize);

	if (!code){

		fclose(file);
		exit(1);

	}

	fread(code, 1, fileSize, file);
	fclose(file);

	VkShaderModuleCreateInfo createInfo = {
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = fileSize,
				.pCode = code
			};

	VkShaderModule shaderModule;

	VkResult s = vkCreateShaderModule(
					device,
					&createInfo,
					NULL,
					&shaderModule
				);

	if ( s != VK_SUCCESS){

		printf("FAILED to create shader module: %d", s);
		exit(1);

	}

	free(code);
	return shaderModule;

}

bool initVulkanPipeline(VulkanCore* core, VulkanDisplay* display, VulkanPipeline* pipeline) {

	VkPhysicalDevice Pdevice = core->physicalDevice;
	VkDevice device = core->device;

	VkShaderModule vertexShader = createShaderModule(device, "triangle.vert.spv");
	VkShaderModule fragmentShader = createShaderModule(device, "triangle.frag.spv");

	VkPipelineShaderStageCreateInfo vertexStage = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = vertexShader,
				.pName = "main"
			};

	VkPipelineShaderStageCreateInfo fragmentStage = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = fragmentShader,
				.pName = "main"
			};

	VkVertexInputBindingDescription vertexBinding = {
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX	
			};

	VkVertexInputAttributeDescription vertexAttribute[] = {
				{
					.location = 0,
					.binding = 0,
					.format = VK_FORMAT_R32G32_SFLOAT,
					.offset = offsetof(Vertex, position)
				},
				{
					.location = 1,
					.binding = 0,
					.format = VK_FORMAT_R32G32B32_SFLOAT,
					.offset = offsetof(Vertex, color)
				},
				{
					.location = 2,
					.binding = 0,
					.format = VK_FORMAT_R32G32B32_SFLOAT,
					.offset = offsetof(Vertex, opt)
				}
			};

	VkPipelineVertexInputStateCreateInfo vertexInput = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount = 1,
				.pVertexBindingDescriptions = &vertexBinding,
				.vertexAttributeDescriptionCount = 3,
				.pVertexAttributeDescriptions = vertexAttribute
			};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.primitiveRestartEnable = VK_FALSE
			};

	VkDynamicState dynamicStates[] = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};

	VkPipelineDynamicStateCreateInfo dynamicState = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.dynamicStateCount = 2,
				.pDynamicStates = dynamicStates
			};

	VkPipelineRasterizationStateCreateInfo rasterizer = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.depthClampEnable = VK_FALSE,
				.rasterizerDiscardEnable = VK_FALSE,
				.polygonMode = VK_POLYGON_MODE_FILL,
				.cullMode = VK_CULL_MODE_NONE,
				.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
				.depthBiasEnable = VK_FALSE
			};

	VkPipelineMultisampleStateCreateInfo multisampling = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
				.sampleShadingEnable = VK_FALSE
			};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
				.blendEnable = VK_FALSE,
				.colorWriteMask = 
						VK_COLOR_COMPONENT_R_BIT |
						VK_COLOR_COMPONENT_G_BIT |
						VK_COLOR_COMPONENT_B_BIT |
						VK_COLOR_COMPONENT_A_BIT
			};

	VkPipelineColorBlendStateCreateInfo colorBlending = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.logicOpEnable = VK_FALSE,
				.attachmentCount = 1,
				.pAttachments = &colorBlendAttachment
			};

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount = 0,
				.pSetLayouts = NULL,
				.pushConstantRangeCount = 0,
				.pPushConstantRanges = NULL
			};

	VkResult s = vkCreatePipelineLayout(
			device,
			&pipelineLayoutCreateInfo,
			NULL,
			&pipeline->pipelineLayout
		);

	if (s != VK_SUCCESS){

		printf("Failed to create pipeline layout");
		return false;

	}

	VkPipelineRenderingCreateInfo renderingInfo = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &display->imageFormat
			};

	VkPipelineShaderStageCreateInfo shaderStages[] = {
			vertexStage,
			fragmentStage
		};

	VkGraphicsPipelineCreateInfo pipelineInfo = {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = 2,
			.pStages = shaderStages,
			.pVertexInputState = &vertexInput,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &(VkPipelineViewportStateCreateInfo){
						.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
						.viewportCount = 1,
						.scissorCount = 1
					},
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicState,
			.layout = pipeline->pipelineLayout,
			.renderPass = VK_NULL_HANDLE,
			.subpass = 0,
			.pNext = &renderingInfo
		};

	s = vkCreateGraphicsPipelines(
			device,
			VK_NULL_HANDLE,
			1,
			&pipelineInfo,
			NULL,
			&pipeline->graphicsPipeline
		);

	if (s != VK_SUCCESS){

		printf("Failed to create graphics pipeline");
		return false;

	}

	vkDestroyShaderModule(device, vertexShader, NULL);
	vkDestroyShaderModule(device, fragmentShader, NULL);

	return true;

}

void cleanupVulkanPipeline(VulkanCore* core, VulkanPipeline* pipeline) {

	if (pipeline->graphicsPipeline) {
		vkDestroyPipeline(core->device, pipeline->graphicsPipeline, NULL);
	}

	if (pipeline->pipelineLayout) {
        	vkDestroyPipelineLayout(core->device, pipeline->pipelineLayout, NULL);
	}

}
