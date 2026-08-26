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

				for( int i = 0; i < dmcr.count_connectors; i++){

					struct drm_mode_get_connector conn = {0};

					conn.connector_id = ConnectorsIds[i];

					int s = ioctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);

					if( s != -1){

						printf("\n%u\t%u\t%u\t%u\t%u", conn.connector_id, conn.connection, conn.count_modes, conn.count_encoders, conn.count_props);

						if( conn.connection == 1){

							struct drm_mode_modeinfo modes[conn.count_modes] = {};
							uint32_t encoders[conn.count_encoders] = {};
							uint32_t props[conn.count_props] = {};
							uint64_t prop_values[conn.count_props] = {};

							conn.modes_ptr = (uint64_t)(uintptr_t)modes;
							conn.encoders_ptr = (uint64_t)(uintptr_t)encoders;
							conn.props_ptr = (uint64_t)(uintptr_t)props;
							conn.prop_values_ptr = (uint64_t)(uintptr_t)prop_values;

							s = ioctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);

							if( s != -1){

								printf("\n%u",encoders[0]);

								for (int j = 0; j < conn.count_modes; j++){
								
									printf("\n%s\t%u\t%u",modes[j].name,modes[j].hdisplay,modes[j].vdisplay);

								}

							} else{ perror("get modes"); }

						}


					} else {perror("getconn fail"); }

				}

			}else { perror("GETRES"); }

		}
		else {printf("fail");}
		close(fd);
	}
	else{
		printf("fail");
	}	
}

