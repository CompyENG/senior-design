#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
		
	try {
	    CHDKCamera cam(dev);
	    struct ptp_command * cmd = cam.new_ptp_command(0x9999, "70", 3);
        struct ptp_command * data = (struct ptp_command *)malloc(sizeof(struct ptp_command));
        data->p.d.code = cmd->p.d.code;
        data->p.d.type = 2;
        data->p.d.transaction_id = cmd->p.d.transaction_id;
        data->p.d.payload = "switch_mode_usb(1)";
        data->p.d.length = sizeof(uint32_t)+sizeof(uint16_t)+sizeof(uint16_t)+sizeof(uint32_t)+19;
        
        int a = cam.send_ptp_message(cmd->p.data, cmd->p.d.length);
        int b = cam.send_ptp_message(data->p.data, data->p.d.length);
        
        printf("a = %d ; b = %d\n", a, b);
    } catch(int e) {
        printf("Error occured opening device: %d\n", e);
        return -1;
    }
    
    
		
	libusb_exit(NULL);
	
	return 0;
}
