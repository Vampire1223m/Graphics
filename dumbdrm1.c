#include <fcntl.h>
#include <unistd.h>
#include </usr/include/libdrm/drm_mode.h>
#include </usr/include/libdrm/drm.h>
#include </usr/include/libdrm/drm_fourcc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>

int main(){

	struct drm_mode_card_res dmcr = {0};
	struct drm_mode_create_dumb dumb = {0};

	int fd = open("/dev/dri/card0",O_RDWR);
	
	if ( fd != -1){
		printf("code = %d",fd);

		dumb.width = 1920;
		dumb.height = 1200;
		dumb.bpp = 32;

		int s = ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &dumb);

		if( s != -1){
			
			printf("\n%u\t%u\t%llu", dumb.handle, dumb.pitch, (unsigned long long)dumb.size);

			struct drm_mode_fb_cmd2 fb = {0};
			fb.width = dumb.width;
			fb.height = dumb.height;
			fb.pixel_format = DRM_FORMAT_XRGB8888;
			fb.pitches[0] = dumb.pitch;
			fb.offsets[0] = 0;
			fb.handles[0] = dumb.handle;
			
			s = ioctl(fd, DRM_IOCTL_MODE_ADDFB2, &fb);

			if( s != -1){

				printf("\n%u",fb.fb_id);

				struct drm_mode_map_dumb map = {0};
				map.handle = dumb.handle;

				s = ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map);

				if( s != -1){

					printf("\n%llu",(unsigned long long)map.offset);

					void *mapped = mmap(NULL, dumb.size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, map.offset);

					if( mapped != MAP_FAILED){

						printf("\nsucces");

						uint32_t *pixels = (uint32_t *)mapped;

						for (int i = 0; i < dumb.width*dumb.height/2; i++){

							pixels[i] = 0x00ffffff;

						}

					} else{ perror("map failed");}

				} else{ perror("map dumb");}

			} else{ perror("create fb");}

		} else{ perror("create dumb");}

		s = ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &dmcr);

		if ( s != -1){
			printf("\nSUCCES! \n %d\t%d\t%d\t%d",dmcr.count_fbs, dmcr.count_crtcs, dmcr.count_connectors, dmcr.count_encoders);

			uint32_t FbsIds[dmcr.count_fbs] = {};
			uint32_t CrtcsIds[dmcr.count_crtcs] = {};
			uint32_t ConnectorsIds[dmcr.count_connectors] = {};
			uint32_t EncodersIds[dmcr.count_encoders] = {};			
		
			dmcr.fb_id_ptr = (uint64_t)(uintptr_t)FbsIds;
			dmcr.crtc_id_ptr = (uint64_t)(uintptr_t)CrtcsIds;
			dmcr.connector_id_ptr = (uint64_t)(uintptr_t)ConnectorsIds;
			dmcr.encoder_id_ptr = (uint64_t)(uintptr_t)EncodersIds;
			
			
			s = ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &dmcr);
			if ( s != -1){
				printf("\nSUCCES! \n %d\t%d", CrtcsIds[0], ConnectorsIds[0]);

				for( int i = 0; i < dmcr.count_connectors; i++){

					struct drm_mode_get_connector conn = {0};

					conn.connector_id = ConnectorsIds[i];

					s = ioctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);

					if( s != -1){

						printf("\n%u\t%u\t%u\t%u\t%u", conn.connector_id, conn.connection, conn.count_modes, conn.count_encoders, conn.count_props);

						if( conn.connection == 1){

							struct drm_mode_modeinfo modes[conn.count_modes] = {};
							struct drm_mode_get_encoder enc[conn.count_encoders] = {};
							uint32_t encoders[conn.count_encoders] = {};
							uint32_t props[conn.count_props] = {};
							uint64_t prop_values[conn.count_props] = {};

							conn.modes_ptr = (uint64_t)(uintptr_t)modes;
							conn.encoders_ptr = (uint64_t)(uintptr_t)encoders;
							conn.props_ptr = (uint64_t)(uintptr_t)props;
							conn.prop_values_ptr = (uint64_t)(uintptr_t)prop_values;

							s = ioctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);

							if( s != -1){

								for (int j = 0; j < conn.count_encoders; j++){

									enc[j].encoder_id = encoders[j];

									struct drm_mode_crtc crtc = {};

									s = ioctl(fd, DRM_IOCTL_MODE_GETENCODER, &enc[j]);

									if ( s != -1){

										printf("\n%u\t%u\t%x\t%x", enc[j].encoder_id, enc[j].crtc_id, enc[j].possible_crtcs, enc[j].possible_clones);

										crtc.crtc_id = enc[j].crtc_id;

										s = ioctl(fd, DRM_IOCTL_MODE_GETCRTC, &crtc);

										if ( s != -1){

											printf("\n%u\t%u\t%u\t%u\t%u\t%u", crtc.fb_id, crtc.x, crtc.y, crtc.mode.vdisplay, crtc.mode.hdisplay, crtc.mode_valid);

											struct drm_mode_fb_cmd fb = {};

											fb.fb_id = crtc.fb_id;
											int s = ioctl(fd, DRM_IOCTL_MODE_GETFB, &fb);

											if ( s != -1){

												printf("\n%u\t%u\t%u\t%u\t%u\t%u", fb.fb_id, fb.width, fb.height, fb.pitch, fb.bpp, fb.depth);

											} else { perror("get fb");}
												
										} else{ perror("get crtc");}

									} else{ perror("get encoders");}

								}

								for (int j = 0; j < conn.count_modes; j++){
								
									printf("\n%s\t%u\t%u",modes[j].name,modes[j].hdisplay,modes[j].vdisplay);

								}

								
								struct drm_mode_crtc crtc = {0};

								crtc.crtc_id = enc[0].crtc_id;
								crtc.fb_id = 115;

								crtc.x = 0;
								crtc.y = 0;
								crtc.mode_valid = 1;
								crtc.mode = modes[0];
								crtc.count_connectors = 1;
								crtc.set_connectors_ptr = (uint64_t)(uintptr_t)&conn.connector_id;

								s = ioctl(fd, DRM_IOCTL_MODE_SETCRTC, &crtc);
								if ( s != -1){

									printf("SUCCESSSS");

								} else{ perror("set crtc error"); }
										
							
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

