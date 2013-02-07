// A conversion of pyptp2 and all that comes with it
//  Targeted at libusb version 1.0, since that's recommended
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libusb-1.0/libusb.h>

#include "libptp2.hpp"
#include "live_view.h"

// TODO: Implementation
void CameraBase::init() {
    this->handle = NULL;
    this->usb_error = 0;
    this->intf = NULL;
    this->ep_in = 0;
    this->ep_out = 0;
    this->_transaction_id = 0;
}

CameraBase::CameraBase() {
    this->init();
}

CameraBase::CameraBase(libusb_device * dev) {
    this->init();
    
    if(dev == NULL) {
        throw LIBPTP2_NO_DEVICE;
    }
    
    this->open(dev);
}

CameraBase::~CameraBase() {
    if(this->handle != NULL) {
        libusb_close(this->handle);
        libusb_release_interface(this->handle, this->intf->bInterfaceNumber);
    }
}

int CameraBase::_bulk_write(unsigned char * bytestr, int length, int timeout) {
    int transferred;
    
    if(this->handle == NULL) {
        throw LIBPTP2_NOT_OPEN;
        return 0;
    }
    
    // TODO: Return the amount of data transferred? Check it here? What should we do if not enough was sent?
    return libusb_bulk_transfer(this->handle, this->ep_out, bytestr, length, &transferred, timeout);
}

int CameraBase::_bulk_write(unsigned char * bytestr, int length) {
    return this->_bulk_write(bytestr, length, 0);
}

int CameraBase::_bulk_read(unsigned char * data_out, int size, int * transferred, int timeout) {
    if(this->handle == NULL) {
        throw LIBPTP2_NOT_OPEN;
        return 0;
    }
    
    // TODO: Return the amount of data transferred? We might get less than we ask for, which means we need to tell the calling function?
    return libusb_bulk_transfer(this->handle, this->ep_in, data_out, size, transferred, timeout);
}

int CameraBase::_bulk_read(unsigned char * data_out, int size, int * transferred) {
    return this->_bulk_read(data_out, size, transferred, 0);
}

int CameraBase::send_ptp_message(unsigned char * data, int size, int timeout) {
    return this->_bulk_write(data, size, timeout);
}

int CameraBase::send_ptp_message(unsigned char * data, int size) {
    return this->send_ptp_message(data, size, 0);
}

int CameraBase::send_ptp_message(PTPContainer * cmd, int timeout) {
    return this->send_ptp_message(cmd->pack(), cmd->get_length(), timeout);
}

int CameraBase::send_ptp_message(PTPContainer * cmd) {
    return this->send_ptp_message(cmd, 0);
}

void CameraBase::recv_ptp_message(PTPContainer *out, int timeout) {
    // Determine size we need to read
    unsigned char buffer[512];
    int read;
    this->_bulk_read((unsigned char *)buffer, 512, &read, timeout); // TODO: Error checking on response
    uint32_t size;
    memcpy(&size, buffer, 4);   // The first four bytes of the buffer are the size
    
    // Copy our first part into the output buffer -- so we can reuse buffer
    unsigned char * out_buf = (unsigned char *)malloc(size);
    if(size < 512) {
        memcpy(out_buf, buffer, size);
    } else {
        memcpy(out_buf, buffer, 512);
    }
    
    if(size > 512) {    // We've already read 512 bytes
        this->_bulk_read(&out_buf[512], size-512, &read, timeout);
    }
    
    if(out != NULL) {
        out->unpack(out_buf);
    }
    
    free(out_buf);
}

void CameraBase::recv_ptp_message(PTPContainer * out) {
    return this->recv_ptp_message(out, 0);
}

