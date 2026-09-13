#include "core.h"
#include "params.h"
#include <stdio.h>
#include <stdlib.h>

bool initVulkanInstance(VulkanCore* core) {

    VkInstance instance;

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
		return false;

	}

	uint32_t deviceCount = 0;
	s = vkEnumeratePhysicalDevices(
				instance,
				&deviceCount,
				NULL
			);

	if ( s != VK_SUCCESS || deviceCount == 0) {

		printf("failure to enumerate GPUs: %d\n", s);
        return false;

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

		printf("GPU %u: %s\n", i, properties.deviceName);

		VkPhysicalDeviceFeatures features;

		vkGetPhysicalDeviceFeatures(
				devices[i],
				&features
			);

		printf("API Version: %u,%u.%u\n", VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));

		printf("Geo Shader: %s\n", features.geometryShader ? "Yes" : "No");

		if (properties.deviceType == PREFERRED_DEVICE_TYPE){

			Pdevice = devices[i];
            printf("Selected GPU: %s\n", properties.deviceName);
            break;

		} else if (properties.deviceType == FALLBACK_DEVICE_TYPE)
		{
			Pdevice = devices[i];
            printf("Selected GPU: %s\n", properties.deviceName);
		}

	}
	
	if (Pdevice == VK_NULL_HANDLE) {

		printf("No discrete GPU found\n");
		return false;

	}

    core->instance = instance;
    core->physicalDevice = Pdevice;

    return true;

}

bool initVulkanDevice(VulkanCore* core, VkSurfaceKHR surface){

    VkPhysicalDevice Pdevice = core->physicalDevice;

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

    uint32_t graphicsFamily = UINT32_MAX;

    for (uint32_t j = 0; j < queueFamilyCount; j++){

		printf("Queue family %u: %u queues, flags = 0x%x\n", j, queueFamilies[j].queueCount, queueFamilies[j].queueFlags);
		
		VkBool32 presentSupport = VK_FALSE;

		VkResult s = vkGetPhysicalDeviceSurfaceSupportKHR(
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
		return false;
	
	}

	printf("Graphics queue works\n");

    float queuePriority = 1.0f;

	VkDeviceQueueCreateInfo queueCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = graphicsFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			};

	VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
				.dynamicRendering = VK_TRUE
			};

	const char* deviceExtensions[] = {	
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

	VkDeviceCreateInfo deviceCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pNext = &dynamicRendering,
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
		return false;
	
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

    core->device = device;
    core->graphicsQueue = graphicsQueue;
    core->graphicsFamily = graphicsFamily;
    core->physicalDevice = Pdevice;

    return true;

}

void cleanupVulkanCore(VulkanCore* core){

    if (core->device) {
        vkDestroyDevice(core->device, NULL);
    }
    if (core->instance) {
        vkDestroyInstance(core->instance, NULL);
    }

}