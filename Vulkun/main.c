#include <stdio.h>
#include <vulkan/vulkan.h>

int main(){

	VkInstance instance;

	uint32_t graphicsFamily = UINT32_MAX;
	uint32_t deviceIndex = UINT32_MAX;

	VkApplicationInfo appInfo = {
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName = "Test 1",
				.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
				.pEngineName = "My Engine",
				.engineVersion = VK_MAKE_VERSION(1, 0, 0),
				.apiVersion = VK_API_VERSION_1_3
			};

	const char* extensions[] = {
				VK_KHR_DISPLAY_EXTENSION_NAME
			};
	
	VkInstanceCreateInfo createInfo = {
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pApplicationInfo = &appInfo,
				.enabledExtensionCount = 1,
				.ppEnabledExtensionNames = extensions
			};

	VkResult s = vkCreateInstance(
				&createInfo,
				NULL,
				&instance
			);

	if ( s != VK_SUCCESS) {

		printf("failure to create instance: %d\n", s);
		return 1;

	}

	uint32_t deviceCount = 0;

	s = vkEnumeratePhysicalDevices(
				instance,
				&deviceCount,
				NULL
			);

	if ( s != VK_SUCCESS) {

		printf("failure to cenumerate GPUs: %d\n", s);

	}

	printf("no. of GPUs: %d\n", deviceCount);

	VkPhysicalDevice devices[deviceCount];

	vkEnumeratePhysicalDevices(
				instance,
				&deviceCount,
				devices
			);

	for (uint32_t i = 0; i < deviceCount; i++){

		VkPhysicalDeviceProperties properties;

		vkGetPhysicalDeviceProperties(
				devices[i],
				&properties
			);

		printf("GPU %u: %s\n:", i, properties.deviceName);

		VkPhysicalDeviceFeatures features;

		vkGetPhysicalDeviceFeatures(
				devices[i],
				&features
			);

		printf("API Version: %u,%u.%u\n", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));

		printf("Geo Shader: %s\n", features.geometryShader ? "Yes" : "No");

		uint32_t queueFamilyCount = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(
				devices[i],
				&queueFamilyCount,
				NULL
			);

		printf("no. of Families: %u\n", queueFamilyCount);

		VkQueueFamilyProperties queueFamilies[queueFamilyCount];

		vkGetPhysicalDeviceQueueFamilyProperties(
				devices[i],
				&queueFamilyCount,
				queueFamilies
			);


		for (uint32_t j = 0; j < queueFamilyCount; j++){

			printf("Queue family %u: %u queues, flags = 0x%x\n", j, queueFamilies[j].queueCount, queueFamilies[j].queueFlags);

			if (queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT){

				graphicsFamily = i;
				break;

			}

		}

		if (graphicsFamily == UINT32_MAX){

			printf("No graphics queue family found");
			return 1;

		}

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){

			deviceIndex = i;

		}

	}

	float queuePriority = 1.0f;

	VkDeviceQueueCreateInfo queueCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = graphicsFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			};

	VkDeviceCreateInfo deviceCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &queueCreateInfo
			};

	VkDevice device;

	s = vkCreateDevice(
			devices[deviceIndex],
			&deviceCreateInfo,
			NULL,
			&device
		);

	if (s != VK_SUCCESS){
			
		printf("Failed to create device: %d\n", s);
		return 1;
	
	}

	VkQueue graphicsQueue;

	vkGetDeviceQueue(
			device,
			graphicsFamily,
			0,
			&graphicsQueue
		);

	if (graphicsQueue != VK_NULL_HANDLE){

		printf("got graphics queue\n");

	}

	uint32_t displayCount = 0;

	s = vkGetPhysicalDeviceDisplayPropertiesKHR(
			devices[deviceIndex],
			&displayCount,
			NULL
		);

	printf("Display count: %u\n", displayCount);
	printf("s: %d\n", s);

	if (s != VK_SUCCESS || displayCount == 0){

		printf("No Display found");
		return 1;

	}

	VkDisplayPropertiesKHR displays[displayCount];

	s = vkGetPhysicalDeviceDisplayPropertiesKHR(
			devices[deviceIndex],
			&displayCount,
			displays
		);

	if ( s != VK_SUCCESS){

		printf("failed to get display properties");
		return 1;

	}

	printf("Displays: %u\n", displayCount);

	for (uint32_t i = 0; i < displayCount; i++){

		printf("Display %u: %s\n", i, displays[i].displayName);

	}

	uint32_t modeCount = 0;

	s = vkGetDisplayModePropertiesKHR(
			devices[deviceIndex],
			displays[0].display,
			&modeCount,
			NULL
		);

	if ( s != VK_SUCCESS || modeCount == 0){

		printf("No display modes");
		return 1;

	}

	VkDisplayModePropertiesKHR modes[modeCount];

	s = vkGetDisplayModePropertiesKHR(
			devices[deviceIndex],
			displays[0].display,
			&modeCount,
			modes
		);

	if ( s != VK_SUCCESS || modeCount == 0){

		printf("Failed to get display modes");
		return 1;

	}

	printf("Modes: %u\n", modeCount);

	VkDisplayModeKHR displayMode = VK_NULL_HANDLE;

	for (uint32_t i = 0; i < modeCount; i++){

		printf("Mode %u: %ux%u @ %u Hz\n", i, modes[i].parameters.visibleRegion.width, modes[i].parameters.visibleRegion.height, modes[i].parameters.refreshRate/1000);

		if ( modes[i].parameters.visibleRegion.width == 1920 && modes[i].parameters.visibleRegion.height == 1200 && modes[i].parameters.refreshRate == 165000){

			displayMode = modes[i].displayMode;
			break;

		}

	}

	if ( displayMode = VK_NULL_HANDLE){

		printf("Desired mode not found");
		return 1;

	}

	printf("selected 1920x1200 @ 165Hz");

	VkCommandPool cmdPool;

	VkCommandPoolCreateInfo poolInfo = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.queueFamilyIndex = graphicsFamily
			};

	s = vkCreateCommandPool(
			device,
			&poolInfo,
			NULL,
			&cmdPool
		);
	
	if ( s != VK_SUCCESS){

		printf("failed to create command");
		return 1;

	}

	VkCommandBuffer cmdBuffer;

	VkCommandBufferAllocateInfo allocInfo = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = cmdPool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1
			};

	s = vkAllocateCommandBuffers(
			device,
			&allocInfo,
			&cmdBuffer
		);

	if ( s != VK_SUCCESS){

		printf("failed to allocate command buffer");
		return 1;

	}

	VkCommandBufferBeginInfo beginInfo = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
			};

	s = vkBeginCommandBuffer(cmdBuffer,&beginInfo);

	if ( s != VK_SUCCESS){

		printf("failed to begin command buffer");
		return 1;

	}

	// cmds

	s = vkEndCommandBuffer(cmdBuffer);

	if ( s != VK_SUCCESS){

		printf("failed to begin command buffer");
		return 1;

	}

	VkSubmitInfo submitInfo = {
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount = 1,
				.pCommandBuffers = &cmdBuffer
			};

	s = vkQueueSubmit(
			graphicsQueue,
			1,
			&submitInfo,
			VK_NULL_HANDLE
		);

	if ( s != VK_SUCCESS){

		printf("failed to submit command buffer");
		return 1;

	}

	vkQueueWaitIdle(graphicsQueue);

	return 0;

}