void CameraBase::ptp_transaction(PTPContainer *cmd, PTPContainer *data, bool receiving, PTPContainer * out_resp, PTPContainer * out_data, int timeout) {
    bool received_data = false;
    bool received_resp = false;

    cmd->transaction_id = this->get_and_increment_transaction_id();
    this->send_ptp_message(cmd, timeout);
    
    if(data != NULL) {
        data->transaction_id = cmd->transaction_id;
        this->send_ptp_message(data, timeout);
    }
    
    if(receiving) {
		PTPContainer out;
        this->recv_ptp_message(&out, timeout);
        if(out.type == PTP_CONTAINER_TYPE_DATA) {
            received_data = true;
			// TODO: It occurs to me that pack() and unpack() might be inefficient. Let's try to find a better way to do this.
			if(out_data != NULL) {
				out_data->unpack(out.pack());
			}
        } else if(out.type == PTP_CONTAINER_TYPE_RESPONSE) {
            received_resp = true;
			if(out_resp != NULL) {
				out_resp->unpack(out.pack());
			}
        }
    }
    
    if(!received_resp) {
        // Read it anyway!
        // TODO: We should return response AND data...
        this->recv_ptp_message(out_resp, timeout);
    }
}

void CameraBase::ptp_transaction(PTPContainer *cmd, PTPContainer *data, bool receiving, PTPContainer * out_resp, PTPContainer * out_data) {
    return this->ptp_transaction(cmd, data, receiving, out_resp, out_data, 0);
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

float CHDKCamera::get_chdk_version(void) {
	PTPContainer cmd(PTP_CONTAINER_TYPE_COMMAND, 0x9999);
	cmd.add_param(CHDK_OP_VERSION);
	
	PTPContainer out_resp;
	this->ptp_transaction(&cmd, NULL, false, &out_resp, NULL);
	// param 1 is four bytes of major version
	// param 2 is four bytes of minor version
	float out;
	unsigned char * payload;
	int payload_size;
	uint32_t major = 0, minor = 0;
	payload = out_resp.get_payload(&payload_size);
	if(payload_size >= 8) { // Need at least 8 bytes in the payload
		memcpy(&major, payload, 4);	// Copy first four bytes into major
		memcpy(&minor, payload+4, 4);	// Copy next four bytes into minor
	}
    
	out = major + minor/10.0;   // This assumes that the minor version is one digit long
	return out;
}

uint32_t CHDKCamera::check_script_status(void) {
    PTPContainer cmd(PTP_CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(CHDK_OP_SCRIPT_STATUS);
    
    PTPContainer out_resp;
    this->ptp_transaction(&cmd, NULL, false, &out_resp, NULL);
    
    uint32_t out = -1;
    unsigned char * payload;
    int payload_size;
    payload = out_resp.get_payload(&payload_size);
    if(payload_size >= 4) { // Need at least 4 bytes in the payload
        memcpy(&out, payload, 4);
    }
    
    return out;
}

uint32_t CHDKCamera::execute_lua(char * script, uint32_t * script_error, bool block) {
    PTPContainer cmd(PTP_CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(CHDK_OP_EXECUTE_SCRIPT);
    cmd.add_param(CHDK_LANGUAGE_LUA);
    
    PTPContainer data(PTP_CONTAINER_TYPE_DATA, 0x9999);
    data.set_payload((unsigned char *)script, strlen(script)+1);
    
    PTPContainer out_resp;
    this->ptp_transaction(&cmd, &data, false, &out_resp, NULL);
    
    uint32_t out = -1;
    unsigned char * payload;
    int payload_size;
    payload = out_resp.get_payload(&payload_size);
    
    if(block) {
        printf("TODO: Blocking code");
    } else {
        if(payload_size >= 8) { // Need at least 8 bytes in the payload
            memcpy(&out, payload, 4);
            if(script_error != NULL) {
                memcpy(script_error, payload+4, 4);
            }
        }
    }
    
    return out;
}

uint32_t CHDKCamera::execute_lua(char * script, uint32_t * script_error) {
    return this->execute_lua(script, script_error, false);
}

void CHDKCamera::read_script_message(PTPContainer * out_resp, PTPContainer * out_data) {
    PTPContainer cmd(PTP_CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(CHDK_OP_READ_SCRIPT_MSG);
    cmd.add_param(CHDK_LANGUAGE_LUA);
    
    this->ptp_transaction(&cmd, NULL, true, out_resp, out_data);
    // We'll just let the caller deal with the data
}

uint32_t CHDKCamera::write_script_message(char * message, uint32_t script_id) {
    PTPContainer cmd(PTP_CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(CHDK_OP_WRITE_SCRIPT_MSG);
    cmd.add_param(script_id);
    
    PTPContainer data(PTP_CONTAINER_TYPE_DATA, 0x9999);
    data.set_payload((unsigned char *)message, strlen(message)+1);
    
    PTPContainer out_resp;
    this->ptp_transaction(&cmd, &data, false, &out_resp, NULL);
    
    uint32_t out = -1;
    unsigned char * payload;
    int payload_size;
    payload = out_resp.get_payload(&payload_size);
    
    if(payload_size >= 4) { // Need four bytes of uint32_t response
        memcpy(&out, payload, 4);
    }
    
    return out;
}

// TODO: This function will need a *lot* of work.  For now, just a simple funciton to get live view. Other flags
//  will essentially be ignored.  Eventually, let's write a CHDKLiveData class which can contain the output and
//  provide useful functions to access it.
void CHDKCamera::get_live_view_data(LVData * data_out, bool liveview, bool overlay, bool palette) {
    uint32_t flags = 0;
    if(liveview) flags |= LV_TFR_VIEWPORT;
    if(overlay)  flags |= LV_TFR_BITMAP;
    if(palette)  flags |= LV_TFR_PALETTE;
    
    PTPContainer cmd(PTP_CONTAINER_TYPE_COMMAND, 0x9999);
    cmd.add_param(CHDK_OP_GET_DISPLAY_DATA);
    cmd.add_param(flags);
    
    PTPContainer out_resp, out_data;
    this->ptp_transaction(&cmd, NULL, true, &out_resp, &out_data);
    
    int payload_size;
    unsigned char * payload = out_data.get_payload(&payload_size);
    
    data_out->read(payload, payload_size);    // The LVData class will completely handle the LV data
    
    /*
    // Live view data parsing test:
    lv_data_header lv_head;
    int payload_size;
    unsigned char * payload = out_data.get_payload(&payload_size);
    memcpy(&lv_head, payload, sizeof(lv_data_header));
    
    lv_framebuffer_desc vp_head;
    memcpy(&vp_head, payload + lv_head.vp_desc_start, sizeof(lv_framebuffer_desc));
    
    unsigned char * vp_data;
    *vp_size = (vp_head.buffer_width*vp_head.visible_height*6)/4;
    vp_data = (unsigned char *)malloc(*vp_size);
    memcpy(vp_data, payload + vp_head.data_start, *vp_size);
    
    
    
    // Convert colorspace:
    //int width = vp_head.visible_width;
    //int height = vp_head.visible_height;
    // TODO: Sort out inconsistencies in the width and height
    // TODO: Probably make a new class to handle live view data
    *width = 360;
    *height = 480;
    int outLen = vp_head.visible_width * vp_head.visible_height;
    uint8_t * out = (uint8_t *) malloc(outLen*3);
    
    return out;
    */
}

uint32_t CHDKCamera::write_script_message(char * message) {
    return this->write_script_message(message, 0);
}

bool CameraBase::open(libusb_device * dev) {
    if(this->handle != NULL) {  // Handle will be non-null if the device is already open
        throw LIBPTP2_ALREADY_OPEN;
        return false;
    }

    int err = libusb_open(dev, &(this->handle));    // Open the device, placing the handle in this->handle
    if(err) {
        throw LIBPTP2_CANNOT_CONNECT;
        return false;
    }
    libusb_unref_device(dev);   // We needed this device refed before we opened it, so we added an extra ref. open adds another ref, so remove one ref
    
    struct libusb_config_descriptor * desc;
    int r = libusb_get_active_config_descriptor(dev, &desc);
    
    if (r < 0) {
        this->usb_error = r;
        return false;
    }
    
    int j, k;
    
    for(j = 0; j < desc->bNumInterfaces; j++) {
        struct libusb_interface interface = desc->interface[j];
        for(k = 0; k < interface.num_altsetting; k++) {
            struct libusb_interface_descriptor altsetting = interface.altsetting[k];
            if(altsetting.bInterfaceClass == 6) { // If this has the PTP interface
                this->intf = &altsetting;
                libusb_claim_interface(this->handle, this->intf->bInterfaceNumber); // Claim the interface -- Needs to be done before I/O operations
                break;
            }
        }
        if(this->intf) break;
    }
    
    
    const struct libusb_endpoint_descriptor * endpoint;
    for(j = 0; j < this->intf->bNumEndpoints; j++) {
        endpoint = &(this->intf->endpoint[j]);
        if((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK == LIBUSB_ENDPOINT_IN) &&
            (endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK) {
            this->ep_in = endpoint->bEndpointAddress;
        } else if((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
            this->ep_out = endpoint->bEndpointAddress;
        }
    }
    
    libusb_free_config_descriptor(desc);
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
    
    if(found) {
        libusb_ref_device(found);     // Add a reference to the device so it doesn't get destroyed when we free_device_list
    }
    
    libusb_free_device_list(list, 1);   // Free the device list with dereferencing. Shouldn't delete our device, since we ref'd it
    
    return found;
}

int CameraBase::get_usb_error() {
    return this->usb_error;
}

int CameraBase::get_and_increment_transaction_id() {
    uint32_t ret = this->_transaction_id;
    this->_transaction_id = this->_transaction_id + 1;
    return ret;
}

void PTPContainer::init() {
    this->length = this->default_length; // Length is at least the sum of the header parts
    this->payload = NULL;
}

PTPContainer::PTPContainer() {
    // Not sure what I want to do here
    this->init();
}

PTPContainer::PTPContainer(uint16_t type, uint16_t op_code) {
    this->init();
    this->type = type;
    this->code = op_code;
}

PTPContainer::PTPContainer(unsigned char * data) {
    // This is essentially lv_framebuffer_desca .pack() function, in the form of a constructor
    this->unpack(data);
}

PTPContainer::~PTPContainer() {
    if(this->payload != NULL) {
        free(this->payload);    // Be sure to free up this memory
        this->payload = NULL;
    }
}

void PTPContainer::add_param(uint32_t param) {
    // Allocate new memory for the payload
    uint32_t old_length = (this->length)-(this->default_length);
    uint32_t new_length = this->length + sizeof(uint32_t);
    unsigned char * new_payload = (unsigned char *)malloc(new_length);
    
    // Copy old payload into new payload
    memcpy(new_payload, this->payload, old_length);
    // Copy new data into new payload
    memcpy(&(new_payload[old_length]), &param, sizeof(uint32_t));
    // Free up old payload memory
    free(this->payload);
    // Change payload pointer to new payload
    this->payload = new_payload;
    // Update length
    this->length = new_length;
}

void PTPContainer::set_payload(unsigned char * payload, int payload_length) {
    // Allocate new memory to copy the payload into
    // This way, we can ensure that we always want to free() the memory
    uint32_t new_length = this->default_length + payload_length;
    unsigned char * new_payload = (unsigned char *)malloc(payload_length);
    
    // Copy the payload over
    memcpy(new_payload, payload, payload_length);
    // Free up the old payload
    free(this->payload);
    // Change payload pointer to new payload
    this->payload = new_payload;
    // Update length
    this->length = new_length;
}

unsigned char * PTPContainer::pack() {
    unsigned char * packed = (unsigned char *)malloc(this->length);
    
    uint32_t header_size = (sizeof this->length)+(sizeof this->type)+(sizeof this->code)+(sizeof this->transaction_id);
    
    memcpy(&(packed[0]), &(this->length), sizeof this->length);  // Copy length
    memcpy(&(packed[4]), &(this->type), sizeof this->type);    // Type
    memcpy(&(packed[6]), &(this->code), sizeof this->code);    // Two bytes of code
    memcpy(&(packed[8]), &(this->transaction_id), sizeof this->transaction_id);    // Four bytes of transaction id
    memcpy(&(packed[12]), this->payload, this->length-header_size);    // The rest of payload
    
    return packed;
}

unsigned char * PTPContainer::get_payload(int * size_out) {
    *size_out = this->length - this->default_length;
    return payload;
}

uint32_t PTPContainer::get_length() {
    return length;
}

void PTPContainer::unpack(unsigned char * data) {
    // First four bytes are the length
    memcpy(&this->length, data, 4);
    // Next, container type
    memcpy(&this->type, &(data[4]), 2);
    // Copy over code
    memcpy(&this->code, &(data[6]), 2);
    // And transaction ID...
    memcpy(&this->transaction_id, &(data[8]), 4);
    
    // Finally, copy over the payload
    this->payload = (unsigned char *)malloc(this->length-12);
    memcpy(this->payload, &(data[12]), this->length-12);
    
    // Since we copied all of this data, the data passed in can be free()d
}

LVData::LVData() {
    this->init();
}

LVData::LVData(uint8_t * payload, int payload_size) {
    this->init();
    this->read(payload, payload_size);
}

LVData::~LVData() {
    free(this->vp_head);
    free(this->fb_desc);
    free(this->payload);
}

void LVData::init() {
    this->vp_head = (lv_data_header *)malloc(sizeof(lv_data_header));
    this->fb_desc = (lv_framebuffer_desc *)malloc(sizeof(lv_framebuffer_desc));
    this->payload = NULL;
}

void LVData::read(uint8_t * payload, int payload_size) {
    if(this->payload != NULL) {
        free(this->payload); // Free up the payload if we're overwriting this object
        this->payload = NULL;
    }
    
    this->payload = (uint8_t *)malloc(payload_size);
    
    memcpy(this->payload, payload, payload_size);   // Copy the payload we're reading in into OUR payload
    
    // Parse the payload data into vp_head and fb_desc
    memcpy(this->vp_head, this->payload, sizeof(lv_data_header));
    memcpy(this->fb_desc, this->payload+this->vp_head->vp_desc_start, sizeof(lv_framebuffer_desc));
}

// TODO: get_rgb function
uint8_t * LVData::get_rgb(int * out_size, int * out_width, int * out_height, bool skip) {
    uint8_t * vp_data;
    int vp_size;
    vp_size = (this->fb_desc->buffer_width * this->fb_desc->visible_height * 12) / 8; // 12 bpp
    vp_data = (uint8_t *)malloc(vp_size);   // Allocate memory for YUV data
    memcpy(vp_data, this->payload + this->fb_desc->data_start, vp_size);    // Copy YUV data into vp_data
    
    int par = skip?2:1; // If skip, par = 2 ; else, par = 1
    
    *out_width = this->fb_desc->visible_width / par;           // Vertical width of output
    unsigned int dispsize = *out_width * (this->fb_desc->visible_height);   // Size of output
    *out_size = dispsize*3;                                             // RGB output size
    
    uint8_t * out = (uint8_t *)malloc(*out_size);  // Allocate space for RGB output
    
    uint8_t * prgb_data = out; // Pointer we can manipulate to transverse RGB output memory
    uint8_t * p_yuv = vp_data; // Pointer we can manipulate to transverse YUV input memory
    
    int i;
    // Transverse input and output. For each four RGB pixels, we increment 6 YUV bytes
    //  See: http://chdk.wikia.com/wiki/Frame_buffers#Viewport
    // This magical code borrowed from http://trac.assembla.com/chdk/browser/trunk/tools/yuvconvert.c
    for(i = 0; i < (this->fb_desc->buffer_width * this->fb_desc->visible_height); i += 4, p_yuv += 6) {
        this->yuv_to_rgb(&prgb_data, p_yuv[1], p_yuv[0], p_yuv[2]);
        this->yuv_to_rgb(&prgb_data, p_yuv[3], p_yuv[0], p_yuv[2]);
        
        if(skip) continue;  // If we skip two, go to the next iteration
        
        this->yuv_to_rgb(&prgb_data, p_yuv[4], p_yuv[0], p_yuv[2]);
        this->yuv_to_rgb(&prgb_data, p_yuv[5], p_yuv[0], p_yuv[2]);
    }
    
    *out_height = this->fb_desc->visible_height;
    
    free(vp_data);  // We don't need this anymore
    
    return out;     // It's up to the caller to free() this when done
}

uint8_t LVData::clip(int v) {
    if (v<0) return 0;
    if (v>255) return 255;
    return v;
}

void LVData::yuv_to_rgb(uint8_t **dest, uint8_t y, int8_t u, int8_t v) {
    *((*dest)++) = LVData::clip(((y<<12) +          v*5743 + 2048)>>12);
    *((*dest)++) = LVData::clip(((y<<12) - u*1411 - v*2925 + 2048)>>12);
    *((*dest)++) = LVData::clip(((y<<12) + u*7258          + 2048)>>12);
}

float LVData::get_lv_version() {
    if(this->vp_head == NULL) return -1;
    
    return this->vp_head->version_major + this->vp_head->version_minor / 10.0;
}
