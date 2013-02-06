#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "libptp2.hpp"
#include "live_view.h"

using namespace std;

static uint8_t clip(int v) {
    if (v<0) return 0;
    if (v>255) return 255;
    return v;
}

void yuv_to_rgb(uint8_t **dest, uint8_t y, int8_t u, int8_t v)
{
    *((*dest)++) = clip(((y<<12) +          v*5743 + 2048)>>12);
    *((*dest)++) = clip(((y<<12) - u*1411 - v*2925 + 2048)>>12);
    *((*dest)++) = clip(((y<<12) + u*7258          + 2048)>>12);
}


int main(int argc, char * argv[]) {
    int r;
    
    r = libusb_init(NULL);
    if (r < 0)
        return r;
        
    libusb_device * dev = CHDKCamera::find_first_camera();
    
    if(dev == NULL) {
        fprintf(stderr, "No camera found.\n");
        return -1;
    }
    
    struct libusb_device_descriptor desc;
	r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		fprintf(stderr, "failed to get device descriptor");
		return -1;
	}

	printf("%04x:%04x (bus %d, device %d)\n",
		desc.idVendor, desc.idProduct,
		libusb_get_bus_number(dev), libusb_get_device_address(dev));
		
	try {
	    CHDKCamera cam(dev);
		
		cout << "CHDK Version: " << cam.get_chdk_version() << endl;
		
        cam.execute_lua("switch_mode_usb(1)", NULL);
        
        sleep(1);
        
        //cam.execute_lua("shoot()", NULL);
        
        //sleep(3);
        
        /*
        uint8_t * lv_data;
        int width, height, vp_size;
        
        lv_data = cam.get_live_view_data(&width, &height, &vp_size, true);
        
        printf("Got vp_size: %d\n", vp_size);
        */
        
        PTPContainer cmd(PTP_CONTAINER_TYPE_COMMAND, 0x9999);
        cmd.add_param(CHDK_OP_GET_DISPLAY_DATA);
        cmd.add_param(LV_TFR_VIEWPORT);
        
        PTPContainer out_resp, out_data;
        cam.ptp_transaction(&cmd, NULL, true, &out_resp, &out_data);
        
        // Live view data parsing test:
        lv_data_header lv_head;
        int payload_size;
        unsigned char * payload = out_data.get_payload(&payload_size);
        memcpy(&lv_head, payload, sizeof(lv_data_header));
        
        lv_framebuffer_desc vp_head;
        memcpy(&vp_head, payload + lv_head.vp_desc_start, sizeof(lv_framebuffer_desc));
        
        unsigned char * vp_data;
        int vp_size;
        vp_size = (vp_head.buffer_width*vp_head.visible_height*12)/8; // 12 bpp... 8=...?
        printf("Requesting size: %d\n", vp_size);
        vp_data = (unsigned char *)malloc(vp_size);
        memcpy(vp_data, payload + vp_head.data_start, vp_size);
        int skip = 1;
        int par = (skip == 1)?2:1;
        
        unsigned vwidth = vp_head.visible_width/par;
	    unsigned dispsize = vwidth*(vp_head.visible_height);
	    printf("vwidth: %d\ndispsize*3: %d\n", vwidth, dispsize*3);
	    
        uint8_t * out = (uint8_t *) malloc(dispsize*3);
        uint8_t * prgb_data = out;
        
        //cam.yuv_live_to_cd_rgb((char *)vp_data, vp_head.buffer_width, vp_head.visible_width, vp_head.visible_height, skip, out, out+1, out+2);
        uint8_t * p_yuv = vp_data;
        int i;
        for (i=0;i<(vp_head.buffer_width*vp_head.visible_height); i+=4, p_yuv+=6) {
            yuv_to_rgb(&prgb_data,p_yuv[1],p_yuv[0],p_yuv[2]);
            yuv_to_rgb(&prgb_data,p_yuv[3],p_yuv[0],p_yuv[2]);
            if(skip)
                continue;
            yuv_to_rgb(&prgb_data,p_yuv[4],p_yuv[0],p_yuv[2]);
            yuv_to_rgb(&prgb_data,p_yuv[5],p_yuv[0],p_yuv[2]);
        }
        
        ofstream img_file;
        img_file.open("lv-test-cpp.ppm", ios::out | ios::binary);
        img_file << "P6 " << vwidth << " " << vp_head.visible_height << " 255" << endl;
        //img_file.write((char *)vp_data, vp_size);
        img_file.write((char *)out, dispsize*3);
        img_file.close();
        
        free(vp_data);
        
    } catch(int e) {
        printf("Error occured opening device: %d\n", e);
        return -1;
    }
    
	libusb_exit(NULL);
	
	return 0;
}
