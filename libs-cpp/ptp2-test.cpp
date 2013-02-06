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
	    PTPCommand cmd = PTPCommand(PTP_CONTAINER_TYPE_COMMAND, 0x9999);
	    cmd.add_param(7);   // CHDK Script command
	    cmd.add_param(0);   // Lua script
	    cmd.transaction_id = cam.get_and_increment_transaction_id();
	    
	    PTPCommand data = PTPCommand(PTP_CONTAINER_TYPE_DATA, 0x9999);
	    data.set_payload((unsigned char *)"switch_mode_usb(1)", strlen("switch_mode_usb(1)")+1); // Compiler can take care of this optimization :/
	    data.transaction_id = cmd.transaction_id;
	    
	    /*
	    struct ptp_command * cmd = cam.new_ptp_command(0x9999, "\x07\x00\x00\x00\x00\x00\x00", 8);
        struct ptp_command * data = (struct ptp_command *)malloc(sizeof(struct ptp_command));
        data->code = cmd->code;
        data->type = 2;
        data->transaction_id = cmd->transaction_id;
        data->payload = "switch_mode_usb(1)";
        data->length = sizeof(uint32_t)+sizeof(uint16_t)+sizeof(uint16_t)+sizeof(uint32_t)+strlen(data->payload)+1;
        
        printf("Length of cmd: %d\n", cmd->length);
        
        unsigned char * send_cmd = cam.pack_ptp_command(cmd);
        unsigned char * send_data = cam.pack_ptp_command(data);
        */
        
        unsigned char * send_cmd = cmd.pack();
        unsigned char * send_data = data.pack();
        
        int i;
        printf("First char: %x\n", send_cmd[0]);
        printf("First transmission: \n");
        for(i=0;i<cmd.length;i++) {
            printf("%02x ", *(send_cmd+i));
        }
        printf("\n");
        
        int a = cam.send_ptp_message(send_cmd, cmd.length);
        printf("a = %d\n", a);
        int b = cam.send_ptp_message(send_data, data.length);
        
        printf("a = %d ; b = %d\n", a, b);
    } catch(int e) {
        printf("Error occured opening device: %d\n", e);
        return -1;
    }
    
    
		
	libusb_exit(NULL);
	
	return 0;
}
