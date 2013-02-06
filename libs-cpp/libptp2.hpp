// Error codes
#define LIBPTP2_CANNOT_CONNECT  1
#define LIBPTP2_NO_DEVICE       2
#define LIBPTP2_ALREADY_OPEN    3
#define LIBPTP2_NOT_OPEN        4

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

// Placeholder structs
struct ptp_command {
    uint32_t length;
    uint16_t type;
    uint16_t code;
    uint32_t transaction_id;
    char * payload;
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
        int send_ptp_message(PTPContainer cmd, int timeout);
        int send_ptp_message(PTPContainer cmd);
        int send_ptp_message(PTPContainer * cmd, int timeout);
        int send_ptp_message(PTPContainer * cmd);
        PTPContainer recv_ptp_message(int timeout);
        PTPContainer recv_ptp_message(void);
        // TODO: Does C++ allow a different way of doing "default" parameter values?
        void ptp_transaction(PTPContainer *cmd, PTPContainer *data, bool receiving, PTPContainer *out, int timeout);
        void ptp_transaction(PTPContainer *cmd, PTPContainer *data, bool receiving, PTPContainer *out);
        static libusb_device * find_first_camera();
        int get_usb_error();
        unsigned char * pack_ptp_command(struct ptp_command * cmd);
        int get_and_increment_transaction_id(); // What a beautiful name for a function
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
