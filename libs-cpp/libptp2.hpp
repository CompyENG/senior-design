// Error codes
#define LIBPTP2_CANNOT_CONNECT  1
#define LIBPTP2_NO_DEVICE       2
#define LIBPTP2_ALREADY_OPEN    3
#define LIBPTP2_NOT_OPEN        4

// PTP Stuff -- TODO: enum
#define PTP_CONTAINER_TYPE_COMMAND  1

// Placeholder structs
struct ptp_command {
    union {
        unsigned char * data;
        struct ptp_command_data {
            uint32_t length;
            uint16_t type;
            uint16_t code;
            uint32_t transaction_id;
            char * payload;
        } d;
    } p;
};

struct ptp_response {
    int x;
};

struct script_return {
    int x;
};

struct lv_data {
    int x;
};

struct param_container {
    unsigned int length;
    unsigned short type;
    unsigned short code;
    unsigned int transaction_id;
};

class CameraBase {
    private:
        libusb_device_handle *handle;
        int usb_error;
        struct libusb_interface_descriptor *intf;
        uint8_t ep_in;
        uint8_t ep_out;
        uint32_t _transaction_id;
        void init();
    protected:
        int _bulk_write(unsigned char * bytestr, int length, int timeout);
        int _bulk_write(unsigned char * bytestr, int length);
        int _bulk_read(unsigned char * data_out, int size, int timeout);
        int _bulk_read(unsigned char * data_out, int size);
    public:
        CameraBase();
        CameraBase(libusb_device *dev);
        ~CameraBase();
        bool open(libusb_device *dev);
        bool close();
        bool reopen();
        int send_ptp_message(unsigned char * bytestr, int size, int timeout);
        int send_ptp_message(unsigned char * bytestr, int size);
        char * recv_ptp_message(int timeout);
        char * recv_ptp_message(void);
        // TODO: Should params be an int?
        struct ptp_command * new_ptp_command(int op_code, char * params, int length);
        struct ptp_command * new_ptp_command(int op_code);
        // TODO: Does C++ allow a different way of doing "default" parameter values?
        struct ptp_response * ptp_transaction(struct ptp_command * command, int * params, char * tx_data, bool receiving, int timeout);
        static libusb_device * find_first_camera();
        int get_usb_error();
};

class PTPCamera : public CameraBase {
    public:
        PTPCamera();
};

class CHDKCamera : public CameraBase {
    struct filebuf * _pack_file_for_upload(char * local_filename, char * remote_filename);
    struct filebuf * _pack_file_for_upload(char * local_filename);
    public:
        CHDKCamera();
        CHDKCamera(libusb_device *dev);
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
