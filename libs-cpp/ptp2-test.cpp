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
        
        LVData lv;
        cam.get_live_view_data(&lv, true);
        
        cout << "Live view version: " << lv.get_lv_version() << endl;
        
        uint8_t * lv_rgb;
        int size, width, height;
        lv_rgb = lv.get_rgb(&size, &width, &height, true);
        
        ofstream img_file;
        img_file.open("lv-test-cpp.ppm", ios::out | ios::binary);
        img_file << "P6 " << width << " " << height << " 255" << endl;
        img_file.write((char *)lv_rgb, size);
        img_file.close();
        
        free(lv_rgb);
        
    } catch(int e) {
        printf("Error occured opening device: %d\n", e);
        return -1;
    }
    
    libusb_exit(NULL);
    
    return 0;
}
