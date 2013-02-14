#ifndef _LIBPTPPP_H
#define _LIBPTPPP_H

#include <libusb-1.0/libusb.h>
#include "live_view.h"

// Error codes
#define LIBPTP2_CANNOT_CONNECT  1
#define LIBPTP2_NO_DEVICE       2
#define LIBPTP2_ALREADY_OPEN    3
#define LIBPTP2_NOT_OPEN        4
#define LIBPTP2_CANNOT_RECV     5
#define LIBPTP2_TIMEOUT         6
#define LIBPTP2_INVALID_RESPONSE 7

#define PTPCONTAINER_NO_PAYLOAD     1
#define PTPCONTAINER_INVALID_PARAM  2

#define LVDATA_NOT_ENOUGH_DATA  1

// PTP Stuff
enum PTP_CONTAINER_TYPE {
    PTP_CONTAINER_TYPE_COMMAND  = 1,
    PTP_CONTAINER_TYPE_DATA     = 2,
    PTP_CONTAINER_TYPE_RESPONSE = 3,
    PTP_CONTAINER_TYPE_EVENT    = 4
};

enum CHDK_OPERATIONS {
    CHDK_OP_VERSION     = 0,
    CHDK_OP_GET_MEMORY,
    CHDK_OP_SET_MEMORY,
    CHDK_OP_CALL_FUNCTION,
    CHDK_OP_TEMP_DATA,
    CHDK_OP_UPLOAD_FILE,
    CHDK_OP_DOWNLOAD_FILE,
    CHDK_OP_EXECUTE_SCRIPT,
    CHDK_OP_SCRIPT_STATUS,
    CHDK_OP_SCRIPT_SUPPORT,
    CHDK_OP_READ_SCRIPT_MSG,
    CHDK_OP_WRITE_SCRIPT_MSG,
    CHDK_OP_GET_DISPLAY_DATA
};

enum CHDK_TYPES {
    CHDK_TYPE_UNSUPPORTED = 0,
    CHDK_TYPE_NIL,
    CHDK_TYPE_BOOLEAN,
    CHDK_TYPE_INTEGER,
    CHDK_TYPE_STRING,
    CHDK_TYPE_TABLE
};

enum CHDK_TEMP_DATA {
    CHDK_TEMP_DOWNLOAD = 1, // Download data instead of upload
    CHDK_TEMP_CLEAR
};

enum CHDK_SCRIPT_LANGAUGE {
    CHDK_LANGUAGE_LUA   = 0,
    CHDK_LANGUAGE_UBASIC
};

enum CHDK_SCRIPT_STATUS {
    CHDK_SCRIPT_STATUS_NONE = 0,    // No script running
    CHDK_SCRIPT_STATUS_RUN,         // Script running
    CHDK_SCRIPT_STATUS_MSG          // Messages waiting
};

enum CHDK_PTP_RESP {
    CHDK_PTP_RC_OK = 0x2001,
    CHDK_PTP_RC_GeneralError = 0x2002,
    CHDK_PTP_RC_ParameterNotSupported = 0x2006,
    CHDK_PTP_RC_InvalidParameter = 0x201D
};

// Have to define the helper class first, or I can't use it in CameraBase
class PTPContainer {
    private:
        static const uint32_t default_length = sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint16_t)+sizeof(uint16_t);
        uint32_t length;
        unsigned char * payload;    // We'll deal with this completely internally
        void init();
    public:
        uint16_t type;
        uint16_t code;
        uint32_t transaction_id;    // We'll end up setting this externally
        PTPContainer();
        PTPContainer(uint16_t type, uint16_t op_code);
        PTPContainer(unsigned char * data);
        ~PTPContainer();
        void add_param(uint32_t param);
        void set_payload(unsigned char * payload, int payload_length);
        unsigned char * pack();
        unsigned char * get_payload(int * size_out);  // This might end up being useful...
        uint32_t get_length();  // So we can get, but not set
        void unpack(unsigned char * data);
        uint32_t get_param_n(uint32_t n);
};

class LVData {
    lv_data_header * vp_head;
    lv_framebuffer_desc * fb_desc;
    uint8_t * payload;
    void init();
    static uint8_t clip(int v);
    static void yuv_to_rgb(uint8_t **dest, uint8_t y, int8_t u, int8_t v);
    public:
        LVData();
        LVData(uint8_t * payload, int payload_size);
        ~LVData();
        void read(uint8_t * payload, int payload_size);
        void read(PTPContainer * container);    // Could this make life easier?
        uint8_t * get_rgb(int * out_size, int * out_width, int * out_height, bool skip=false);    // Some cameras don't require skip
        float get_lv_version();
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
        int _bulk_write(unsigned char * bytestr, int length, int timeout=0);
        int _bulk_read(unsigned char * data_out, int size, int * transferred, int timeout=0);
        int get_and_increment_transaction_id(); // What a beautiful name for a function
    public:
        CameraBase();
        CameraBase(libusb_device *dev);
        ~CameraBase();
        bool open(libusb_device *dev);
        bool close();
        bool reopen();
        int send_ptp_message(PTPContainer * cmd, int timeout=0);
        void recv_ptp_message(PTPContainer *out, int timeout=0);
        void ptp_transaction(PTPContainer *cmd, PTPContainer *data, bool receiving, PTPContainer *out_resp, PTPContainer *out_data, int timeout=0);
        static libusb_device * find_first_camera();
        int get_usb_error();
};

class PTPCamera : public CameraBase {
    public:
        PTPCamera();
};

class CHDKCamera : public CameraBase {
    static uint8_t * _pack_file_for_upload(uint32_t * out_size, char * local_filename, char * remote_filename=NULL);
    public:
        CHDKCamera();
        CHDKCamera(libusb_device *dev);
        float get_chdk_version(void);
        uint32_t check_script_status(void);
        uint32_t execute_lua(char * script, uint32_t * script_error, bool block=false);
        void read_script_message(PTPContainer * out_data, PTPContainer * out_resp);
        uint32_t write_script_message(char * message, uint32_t script_id=0);
        bool upload_file(char * local_filename, char * remote_filename, int timeout=0);
        char * download_file(char * filename, int timeout);
        void get_live_view_data(LVData * data_out, bool liveview=true, bool overlay=false, bool palette=false);
        char * _wait_for_script_return(int timeout);
};

#endif /* _LIBPTP++_H */
