# -*- coding: utf-8 -*-
import struct


class PtpRequest:
    """Class encapsulating a PTP Request."""

    def __init__(self, opcode, sessionid, transactionid, params=None):
        self.opcode = opcode
        self.sessionid = sessionid
        self.transactionid = transactionid
        self.params = () if params == None else params
        
    def __str__(self):
        tmp = "Opcode:0x%04x\nSessionId:0x%04x\nTransactionId:0x%04x\n" % (self.opcode, self.sessionid, self.transactionid)
        for p in self.params:
            tmp += repr(p) + "\n"
        return tmp
        

class PtpResponse:
    """Class encapsulating a PTP Response."""

    def __init__(self, respcode, sessionid, transactionid, params=None):
        self.respcode = respcode
        self.sessionid = sessionid
        self.transactionid = transactionid
        self.params = () if params == None else params

    def __str__(self):
        tmp = "Respcode:0x%04x\nSessionId:0x%04x\nTransactionId:0x%04x\n" % (self.respcode, self.sessionid, self.transactionid)
        for p in self.params:
            tmp += repr(p) + "\n"
        return tmp


class PtpEvent:
    """Class encapsulating a PTP Event."""

    def __init__(self, eventcode, sessionid, transactionid, params=None):
        self.eventcode = eventcode
        self.sessionid = sessionid
        self.transactionid = transactionid
        self.params = () if params == None else params

    def __str__(self):
        tmp = "Eventcode:0x%04x\nSessionId:0x%04x\nTransactionId:0x%04x\n" % (self.eventcode, self.sessionid, self.transactionid)
        for p in self.params:
            tmp += repr(p) + "\n"
        return tmp



class PtpAbstractTransport:
    """Class defining an abstract PTP transport."""

    def __init__(self):
        raise NotImplementedError('Cannot create an instance of PtpAbstractTransport')

    def NewSession(self):
        """Get a new session id for this transport.
        
        Returns: A new session ID."""
        if not hasattr(self, 'sessionid'):
            self.sessionid = 0
        self.sessionid += 1
        return self.sessionid

    def send_ptp_request(self, request):
        """Transport specific code to send a PtpRequest structure to a PTP device.
        
        Arguments:
        request --- A PtpRequest to send."""
        
        raise NotImplementedError('send_ptp_request not implemented')

    def send_ptp_data(self, request, data):
        """Transport specific code to send in-memory data to a PTP device.
        
        Arguments:
        request -- The PtpRequest.
        data -- String of data to send."""

        raise NotImplementedError('send_ptp_data not implemented')

    def get_ptp_data(self, request, stream = None):
        """Transport specific code to get data from a PTP device.
        
        Arguments:
        request -- The PtpRequest.
        stream -- A stream to which data should be written to if desired.

        Returns:
        A tuple of (data size, received data as string)
        
        Note: received data as string will be None if stream was supplied."""

        raise NotImplementedError('get_ptp_data not implemented')

    def get_ptp_response(self, request):
        """Transport specific code to get a PtpResponse from a PTP device.
        
        Arguments:
        request -- The PtpRequest.
        
        Returns:
        A PtpResponse object."""

        raise NotImplementedError('get_ptp_response not implemented')
    
    def check_ptp_event(self, sessionid, timeout=None):
        
        raise NotImplementedError('check_ptp_event not implemented')    

    def ptp_simple_transaction(self, request, tx_data=None, receiving=False):
        """Perform a simple PTP operation. 
        
        Arguments: 
        request -- A PTPRequest class instance
        tx_data -- Data to transmit, or None
        receiving -- Are we expecting to receive data?

        Returns:
        A tuple of (PTPResponse, received data as string)."""

        rx_data = None
        response = None
        
        self.send_ptp_request(request)
        if tx_data != None:
            self.send_ptp_data(request, tx_data)
        elif receiving:
            rx_data = self.get_ptp_data(request)
            if isinstance(rx_data, PtpResponse):
                response = rx_data
                rx_data = None
        
        if response == None:
            response = self.get_ptp_response(request)

        return (response, rx_data)
