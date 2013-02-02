// A conversion of pyptp2 and all that comes with it
//  Targeted at libusb version 1.0, since that's recommended
#include <stdio.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

#include "libptp2.hpp"

// TODO: Implementation
CameraBase::CameraBase() {
    ;
}

CameraBase::CameraBase(libusb_device * dev) {
    ;
}

PTPCamera::PTPCamera() {
    fprintf(stderr, "This class is not implemented.\n");
}

CHDKCamera::CHDKCamera() : CameraBase() {
    ;
}

CHDKCamera::CHDKCamera(libusb_device * dev) : CameraBase(dev) {
    ;
}

libusb_device * CameraBase::find_first_camera() {
    // discover devices
    libusb_device **list;
    libusb_device *found = NULL;
    ssize_t cnt = libusb_get_device_list(NULL, &list);
    ssize_t i = 0, j = 0, k = 0;
    int err = 0;
    if (cnt < 0) {
        return NULL;
    }

    for (i = 0; i < cnt; i++) {
        libusb_device *device = list[i];
        struct libusb_config_descriptor * desc;
        int r = libusb_get_active_config_descriptor(device, &desc);
        
        if (r < 0) {
            fprintf(stderr, "failed to get config descriptor");
            return NULL;
        }
        
        for(j = 0; j < desc->bNumInterfaces; j++) {
            struct libusb_interface interface = desc->interface[j];
            for(k = 0; k < interface.num_altsetting; k++) {
                struct libusb_interface_descriptor altsetting = interface.altsetting[k];
                if(altsetting.bInterfaceClass == 6) { // If this has the PTP interface
                    found = device;
                    break;
                }
            }
            if(found) break;
        }
        
        libusb_free_config_descriptor(desc);
        
        if(found) break;
    }

    /*
    if (found) {
        libusb_device_handle *handle;

        err = libusb_open(found, &handle);
        if (err)
            error();
        // etc
    }

    libusb_free_device_list(list, 1); // Can I do this here?
    */
    
    if(found) {
        libusb_ref_device(found);     // Add a reference to the device so it doesn't get destroyed when we free_device_list
    }
    
    libusb_free_device_list(list, 1);   // Free the device list with dereferencing. Shouldn't delete our device, since we ref'd it
    
    return found;
}
