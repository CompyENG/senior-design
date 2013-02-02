#include <stdio.h>
#include <libusb-1.0/libusb.h>

#include "libptp2.hpp"

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
		
	libusb_exit(NULL);
	
	return 0;
}
