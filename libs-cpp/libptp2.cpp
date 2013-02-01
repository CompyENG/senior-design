// A conversion of pyptp2 and all that comes with it
//  Targeted at libusb version 0.1, since that's what pyptp2 uses
#include "usb.h"

class _CameraBase {
    
    protected:
        bool _bulk_write(char * bytestr, int timeout);
        bool _bulk_write(char * bytestr);
        bool _bulk_read(int size, int timeout);
        bool _bulk_read(int size);
    public:
        _CameraBase();
        _CameraBase(struct usb_device *dev);
        bool open(struct usb_device *dev);
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

class PTPCamera : public _CameraBase {
    public:
        PTPCamera() { printf("This class is not implemented.\n"); }
};

class CHDKCamera : public _CameraBase {
    struct filebuf * __pack_file_for_upload(char * local_filename, char * remote_filename);
    struct filebuf * __pack_file_for_upload(char * local_filename);
    public:
        CHDKCamera() : _CameraBase() { ; }
        CHDKCamera(struct usb_device *dev) : _CameraBase(dev) { ; }
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
