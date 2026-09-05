#include <stdio.h>
#include <vulkan/vulkan.h>

int main(){

	VkInstance instance;

	uint32_t graphicsFamily = UINT32_MAX;

	VkApplicationInfo appInfo = {
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName = "Test 1",
				.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
				.pEngineName = "My Engine",
				.engineVersion = VK_MAKE_VERSION(1, 0, 0),
				.apiVersion = VK_API_VERSION_1_3
			};

	const char* extensions[] = {
				VK_KHR_DISPLAY_EXTENSION_NAME,
				VK_KHR_SURFACE_EXTENSION_NAME,
			};
	
	VkInstanceCreateInfo createInfo = {
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pApplicationInfo = &appInfo,
				.enabledExtensionCount = 2,
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
	
	VkPhysicalDevice Pdevice = VK_NULL_HANDLE;

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

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){

			Pdevice = devices[i];

		}

	}
	
	if (Pdevice == VK_NULL_HANDLE) {

		printf("No discrete GPU found\n");
		return 1;

	}

	uint32_t displayCount = 0;

	s = vkGetPhysicalDeviceDisplayPropertiesKHR(
			Pdevice,
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
			Pdevice,
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
			Pdevice,
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
			Pdevice,
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

	if ( displayMode == VK_NULL_HANDLE){

		printf("Desired mode not found");
		return 1;

	}

	printf("selected 1920x1200 @ 165Hz\n");
	
	uint32_t planeCount = 0;
	s = vkGetPhysicalDeviceDisplayPlanePropertiesKHR(Pdevice, &planeCount, NULL);
	
	if ( s != VK_SUCCESS){

		printf("Failed to get Plane count");
		return 1;

	}

	printf("Plane Count: %u\n", planeCount);

	uint32_t PlaneIndex = UINT32_MAX;
	
	for (uint32_t p = 0; p < planeCount; p++) {
	
		uint32_t supportedDisplayCount = 0;
		vkGetDisplayPlaneSupportedDisplaysKHR(Pdevice, p, &supportedDisplayCount, NULL);

		if (supportedDisplayCount == 0) continue;

		VkDisplayKHR supportedDisplays[supportedDisplayCount];
		vkGetDisplayPlaneSupportedDisplaysKHR(
				Pdevice,
				p,
				&supportedDisplayCount,
				supportedDisplays
			);

		for (uint32_t d = 0; d < supportedDisplayCount; d++) {
			
			if (supportedDisplays[d] == displays[0].display) {
				
				PlaneIndex = p;
				break;
				
			}
	
		}
		if (PlaneIndex != UINT32_MAX) break;
	
	}

	if (PlaneIndex == UINT32_MAX) {
		printf("No display plane supports display 0 on this GPU!\n");
		return 1;
	}

	VkDisplaySurfaceCreateInfoKHR surfaceInfo = {
				.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
				.displayMode = displayMode,
				.planeIndex = PlaneIndex,
				.planeStackIndex = 0,
				.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
				.globalAlpha = 1.0f,
				.alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
				.imageExtent = { 1920, 1200}
			};

	VkSurfaceKHR surface;

	s = vkCreateDisplayPlaneSurfaceKHR(
			instance,
			&surfaceInfo,
			NULL,
			&surface
		);

	if ( s != VK_SUCCESS){

		printf("Failed to create surface: %d\n", s);
		return 1;

	}

	printf("surface created\n");
	
	uint32_t queueFamilyCount = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(
			Pdevice,
			&queueFamilyCount,
			NULL
		);

	printf("no. of Families: %u\n", queueFamilyCount);

	VkQueueFamilyProperties queueFamilies[queueFamilyCount];

	vkGetPhysicalDeviceQueueFamilyProperties(
			Pdevice,
			&queueFamilyCount,
			queueFamilies
		);


	for (uint32_t j = 0; j < queueFamilyCount; j++){

		printf("Queue family %u: %u queues, flags = 0x%x\n", j, queueFamilies[j].queueCount, queueFamilies[j].queueFlags);
		
		VkBool32 presentSupport = VK_FALSE;

		s = vkGetPhysicalDeviceSurfaceSupportKHR(
					Pdevice,
					j,
					surface,
					&presentSupport
				);

		if ((queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport ){

			graphicsFamily = j;
			break;

		}

	}

	if (graphicsFamily == UINT32_MAX){

		printf("No (graphics and present support) queue family found");
		return 1;
	
	}

	printf("Graphics queue works\n");

	float queuePriority = 1.0f;

	VkDeviceQueueCreateInfo queueCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = graphicsFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			};

	const char* deviceExtensions[] = {	
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

	VkDeviceCreateInfo deviceCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &queueCreateInfo,
				.enabledExtensionCount = 1,
				.ppEnabledExtensionNames = deviceExtensions
			};

	VkDevice device;

	s = vkCreateDevice(
			Pdevice,
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

	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	s = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			Pdevice,
			surface,
			&surfaceCapabilities
		);

	if (s != VK_SUCCESS){

		printf("Failed to get surface capabilities\n");
		return 1;

	}

	printf("Min images: %u\n", surfaceCapabilities.minImageCount);
	printf("Max images: %u\n", surfaceCapabilities.maxImageCount);
	printf("Extent: %ux%u\n", surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);

	uint32_t formatCount = 0;

	s = vkGetPhysicalDeviceSurfaceFormatsKHR(
			Pdevice,
			surface,
			&formatCount,
			NULL
		);

	if (s != VK_SUCCESS || formatCount == 0){

		printf("Failed to get surface formats\n");
		return 1;

	}

	VkSurfaceFormatKHR formats[formatCount];
	VkSurfaceFormatKHR surfaceFormat;

	s = vkGetPhysicalDeviceSurfaceFormatsKHR(
			Pdevice,
			surface,
			&formatCount,
			formats
		);


	if (s != VK_SUCCESS){

		printf("Failed to get surface formats 2\n");
		return 1;

	}

	printf("Surface formats: %u\n", formatCount);

	for (uint32_t i = 0; i < formatCount; i++){

		printf("Format %u: %d, Color space: %d\n", i, formats[i].format, formats[i].colorSpace);
		
		if (formats[i].format == VK_FORMAT_A8B8G8R8_UNORM_PACK32){

			surfaceFormat = formats[i];
			break;

		}

	}

	uint32_t presentModeCount = 0;

	s = vkGetPhysicalDeviceSurfacePresentModesKHR(
			Pdevice,
			surface,
			&presentModeCount,
			NULL
		);

	if (s != VK_SUCCESS || presentModeCount == 0){

		printf("Failed to get present modes\n");
		return 1;

	}

	VkPresentModeKHR presentModes[presentModeCount];
	VkPresentModeKHR presentMode;

	s = vkGetPhysicalDeviceSurfacePresentModesKHR(
			Pdevice,
			surface,
			&presentModeCount,
			presentModes
		);


	if (s != VK_SUCCESS){

		printf("Failed to get present modes 2\n");
		return 1;

	}

	printf("Present modes: %u\n", presentModeCount);

	for (uint32_t i = 0; i < presentModeCount; i++){

		printf("Present modes %u: %d\n", i, presentModes[i]);
		
		if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR){

			presentMode = presentModes[i];
			break;

		}

	}

	VkSwapchainCreateInfoKHR swapchainInfo = {
				.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.surface = surface,
				.minImageCount = surfaceCapabilities.minImageCount,
				.imageFormat = surfaceFormat.format,
				.imageColorSpace = surfaceFormat.colorSpace,
				.imageExtent = surfaceCapabilities.currentExtent,
				.imageArrayLayers = 1,
				.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.preTransform = surfaceCapabilities.currentTransform,
				.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				.presentMode = presentMode,
				.clipped = VK_TRUE,
				.oldSwapchain = VK_NULL_HANDLE
			};

	VkSwapchainKHR swapchain;

	s = vkCreateSwapchainKHR(
			device,
			&swapchainInfo,
			NULL,
			&swapchain
		);

	if ( s != VK_SUCCESS){

		printf("failed to create swapchain: %d", s);
		return 1;

	}

	printf("Swapchain Created!!\n");

	uint32_t imageCount = 0;
	s = vkGetSwapchainImagesKHR(
			device,
			swapchain,
			&imageCount,
			NULL
		);

	VkImage swapchainImages[imageCount];

	s = vkGetSwapchainImagesKHR(
			device,
			swapchain,
			&imageCount,
			swapchainImages
		);

	printf("Swapchain Images: %u\n", imageCount);

	VkImageView imageViews[imageCount];

	for (uint32_t i = 0; i < imageCount; i++){
	
		VkImageViewCreateInfo viewInfo = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = swapchainImages[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = surfaceFormat.format,
				.components = {
						.r = VK_COMPONENT_SWIZZLE_IDENTITY,
						.g = VK_COMPONENT_SWIZZLE_IDENTITY,
						.b = VK_COMPONENT_SWIZZLE_IDENTITY,
						.a = VK_COMPONENT_SWIZZLE_IDENTITY
					},
				.subresourceRange = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1
					},
			};

		s = vkCreateImageView(
				device,
				&viewInfo,
				NULL,
				&imageViews[i]
			);

		if ( s != VK_SUCCESS){

			printf("Failed to create image view %u: %d\n", i, s);
			return 1;

		}

	}

	VkSemaphore imageAvailable;
	VkSemaphore renderFinished;

	VkSemaphoreCreateInfo semaphoreInfo = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
			};

	s = vkCreateSemaphore(
			device,
			&semaphoreInfo,
			NULL,
			&imageAvailable
		);
	if ( s != VK_SUCCESS){

		printf("failed to create image available Semaphore: %d\n", s);
		return 1;

	}

	s = vkCreateSemaphore(
			device,
			&semaphoreInfo,
			NULL,
			&renderFinished
		);
	if ( s != VK_SUCCESS){

		printf("failed to create render finished Semaphore: %d\n", s);
		return 1;

	}

	uint32_t imageIndex;

	s = vkAcquireNextImageKHR(
			device,
			swapchain,
			UINT64_MAX,
			imageAvailable,
			VK_NULL_HANDLE,
			&imageIndex
		);

	if ( s != VK_SUCCESS){

		printf("failed to acquire sc image: %d", s);
		return 1;

	}

	printf("acquired image: %u\n", imageIndex);

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

		printf("failed to end command buffer");
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
	
	vkDestroyCommandPool(device, cmdPool, NULL);
	vkDestroyDevice(device, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);
	vkDestroyInstance(instance, NULL);

	printf("Clean shutdown complete.\n");

	return 0;

}
