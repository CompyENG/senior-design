#include <iostream>
#include <unistd.h>
#include <libptp++.hpp>
#include <libusb-1.0/libusb.h>

// We need the sub joystick class so we can reuse our data struct
#include "../sd-surface/SubJoystick.hpp"
#include "Motor.hpp"
#include "submarine.hpp"

int main(int argv, char * argc[]) {
    int error;
    CHDKCamera cam;
    Motor subMotors[4]; // We need to control 4 motors
    bool quit = false;
    
    // Keep trying to set up the camera until something tells us to stop
    while(setup_camera(&cam, &error) == false && quit == false) {
        cout << "Error setting up camera: " << error << " -- Trying again" << endl;
    }
    if(quit == true) {
        cout << "Failed to set up camera. Quitting. Last error: " << error << endl;
        return 1;
    }
    
    cout << "Camera is ready" << endl;
    cout << "CHDK Version: " << cam.get_chdk_version() << endl;
    
    // Initialize motors
    setup_motors(subMotors);
    cout << "Motors are ready" << endl;
    
    // TODO: Signal handler to allow us to quit loop when we receive SIGUSR1
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

bool setup_camera(CHDKCamera * cam, int * error) {
    libusb_device * dev;
    
    *error = libusb_init(NULL);
    if(*error < 0) {
        return false;
    }
    
    *error = 0;
    
    dev = CHDKCamera::find_first_camera();
    
    if(dev == NULL) {
        *error = 1;
        return false;
    }
    
    try {
        cam->open(dev);
    } catch(int e) {
        *error = e;
        return false;
    }
    
    cam->execute_lua("switch_mode_usb(1)", NULL); // TODO: block instead of sleep?
    sleep(1);
    cam->execute_lua("set_prop(121, 1)", NULL); // Set flash to manual adjustment
    usleep(500 * 10^3);   // Sleep for half a second -- TODO: Block instead?
    cam->execute_lua("set_prop(143, 2)", NULL); // Set flash mode to off
    usleep(500 * 10^3);
}

void setup_motors(Motor * subMotors) {
    int i;
    
    for(i=0;i<4;i++) {
        subMotors[i].setup(MOTOR_PINS[i]);
    }
}
