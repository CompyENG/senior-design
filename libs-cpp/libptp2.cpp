// A conversion of pyptp2 and all that comes with it
//  Targeted at libusb version 1.0, since that's recommended
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libusb-1.0/libusb.h>

#include "libptp2.hpp"

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

int CameraBase::_bulk_read(unsigned char * data_out, int size, int timeout) {
    int transferred;
    
    if(this->handle == NULL) {
        throw LIBPTP2_NOT_OPEN;
        return 0;
    }
    
    // TODO: Return the amount of data transferred? We might get less than we ask for, which means we need to tell the calling function?
    return libusb_bulk_transfer(this->handle, this->ep_in, data_out, size, &transferred, timeout);
}

int CameraBase::_bulk_read(unsigned char * data_out, int size) {
    return this->_bulk_read(data_out, size, 0);
}

int CameraBase::send_ptp_message(unsigned char * data, int size, int timeout) {
    return this->_bulk_write(data, size, timeout);
}

int CameraBase::send_ptp_message(unsigned char * data, int size) {
    return this->send_ptp_message(data, size, 0);
}

int CameraBase::send_ptp_message(PTPContainer cmd, int timeout) {
    return this->send_ptp_message(cmd.pack(), cmd.get_length(), timeout);
}

int CameraBase::send_ptp_message(PTPContainer cmd) {
    return this->send_ptp_message(cmd, 0);
}

PTPContainer CameraBase::recv_ptp_message(int timeout) {
    // Determine size we need to read
    unsigned char buffer[512];
    this->_bulk_read((unsigned char *)buffer, 512, timeout); // TODO: Error checking on response
    uint32_t size;
    memcpy(&size, buffer, 4);   // The first four bytes of the buffer are the size
    
    // Copy our first part into the output buffer -- so we can reuse buffer
    unsigned char * out_buf = (unsigned char *)malloc(size);
    memcpy(out_buf, buffer, 512);
    
    if(size > 512) {    // We've already read 512 bytes
        this->_bulk_read((unsigned char *)buffer, size-512, timeout);
        memcpy(&out_buf[512], buffer, size-512);    // Copy the rest in
    }
    
    PTPContainer ret(out_buf);
    
    free(out_buf);
    
    return ret;
}

PTPContainer CameraBase::recv_ptp_message() {
    return this->recv_ptp_message(0);
}

PTPContainer CameraBase::ptp_transaction(PTPContainer * cmd, PTPContainer * data, bool receiving, int timeout) {
    // Send command
    this->send_ptp_message(*cmd, timeout);
    
    // If we have data
    if(data != NULL) {
        this->send_ptp_message(*data, timeout);
    }
    
    if(receiving) {
        return this->recv_ptp_message(timeout);
    } else {
        return NULL;
    }
}

PTPContainer CameraBase::ptp_transaction(PTPContainer *cmd, PTPContainer *data, bool receiving) {
    return this->ptp_transaction(cmd, data, receiving, 0);
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
    return (this->_transaction_id++);
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
    // This is essentially a .pack() function, in the form of a constructor
    
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

PTPContainer::~PTPContainer() {
    free(this->payload);    // Be sure to free up this memory
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

