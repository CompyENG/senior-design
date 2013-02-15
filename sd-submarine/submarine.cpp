#include <iostream>
#include <unistd.h>
#include <libptp++.hpp>
#include <libusb-1.0/libusb.h>

// We need the sub joystick class so we can reuse our data struct
#include "../sd-surface/SubJoystick.hpp"
#include "submarine.hpp"

int main(int argv, char * argc[]) {
    int r;
    
    r = libusb_init(NULL);
    if(r < 0) {
        cout << "Unable to initialize USB" << endl;
        return r;
    }
    
    libusb_device * dev = CHDKCamera::find_first_camera();
    
    if(dev == NULL) {
        cout << "No camera found." << endl;
        return -1;
    }
    
    CHDKCamera cam;
    try {
        cam.open(dev);
    } catch(int e) {
        cout << "Error occured opening device: " << e << endl;
        return -1;
    }
    
    cout << "CHDK Version: " << cam.get_chdk_version() << endl;
    
    cam.execute_lua("switch_mode_usb(1)", NULL); // TODO: block instead of sleep?
    sleep(1);
    cam.execute_lua("set_prop(121, 1)", NULL); // Set flash to manual adjustment
    usleep(500 * 10^3);   // Sleep for half a second -- TODO: Block instead?
    cam.execute_lua("set_prop(143, 2)", NULL); // Set flash mode to off
    usleep(500 * 10^3);
    
    cout << "Camera is ready" << endl;
    
    while(1) {
        // TODO: Receive data
        
        // TODO: Motor control
        
        LVData lv;
        cam.get_live_view_data(&lv, true);
        uint8_t * lv_rgb;
        uint32_t size, width, height;
        uint16_t send_width, send_height;
        lv_rgb = lv.get_rgb((int *)&size, (int *)&width, (int *)&height, true);
        
        send_width = 0xFFFF & width; // Just chop off the higher bytes
        send_height = 0xFFFF & height;
        // TODO: Send live view data
        //  Protocol: send size as four bytes, then width and height as two bytes
        //   then, send live view data
        
        
        free(lv_rgb);
    }
    
    cam.close();
    
    libusb_exit(NULL);
    
    return 0;
}
