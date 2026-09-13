#include "display.h"
#include "params.h"
#include <stdio.h>
#include <stdlib.h>

bool initVulkanSurface(VulkanCore* core, VulkanDisplay* display) {

    VkPhysicalDevice Pdevice = core->physicalDevice;
    VkInstance instance = core->instance;

    uint32_t displayCount = 0;

    VkResult s = vkGetPhysicalDeviceDisplayPropertiesKHR(
			Pdevice,
			&displayCount,
			NULL
		);

	printf("Display count: %u\n", displayCount);
	printf("s: %d\n", s);

	if (s != VK_SUCCESS || displayCount == 0){

		printf("No Display found");
		return false;

	}

	VkDisplayPropertiesKHR displays[displayCount];

	s = vkGetPhysicalDeviceDisplayPropertiesKHR(
			Pdevice,
			&displayCount,
			displays
		);

	if ( s != VK_SUCCESS){

		printf("failed to get display properties");
		return false;

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
		return false;

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
		return false;

	}

	printf("Modes: %u\n", modeCount);

	VkDisplayModeKHR displayMode = VK_NULL_HANDLE;

	for (uint32_t i = 0; i < modeCount; i++){

		printf("Mode %u: %ux%u @ %u Hz\n", i, modes[i].parameters.visibleRegion.width, modes[i].parameters.visibleRegion.height, modes[i].parameters.refreshRate/1000);

		if ( modes[i].parameters.visibleRegion.width == TARGET_DISPLAY_WIDTH && 
			modes[i].parameters.visibleRegion.height == TARGET_DISPLAY_HEIGHT && 
			modes[i].parameters.refreshRate == TARGET_REFRESH_RATE){

			displayMode = modes[i].displayMode;
			break;

		}

	}

	if ( displayMode == VK_NULL_HANDLE){

		printf("Desired mode not found");
		return false;

	}

	printf("Selected target %ux%u @ %uHz\n", TARGET_DISPLAY_WIDTH, TARGET_DISPLAY_HEIGHT, TARGET_REFRESH_RATE / 1000);
	
	uint32_t planeCount = 0;
	s = vkGetPhysicalDeviceDisplayPlanePropertiesKHR(Pdevice, &planeCount, NULL);
	
	if ( s != VK_SUCCESS){

		printf("Failed to get Plane count");
		return false;

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
		return false;
	}

	VkDisplaySurfaceCreateInfoKHR surfaceInfo = {
				.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
				.displayMode = displayMode,
				.planeIndex = PlaneIndex,
				.planeStackIndex = 0,
				.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
				.globalAlpha = TARGET_GLOBAL_ALPHA,
				.alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
				.imageExtent = {TARGET_DISPLAY_WIDTH, TARGET_DISPLAY_HEIGHT}
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
		return false;

	}

	printf("surface created\n");

    core->instance = instance;
    core->physicalDevice = Pdevice;
    display->surface = surface;

    return true;
}

bool initVulkanSwapchain(VulkanCore* core, VulkanDisplay* display) {

    VkPhysicalDevice Pdevice = core->physicalDevice;
    VkDevice device = core->device;
    VkInstance instance = core->instance;
    VkSurfaceKHR surface = display->surface;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;

	VkResult s = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			Pdevice,
			surface,
			&surfaceCapabilities
		);

	if (s != VK_SUCCESS){

		printf("Failed to get surface capabilities");
		return false;

	}

	printf("Min images: %u\n", surfaceCapabilities.minImageCount);
	printf("Max images: %u\n", surfaceCapabilities.maxImageCount);
	printf("Extent: %ux%u\n", surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);
	printf("Usage flags: 0x%x\n", surfaceCapabilities.supportedUsageFlags);

	uint32_t formatCount = 0;

	s = vkGetPhysicalDeviceSurfaceFormatsKHR(
			Pdevice,
			surface,
			&formatCount,
			NULL
		);

	if (s != VK_SUCCESS || formatCount == 0){

		printf("Failed to get surface formats\n");
		return false;

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
		return false;

	}

	printf("Surface formats: %u\n", formatCount);

	for (uint32_t i = 0; i < formatCount; i++){

		printf("Format %u: %d, Color space: %d\n", i, formats[i].format, formats[i].colorSpace);
		
		if (formats[i].format == TARGET_SURFACE_FORMAT && formats[i].colorSpace == TARGET_COLOR_SPACE){

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
		return false;

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
		return false;

	}

	printf("Present modes: %u\n", presentModeCount);

	for (uint32_t i = 0; i < presentModeCount; i++){

		printf("Present modes %u: %d\n", i, presentModes[i]);
		
		if (presentModes[i] == TARGET_PRESENT_MODE){

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
		return false;

	}

	printf("Swapchain Created!!\n");

    display->swapchain = swapchain;
    display->imageFormat = surfaceFormat.format;
    display->extent = surfaceCapabilities.currentExtent;

    uint32_t imageCount = 0;
	s = vkGetSwapchainImagesKHR(
			device,
			swapchain,
			&imageCount,
			NULL
		);

	display->imageCount = imageCount;
    display->images = malloc(imageCount * sizeof(VkImage));
    display->imageViews = malloc(imageCount * sizeof(VkImageView));

    if (!display->images || !display->imageViews) {
        printf("Memory allocation failed");
        return false;
    }

	s = vkGetSwapchainImagesKHR(
			device,
			swapchain,
			&imageCount,
			display->images
		);
	if (s != VK_SUCCESS) {
        printf("Failed to get swapchain images: %d", s);
        return false;
    }

	printf("Swapchain Images: %u\n", imageCount);

	VkImageView imageViews[imageCount];

	for (uint32_t i = 0; i < imageCount; i++){
	
		VkImageViewCreateInfo viewInfo = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = display->images[i],
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
				&display->imageViews[i]
			);

		if ( s != VK_SUCCESS){

			printf("Failed to create image view %u: %d\n", i, s);
			return false;

		}

	}

    return true;
}

void cleanupVulkanDisplay(VulkanCore* core, VulkanDisplay* display) {
    if (display->imageViews) {
        for (uint32_t i = 0; i < display->imageCount; i++) {
            vkDestroyImageView(core->device, display->imageViews[i], NULL);
        }
        free(display->imageViews);
    }
    
    if (display->images) {
        free(display->images);
    }

    if (display->swapchain) {
        vkDestroySwapchainKHR(core->device, display->swapchain, NULL);
    }

    if (display->surface) {
        vkDestroySurfaceKHR(core->instance, display->surface, NULL);
    }
}
