#include <fcntl.h>
#include <unistd.h>
#include </usr/include/libdrm/drm_mode.h>
#include </usr/include/libdrm/drm.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>

int main(){

	struct drm_mode_card_res dmcr = {0};

	int fd = open("/dev/dri/card0",O_RDWR);
	
	if (fd != -1){
		printf("code = %d",fd);
		int s = ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &dmcr);
		if (s != -1){
			printf("\nSUCCES! \n %d\t%d\t%d\t%d",dmcr.count_fbs, dmcr.count_crtcs, dmcr.count_connectors, dmcr.count_encoders);

			uint32_t FbsIds[dmcr.count_fbs] = {};
			uint32_t CrtcsIds[dmcr.count_crtcs] = {};
			uint32_t ConnectorsIds[dmcr.count_connectors] = {};
			uint32_t EncodersIds[dmcr.count_encoders] = {};			
		
			dmcr.fb_id_ptr = (uint64_t)(uintptr_t)FbsIds;
			dmcr.crtc_id_ptr = (uint64_t)(uintptr_t)CrtcsIds;
			dmcr.connector_id_ptr = (uint64_t)(uintptr_t)ConnectorsIds;
			dmcr.encoder_id_ptr = (uint64_t)(uintptr_t)EncodersIds;
			
			
			int s = ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &dmcr);
			if (s != -1){
				printf("\nSUCCES! \n %d\t%d", CrtcsIds[0], ConnectorsIds[0]);
			}else { perror("GETRES"); }

		}
		else {printf("fail");}
		close(fd);
	}
	else{
		printf("fail");
	}	
}

