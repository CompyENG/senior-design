// A conversion of pyptp2 and all that comes with it
//  Targeted at libusb version 1.0, since that's recommended
#include "libusb.h"

class CameraBase {
    
    protected:
        bool _bulk_write(char * bytestr, int timeout);
        bool _bulk_write(char * bytestr);
        bool _bulk_read(int size, int timeout);
        bool _bulk_read(int size);
    public:
        CameraBase();
        CameraBase(libusb_device *dev);
        bool open(libusb_device *dev);
        bool close();
        bool reopen();
        bool send_ptp_message(char * bytestr, int timeout);
        bool send_ptp_message(char * bytestr);
        char * recv_ptp_message(int timeout);
        char * recv_ptp_message(void);
        // TODO: Should params be an int?
        struct ptp_command * new_ptp_command(int op_code, int[] params);
        struct ptp_command * new_ptp_command(int op_code);
        // TODO: Does C++ allow a different way of doing "default" parameter values?
        struct ptp_response * ptp_transaction(struct ptp_command * command, int[] params, char * tx_data, bool receiving, timeout);
};

class PTPCamera : public CameraBase {
    public:
        PTPCamera() { printf("This class is not implemented.\n"); }
};

class CHDKCamera : public CameraBase {
    struct filebuf * _pack_file_for_upload(char * local_filename, char * remote_filename);
    struct filebuf * _pack_file_for_upload(char * local_filename);
    public:
        CHDKCamera() : CameraBase() { ; }
        CHDKCamera(struct usb_device *dev) : CameraBase(dev) { ; }
        float get_chdk_version(void);
        int check_script_status(void);
        struct script_return * execute_lua(char * script, bool block);
        struct script_return * execute_lua(char * script);
        struct ptp_response * read_script_message(void);
        bool write_script_message(char * message, int script_id);
        bool write_script_message(char * message);
        bool upload_file(char * local_filename, char * remote_filename, int timeout);
        char * download_file(char * filename, int timeout);
        struct lv_data * get_live_view_data(bool liveview, bool overlay, bool palette);
        char * _wait_for_script_return(int timeout);
};

libusb_device * find_first_camera() {
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
    
    return found;
}