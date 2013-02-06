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

void yuv_live_to_cd_rgb(const char *p_yuv,
						unsigned buf_width, unsigned buf_height,
						unsigned x_offset, unsigned y_offset,
						unsigned width,unsigned height,
						int skip,
						uint8_t *r,uint8_t *g,uint8_t *b);
						


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
	    PTPContainer cmd(PTP_CONTAINER_TYPE_COMMAND, 0x9999);
	    cmd.add_param(CHDK_OP_EXECUTE_SCRIPT);   // CHDK Script command
	    cmd.add_param(CHDK_LANGUAGE_LUA);   // Lua script
	    
	    PTPContainer data(PTP_CONTAINER_TYPE_DATA, 0x9999);
	    data.set_payload((unsigned char *)"switch_mode_usb(1)", strlen("switch_mode_usb(1)")+1); // Compiler can take care of this optimization :/
        
	    cam.ptp_transaction(&cmd, &data, false, NULL);
        
        //sleep(5);
        
        PTPContainer read = PTPContainer();
        
        PTPContainer cmd2(PTP_CONTAINER_TYPE_COMMAND, 0x9999);
        cmd2.add_param(CHDK_OP_GET_DISPLAY_DATA);
        cmd2.add_param(0x01);
        
        cam.ptp_transaction(&cmd2, NULL, true, &read);
        
        // Live view data parsing test:
        lv_data_header lv_head;
        int payload_size;
        unsigned char * payload = read.get_payload(&payload_size);
        memcpy(&lv_head, payload, sizeof(lv_data_header));
        printf("LV Version: %d.%d\n", lv_head.version_major, lv_head.version_minor);
        
        lv_framebuffer_desc vp_head;
        memcpy(&vp_head, payload + lv_head.vp_desc_start, sizeof(lv_framebuffer_desc));
        printf("Viewport size: %dx%d\n", vp_head.visible_width, vp_head.visible_height);
        
        int vp_size;
        unsigned char * vp_data;
        vp_size = (vp_head.buffer_width*vp_head.visible_height*6)/4;
        vp_data = (unsigned char *)malloc(vp_size);
        memcpy(vp_data, payload + vp_head.data_start, vp_size);
        printf("Copied data over\n");
        
        // Convert colorspace:
        //int width = vp_head.visible_width;
        //int height = vp_head.visible_height;
        // TODO: Sort out inconsistencies in the width and height
        // TODO: Probably make a new class to handle live view data
        int width = 360;
        int height = 480;
        int outLen = vp_head.visible_width * vp_head.visible_height;
	    uint8_t * out = (uint8_t *) malloc(outLen*3);
	    
	    yuv_live_to_cd_rgb((char *)vp_data, width, height, 0, 0, width, height, 0, out, out+1, out+2);
        
        ofstream img_file;
        img_file.open("lv-test-cpp.ppm", ios::out | ios::binary);
        img_file << "P6 " << width << " " << height/2 << " 255" << endl;
        img_file.write((char *)out, vp_size);
        img_file.close();
        
        free(vp_data);
        
    } catch(int e) {
        printf("Error occured opening device: %d\n", e);
        return -1;
    }
    
	libusb_exit(NULL);
	
	return 0;
}

static uint8_t clip_yuv(int v) {
	if (v<0) return 0;
	if (v>255) return 255;
	return v;
}

static uint8_t yuv_to_r(uint8_t y, int8_t v) {
	return clip_yuv(((y<<12) +          v*5743 + 2048)>>12);
	//return r[y][((int)v)+128];
}

static uint8_t yuv_to_g(uint8_t y, int8_t u, int8_t v) {
	return clip_yuv(((y<<12) - u*1411 - v*2925 + 2048)>>12);
	//return g[y][((int)u)+128][((int)v)+128];
}

static uint8_t yuv_to_b(uint8_t y, int8_t u) {
	return clip_yuv(((y<<12) + u*7258          + 2048)>>12);
	//return b[y][((int)u)+128];
}

void yuv_live_to_cd_rgb(const char *p_yuv,
						unsigned buf_width, unsigned buf_height,
						unsigned x_offset, unsigned y_offset,
						unsigned width,unsigned height,
						int skip,
						uint8_t *r,uint8_t *g,uint8_t *b) {
	unsigned x,row;
	unsigned row_inc = (buf_width*12)/8;
	const char *p;
	// start at end to flip for CD
	const char *p_row = p_yuv + (y_offset * row_inc) + ((x_offset*12)/8);
	for(row=0;row<height;row++,p_row += row_inc) {
		for(x=0,p=p_row;x<width;x+=4,p+=6) {
			*r = yuv_to_r(p[1],p[2]);
			*g = yuv_to_g(p[1],p[0],p[2]);
			*b = yuv_to_b(p[1],p[0]);

			*r = yuv_to_r(p[3],p[2]);
			*g = yuv_to_g(p[3],p[0],p[2]);
			*b = yuv_to_b(p[3],p[0]);
			r+=3;g+=3;b+=3;
			if(!skip) {
				// TODO it might be better to use the next pixels U and V values
				*r = yuv_to_r(p[4],p[2]);
				*g = yuv_to_g(p[4],p[0],p[2]);
				*b = yuv_to_b(p[4],p[0]);

				*r = yuv_to_r(p[5],p[2]);
				*g = yuv_to_g(p[5],p[0],p[2]);
				*b = yuv_to_b(p[5],p[0]);
				r+=3;g+=3;b+=3;
			}
		}
	}
}

