#include <fcntl.h>
#include <unistd.h>
#include </usr/include/libdrm/drm_mode.h>
#include </usr/include/libdrm/drm.h>
#include </usr/include/libdrm/drm_fourcc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>

struct drmSetup {
    int fd;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
	uint64_t size;
	uint32_t pitch;
    uint32_t handle;
    uint32_t fb_id;
    uint32_t crtc_id;
    uint32_t connector_id;
	void *mapped;
    struct drm_mode_modeinfo mode;
};

void drawPixel(struct drmSetup *setup, int x, int y){

    if (x < 0 || x >= setup->width ||
        y < 0 || y >= setup->height)
        return;

    uint32_t *pixels = setup->mapped;
    pixels[x + y * (setup->pitch / 4)] = 0x00ffffff;
	
}

int openDrmDevice(const char *path){
	
	int fd = open(path, O_RDWR);
	if ( fd == -1){
		perror("Open Drm Device Failure");
	}
	
	return fd;
}

int findDeviceConfig(struct drmSetup *setup){

	int fd = setup->fd;
	
	struct drm_mode_card_res dmcr = {0};
	
	int s = ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &dmcr);
	if ( s == -1){
		perror("Get Resources Failure");
		return -1;
	}
	
	printf("\nGet Resources Success \n Frame buffer count:%d\t Crtcs count:%d\t Connectors Count:%d\t Encoders count:%d", dmcr.count_fbs, dmcr.count_crtcs, dmcr.count_connectors, dmcr.count_encoders);

	uint32_t FbsIds[dmcr.count_fbs] = {};
	uint32_t CrtcsIds[dmcr.count_crtcs] = {};
	uint32_t ConnectorsIds[dmcr.count_connectors] = {};
	uint32_t EncodersIds[dmcr.count_encoders] = {};            

	dmcr.fb_id_ptr = (uint64_t)(uintptr_t)FbsIds;
	dmcr.crtc_id_ptr = (uint64_t)(uintptr_t)CrtcsIds;
	dmcr.connector_id_ptr = (uint64_t)(uintptr_t)ConnectorsIds;
	dmcr.encoder_id_ptr = (uint64_t)(uintptr_t)EncodersIds;
	
	s = ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &dmcr);
	if ( s == -1){
		perror("Get Id Resources Failure");
		return -1;
	}
	
	for( int i = 0; i < dmcr.count_connectors; i++){
		struct drm_mode_get_connector conn = {0};
		conn.connector_id = ConnectorsIds[i];
		
		s = ioctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn);
		if ( s == -1){
			perror("Get Connectors Failure");
			return -1;
		}
		
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
			
			if ( s == -1){
				perror("Get Connector Ids Failure");
				return -1;
			}
			
			setup->connector_id = conn.connector_id;
			
			if ( conn.count_encoders == 0){
				perror("Count Encoders = 0");
				return -1;
			}
			enc[0].encoder_id = encoders[0];
			s = ioctl(fd, DRM_IOCTL_MODE_GETENCODER, &enc[0]);
			if ( s == -1){
				perror("Get Encoders Failure");
				return -1;
			}
			
			setup->crtc_id = enc[0].crtc_id;
			
			for (int j = 0; j < conn.count_modes; j++){

				if (modes[j].hdisplay == setup->width && modes[j].vdisplay == setup->height){
					setup->mode = modes[j];
					break;
				}

			}
			
		}
		
	}
	
	return 0;
	
}

int createFrameBuffer(struct drmSetup *setup){

	int fd = setup->fd;

	struct drm_mode_create_dumb dumb = {0};
    dumb.width = setup->mode.hdisplay;
    dumb.height = setup->mode.vdisplay;
    dumb.bpp = setup->bpp;
	
	int s = ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &dumb);
	if ( s == -1){
		perror("Create Dumb Failure");
		return -1;
	}
	
	setup->size = dumb.size;
    setup->handle = dumb.handle;
	setup->pitch = dumb.pitch;
	
	struct drm_mode_fb_cmd2 fb = {0};

	fb.width = dumb.width;
	fb.height = dumb.height;
	fb.pixel_format = DRM_FORMAT_XRGB8888;
	fb.pitches[0] = dumb.pitch;
	fb.offsets[0] = 0;
	fb.handles[0] = dumb.handle;
	
	s = ioctl(fd, DRM_IOCTL_MODE_ADDFB2, &fb);
	if ( s == -1){
		perror("Create Fb Failure");
		return -1;
	}
	
	setup->fb_id = fb.fb_id;
	return 0;

}

int mapFrameBuffer(struct drmSetup *setup){

	int fd = setup->fd;
	
	struct drm_mode_map_dumb map = {};
	map.handle = setup->handle;
	
	int s = ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
	if ( s == -1){
		perror("Create Map Failure");
		return -1;
	}
	
	void *mapped = mmap(NULL, setup->size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, map.offset);
	
	if (mapped == MAP_FAILED) {
		perror("mmap failed");
		return -1;
	}
	
	setup->mapped = mapped;
	
	return 0;

}

int setDisplayMode(struct drmSetup *setup) {
    struct drm_mode_crtc crtc = {0};
    crtc.crtc_id = setup->crtc_id;
    crtc.fb_id = setup->fb_id;
    crtc.x = 0;
    crtc.y = 0;
    crtc.mode_valid = 1;
    crtc.mode = setup->mode;
    crtc.count_connectors = 1;
    crtc.set_connectors_ptr = (uint64_t)(uintptr_t)&setup->connector_id;

    if (ioctl(setup->fd, DRM_IOCTL_MODE_SETCRTC, &crtc) < 0) {
        perror("Failed to set CRTC");
        return -1;
    }
    
    printf("Successfully set display mode!\n");
    return 0;
}

int main(){
	struct drmSetup setup = {};
	setup.width = 1920;
	setup.height = 1200;
	setup.bpp = 32;
	
	setup.fd = openDrmDevice("/dev/dri/card0");
	if (setup.fd < 0) return 1;

	if (findDeviceConfig(&setup) == 0){
		if (createFrameBuffer(&setup) == 0){
			if (mapFrameBuffer(&setup) == 0){
				drawPixel(&setup, 20, 20);
				setDisplayMode(&setup);
				sleep(5);
			}
		}
	}

	if (setup.mapped && setup.mapped != MAP_FAILED) munmap(setup.mapped, setup.size);

	if (setup.fb_id) ioctl(setup.fd, DRM_IOCTL_MODE_RMFB, &setup.fb_id);

	if (setup.handle) {
		struct drm_mode_destroy_dumb destroy = {
			.handle = setup.handle
		};
		ioctl(setup.fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
	}

	close(setup.fd);
    return 0;
}
