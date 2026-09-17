#include "core.h"
#include "display.h"
#include "pipeline.h"
#include "renderer.h"
#include <stdio.h>

int main(){

	VulkanCore core = {0};
    VulkanDisplay display = {0};
    VulkanPipeline pipeline = {0};
    VulkanRenderer renderer = {0};

    if (!initVulkanInstance(&core)) return 1;
    if (!initVulkanSurface(&core, &display)) return 1;
    if (!initVulkanDevice(&core, display.surface)) return 1;
    if (!initVulkanSwapchain(&core, &display)) return 1;
    if (!initVulkanPipeline(&core, &display, &pipeline)) return 1;
    if (!initVulkanRenderer(&core, &renderer, &pipeline)) return 1;

	printf("INIT success!!\n");

	for (int i = 0; i < 5000; i++) {

		bool s = drawFrame(&core, &display, &pipeline, &renderer);
        if (!s) {
            printf("Rendering failed on frame %d\n", i);
            break;
        }
    }

	vkDeviceWaitIdle(core.device);
    
    cleanupVulkanRenderer(&core, &renderer);
    cleanupVulkanPipeline(&core, &pipeline);
    cleanupVulkanDisplay(&core, &display);
    cleanupVulkanCore(&core);

    printf("Clean shutdown complete.\n");
    return 0;
}
