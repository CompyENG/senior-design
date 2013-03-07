/**
 * A small script to test sending commands to a script.
 * Should allow us to have a smooth, continuous zoom in, and generally be more
 * reliable, without interfering with getting live view data.
 * Additionally, this would allow us to retrieve information FROM the camera if
 * we wanted to.
 */

#include <iostream>
#include <string>
#include "../libs/libptp++.hpp"

bool setup_camera(CHDKCamera * cam, int * error);

int main(int argc, char * argv[]) {
    CHDKCamera cam;
    int e;
    string inp = "";
    
    if(!setup_camera(&cam, &e)) {
        std::cout << "An error occured when setting up the camera: " << e << std::endl;
        return 1;
    }
    
    cam.execute_lua("loadfile('A/CHDK/SCRIPTS/sd-sub.lua')()");
    
    do {
        cout << "cmd: ";
        getline(cin, inp);
        
        cam.write_script_message(inp.c_str());
        
    } while(inp.compare("") != 0);
    
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
    
    return true;
}