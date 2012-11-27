# -*- coding: utf-8 -*-
import PtpAbstractTransport
import usb
import struct


class PtpUsbTransport(PtpAbstractTransport.PtpAbstractTransport):
    """Class defining a PTP USB transport."""
    
    USB_CLASS_PTP                           = 6
    
    PTP_USB_CONTAINER_COMMAND               = 1
    PTP_USB_CONTAINER_DATA                  = 2
    PTP_USB_CONTAINER_RESPONSE              = 3
    PTP_USB_CONTAINER_EVENT                 = 4

    def __init__(self, device):
        """Create a new PtpUsbTransport instance.
        
        Arguments:
        device -- USB ptp device."""
                
        # Check the device is a PTP one
        if (device.configurations[0].interfaces[0][0].interfaceClass != self.USB_CLASS_PTP):
            raise UsbException("Supplied USB device is not a PTP device")
        if (device.deviceClass == usb.CLASS_HUB):
            raise UsbException("Supplied USB device is not a PTP device")
        
        # Find the endpoints we need
        self.__bulkin = None
        self.__bulkout = None
        self.__irqin = None
        for ep in device.configurations[0].interfaces[0][0].endpoints:
            if ep.type == usb.ENDPOINT_TYPE_BULK:
                if (ep.address & usb.ENDPOINT_DIR_MASK) == usb.ENDPOINT_IN:
                    self.__bulkin = ep.address
                elif (ep.address & usb.ENDPOINT_DIR_MASK) == usb.ENDPOINT_OUT:
                    self.__bulkout = ep.address
            elif ep.type == usb.ENDPOINT_TYPE_INTERRUPT:
                self.__irqin = ep.address
                
        if  (self.__bulkin == None) or (self.__bulkout == None) or (self.__irqin == None):
            raise RuntimeError("Unable to find all required endpoints")

        # Open the USB device
        self.__usb_interface = device.configurations[0].interfaces[0][0]
        self.__usb_handle = device.open()
        self.__usb_handle.setConfiguration(device.configurations[0])
        self.__usb_handle.claimInterface(device.configurations[0].interfaces[0][0])
        self.usb_read_timeout = 5000
        self.usb_write_timeout = 5000

        
    def __del__(self):
        """Cleanup a PtpUsbTransport structure."""
        
        try:
            self.__usb_handle.releaseInterface(self.__usb_interface)
            del self.__usb_handle
        except:
            pass

    
    def send_ptp_request(self, request):
        length = 12 + (len(request.params) * 4)
        buffer = struct.pack("<IHHI", length, self.PTP_USB_CONTAINER_COMMAND, request.opcode, request.transactionid)
        for p in request.params:
            buffer += struct.pack("<I", p)
            
        if self.__usb_handle.bulkWrite(self.__bulkout, buffer, self.usb_write_timeout) != length:
            raise UsbException(usb.USBError)

    
    def send_ptp_data(self, request, data):
        buffer = struct.pack("<IHHI", 12 + len(data), self.PTP_USB_CONTAINER_DATA, request.opcode, request.transactionid)
        buffer += data

        # NOTE: UNTESTED CODE
        while len(buffer) > 0:
            tmp = self.__usb_handle.bulkWrite(self.__bulkout, buffer, self.usb_write_timeout)
            if tmp == 0:
                raise UsbException(usb.USBError)

            buffer = buffer[tmp:]


    def get_ptp_data(self, request, stream = None):
        # Get the header
        pkt = self.__usb_bulkread()
        (data_size, container_type, code, transactionid) = struct.unpack("<IHHI", pkt[0:12])
        
        # Make sure transactionid is correct
        if transactionid != request.transactionid:
            raise UsbException("Received unexpected PTP USB transactionid")
        
        # Handle the possibility of receiving a RESPONSE instead of data (e.g. on error condition)
        if container_type == self.PTP_USB_CONTAINER_RESPONSE:
            return self.__decode_ptp_response(request, pkt)
            # FIXME
        elif container_type != self.PTP_USB_CONTAINER_DATA:
            raise UsbException("Received unexpected PTP USB container type (%i)" % container_type)
        
        # Make sure it is sane (paranoia mode++)
        if code != request.opcode:
            raise UsbException("Received unexpected PTP USB opcode")
        if transactionid != request.transactionid:
            raise UsbException("Received unexpected PTP USB opcode")
        data_size -= 12
        
        # Deal with the piece of the data body we've already read
        toread = len(pkt) - 12
        if toread > data_size: 
            toread = data_size
            
        buffer = None
        if (stream == None):
            buffer = pkt[12:12+toread]
        else:
            stream.write(pkt[12:12+toread])
        done = toread

        # Read the rest of the data
        while done != data_size:
            pkt = self.__usb_bulkread()

            toread = len(pkt)
            if toread > (data_size - done):
                toread = data_size - done
                
            if stream == None:
                buffer += pkt[0:toread]
            else:
                stream.write(pkt[0:toread])
            done += toread

        return (data_size, buffer)


    def get_ptp_response(self, request):
        return self.__decode_ptp_response(request, self.__usb_bulkread())
    
    def check_ptp_event(self, sessionid, timeout=None):
        if timeout == None:
            timeout = 0
        
        # Read the packet
        pkt = self.__usb_bulkread(timeout = timeout, ep=self.__irqin)
        (data_size, container_type, code) = struct.unpack("<IHH", pkt[0:8])

        # Make sure it is sane (paranoia mode++)
        if container_type != self.PTP_USB_CONTAINER_EVENT:
            raise UsbException("Received unexpected PTP USB container type (0x%04x)" % container_type)

        # Extract the body
        pkt = pkt[8:]
        param_count = (data_size - 12) / 4

        # Parse it
        (transactionid, ) = struct.unpack("<I", pkt[0:4])
        params = struct.unpack("<" + ("I" * param_count), pkt[4:])

        return PtpAbstractTransport.PtpEvent(code, sessionid, transactionid, params)

    def __decode_ptp_response(self, request, pkt):
        # Read the packet
        (data_size, container_type, code, transactionid) = struct.unpack("<IHHI", pkt[0:12])

        # Make sure it is sane (paranoia mode++)
        if data_size > 32:
            raise UsbException("Received unexpected data size (%i) for PTP response" % data_size)            
        if container_type != self.PTP_USB_CONTAINER_RESPONSE:
            raise UsbException("Received unexpected PTP USB container type (%i)" % container_type)
        if transactionid != request.transactionid:
            raise UsbException("Received unexpected PTP USB transactionid (%i != %i)" % (transactionid, request.transactionid))

        param_count = (data_size - 12) / 4
        params = struct.unpack("<" + ("I" * param_count), pkt[12: 12 + (4*param_count)])
        return PtpAbstractTransport.PtpResponse(code, request.sessionid, transactionid, params)
    

    def __usb_bulkread(self, urb_size=512, timeout=None, ep=None):
        if timeout == None:
            timeout = self.usb_read_timeout
        if ep == None:
            ep = self.__bulkin

        tmp = ''.join([chr(x) for x in self.__usb_handle.bulkRead(ep, urb_size, timeout)])
        if len(tmp) == 0:
            # Retry...
            tmp = ''.join([chr(x) for x in self.__usb_handle.bulkRead(ep, urb_size, timeout)])
        return tmp
    
    def __hexdump(self, data):
        print
        for b in data:
            print hex(ord(b))

    @classmethod
    def findptps(cls):
        ptps = ()

        for bus in usb.busses():
            for device in bus.devices:
                if (device.configurations[0].interfaces[0][0].interfaceClass != PtpUsbTransport.USB_CLASS_PTP):
                    continue
                if (device.deviceClass == usb.CLASS_HUB):
                    continue
                ptps += (device, )

        return ptps

class UsbException(Exception):

    def __init__(self, value):
        self.value = value

    def __str__(self):
        return "USBException(%s)" % repr(self.value)
