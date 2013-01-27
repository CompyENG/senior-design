import logging
import usb
import struct
import time
from os import path

import util
from typedefs import *
from chdk_ptp_values import *
from ptp_values import StandardResponses

__all__ = ['PTPCamera', 'CHDKCamera']

class _CameraBase(object):

    def __init__(self, usb_device=None):


        self._intf      = None
        self._handle    = None

        self._ep_in     = None
        self._ep_out    = None
        self._ep_intr   = None       

        self._transaction_id = 0
        if usb_device is not None:
            self.open(usb_device)

    def __del__(self):
        self.close()

    def open(self, usb_device):
        
        intf = util.get_ptp_interface(usb_device)

        if intf is None:
            raise TypeError('USB Device %s not a PTP Camera' %(usb_device))

        self._intf = intf
        self._handle = usb_device

        #Grab endpoints
        for ep in self._intf:
            ep_type = usb.util.endpoint_type(ep.bmAttributes)
            ep_dir  = usb.util.endpoint_direction(ep.bEndpointAddress)

            if ep_type == usb.util.ENDPOINT_TYPE_BULK:
                if ep_dir == usb.util.ENDPOINT_IN:
                    self._ep_in = ep.bEndpointAddress

                elif ep_dir == usb.util.ENDPOINT_OUT:
                    self._ep_out = ep.bEndpointAddress

            elif ep_type == usb.util.ENDPOINT_TYPE_INTR:
                self._ep_intr = ep.bEndpointAddress

    def close(self):
        #Excplicity release usb device
        if self._handle is not None:
            usb.util.dispose_resources(self._handle)

        # _, self._handle = self._handle, None
        _, self._intf   = self._intf, None
        
        self._ep_in     = None  
        self._ep_out    = None
        self._ep_intr   = None


    def reopen(self):
        if self._handle is None:
            raise ValueError('No USB Device assigned.  (Did you open it first?)')

        if self._intf is not None:
            raise ValueError('Already open')

        self.open(self._handle)

    def _bulk_write(self, bytestr, timeout=0):

        return self._handle.write(self._ep_out, bytestr, interface=self._intf, timeout=timeout)

    def _bulk_read(self, size, timeout=0):
        return self._handle.read(self._ep_in, size, interface=self._intf, timeout=timeout).tostring()

    def send_ptp_message(self, bytestr, timeout=0):
        return self._bulk_write(bytestr, timeout)

    def recv_ptp_message(self, timeout=0):
        buf = self._bulk_read(size=512, timeout=timeout)
        msg_len = struct.unpack('<I', buf[:4])[0]
        bytes_left = msg_len - 512
        if bytes_left > 0:
            buf += self._bulk_read(size=bytes_left, timeout=timeout)

        return buf

    def new_ptp_command(self, op_code, params=[]):

        ptp_command = ParamContainer()
        ptp_command.type = PTP_CONTAINER_TYPE.COMMAND
        ptp_command.code = op_code
        ptp_command.transaction_id = self._transaction_id

        ptp_command.params = params

        self._transaction_id += 1
        return ptp_command


    def ptp_transaction(self, command, params=[], tx_data=None, receiving=True, timeout=0):

        recvd_data      = None
        recvd_response  = None

        ptp_request = self.new_ptp_command(command, params)
        ptp_request_data = None
        
        if tx_data is not None:
            assert isinstance(tx_data, str)
            
            ptp_request_data = DataContainer()
            ptp_request_data.code = ptp_request.code
            ptp_request_data.transaction_id = ptp_request.transaction_id
            ptp_request_data.data = tx_data


        #Send request
        bytes_xfrered = self.send_ptp_message(ptp_request.pack(), timeout)

        #Send data 
        if ptp_request_data is not None:
            bytes_xfered = self.send_ptp_message(ptp_request_data.pack(), timeout)

        if receiving:
            #read first 512 bytes to grab total data length
            buf = self.recv_ptp_message(timeout)
            _, type_ = struct.unpack('<IH', buf[:6])
        
            if type_ == PTP_CONTAINER_TYPE.DATA:
                recvd_data = DataContainer(buf)
            
            elif type_ == PTP_CONTAINER_TYPE.RESPONSE:
                recvd_response = ParamContainer(buf)

            elif type_ in [PTP_CONTAINER_TYPE.COMMAND, PTP_CONTAINER_TYPE.EVENT]:
                recvd_data = ParamContainer(buf)
            
            else:
                raise TypeError('Unknown PTP USB container type: %d' %(type_))

        #If we haven't got the response yet, try again
        if recvd_response is None:
            buf = self.recv_ptp_message(timeout=timeout)
            _, type_ = struct.unpack('<IH', buf[:6])
            
            if type_ == PTP_CONTAINER_TYPE.RESPONSE:
                recvd_response = ParamContainer(buf)
            
            else:
                raise TypeError('Expected response container, received type: %d' %(type_))

        return recvd_response, recvd_data

class PTPCamera(_CameraBase):
    '''
    '''

    def __init__(self, usb_dev=None):
        # _CameraBase.__init__(self, usb_dev)
        raise NotImplimentedError('Work in progress')


class CHDKCamera(_CameraBase):
    '''
    For use with Canon cameras using the CHDK firmware.

    Available functions (see docstrings for info):
        get_chdk_version
        upload_file
        download_file
        get_live_view_data
        execute_lua
        read_script_message
        write_script_message

    '''

    def __init__(self, usb_device=None):
        _CameraBase.__init__(self, usb_device)


    def get_chdk_version(self):
        '''
        Retrieves the PTP-core (MAJOR,MINOR) version tuple from the
        camera.

        Note:  This is different than the (MAJOR,MINOR) version tuple
        for the live_view PTP extensions. 
        '''
        recvd_response, _ = self.ptp_transaction(command=PTP_OC_CHDK,
            params=[CHDKOperations.Version],
            tx_data=None, receiving=False, timeout=0)

        major, minor = recvd_response.params
        return major, minor

    def check_script_status(self):
        '''
        :returns: CHDKScriptStatus

        Check status of running scripts on camera
        '''
        recvd_response, _ = self.ptp_transaction(command=PTP_OC_CHDK,
            params=[CHDKOperations.ScriptStatus],
            tx_data=None, receiving=False, timeout=0)

        status = recvd_response.params[0]
        return status

    def execute_lua(self, script, block=False):
        '''
        :param script: LUA script to execute on camera
        :type script: str

        :param block:  Wait for script to return before continuing
        :type block: bool

        :returns: (script_id, script_error, [msgs])

        Execute a script on the camera.

        Values returned by the LUA script are passed in individual
        messages.
        '''
        #NULL terminate script if necessary
        if not script.endswith('\0'):
            script += '\0'

        recvd_response, _ = self.ptp_transaction(command=PTP_OC_CHDK,
            params=[CHDKOperations.ExecuteScript, CHDKScriptLanguage.LUA],
            tx_data=script, receiving=False, timeout=0)

        script_id, script_error = recvd_response.params
        if not block:
            return script_id, script_error, []
        
        else:
            msgs = self._wait_for_script_return()
            return script_id, script_error, msgs

    def read_script_message(self):
        '''
        Checks camera for messages created by running scripts.
        '''
        recvd_response, recvd_data = self.ptp_transaction(command=PTP_OC_CHDK,
            params=[CHDKOperations.ReadScriptMsg, CHDKScriptLanguage.LUA],
            tx_data=None, receiving=True, timeout=0)

        return recvd_response, recvd_data

    def write_script_message(self, message, script_id=0):
        '''
        :param message: Message to send
        :type message: str

        :param script_id:  ID of script to deliver message to.
        :type script_id: int

        Passes a message to a running script.
        '''
        recvd_response, _ = self.ptp_transaction(command=PTP_OC_CHDK,
            params=[CHDKOperations.WriteScriptMsg, script_id],
            tx_data=message, receiving=False, timeout=0)

        msg_status = recvd_response.params[0]
        return msg_status

    @classmethod 
    def __pack_file_for_upload(cls, local_filename, remote_filename=None):
        '''
        Private method to create a buffer holding
        filename's contents for uploading to the camera.
        called in `CHDKCamera.upload_file'
        '''
        if remote_filename is None:
            remote_filename = path.basename(remote_filename)

        if not remote_filename.endswith('\0'):
            remote_filename += '\0'

        filename_len = len(remote_filename)
        fmt = '<I%dc' %(filename_len)
        filebuf = struct.pack(fmt, filename_len, remote_filename)
        with open(local_filename, 'rb') as fid:
            contents = fid.read(-1)

        fmt = '<%dB' % (len(contents))
        filebuf += struct.pack(fmt, *contents)

        return filebuf

    def upload_file(self, local_filename, remote_filename=None, timeout=0):
        '''
        :param local_filename:  Name of file on computer
        :type local_filename: str


        :param remote_filename: Name of file on camera
        :type remote_filename: str

        Upload a file to the camera.  If remote_filename is None, the
        file is uploaded to the root folder on the SD card.
        '''
        filestr = self.__pack_file_for_upload(local_filename, remote_filename)
        dlfile_response, dlfile_data = self.ptp_transaction(command=PTP_OC_CHDK,
            params=[CHDKOperations.UploadFile],
            tx_data=filestr, receiving=False, timeout=timeout)

        if ret_code != CHDKResponses.OK:
            raise PTPError(tempdata_response.params[0], CHDKResponses.message[ret_code])


    def download_file(self, filename, timeout=0):
        '''
        :param filename: Full path of file to download
        :type filename:  str

        Download a file from the camera
        '''
        #CHDK Download process:
        #  - Store desried filename on camera w/ TempData
        #  - Send DownloadFile command

        if not filename.endswith('\0'):
            filename += '\0'

        tempdata_response, _ = self.ptp_transaction(command=PTP_OC_CHDK,
            params=[CHDKOperations.TempData, 0],
            tx_data=filename, receiving=False, timeout=timeout)

        ret_code = tempdata_response.params[0]
        #check response for problems
        if ret_code != CHDKResponses.OK:
            raise PTPError(tempdata_response.params[0], CHDKResponses.message[ret_code])

        dlfile_response, dlfile_data = self.ptp_transaction(command=PTP_OC_CHDK,
            params=[CHDKOperations.DownloadFile],
            tx_data=None, receiving=True, timeout=timeout)

        ret_code = tempdata_response.params[0]
        #check response for problems
        if ret_code != CHDKResponses.OK:
            raise PTPError(tempdata_response.params[0], CHDKResponses.message[ret_code])

        #Clear tempdata field
        clear_response, _ = self.ptp_transaction(command=PTP_OC_CHDK,
            params=[CHDKOperations.TempData, CHDKTempData.CLEAR],
            tx_data=None, receiving=False, timeout=timeout)        
        
        #Return the raw string buffer
        return dlfile_data.data

    def get_live_view_data(self, liveview=True, overlay=False, palette=False):
        '''
        :param liveview:  Return the liveview image
        :type liveview: bool

        :param overlay:  Return the overlay image
        :type overlay: bool

        :param palette:  Return the overlay palette
        :type palette: bool

        :returns: :class:`typdefs.CHDK_LV_Data`
        
        Grabs a live view image from the camera.
        '''
        flags = 0
        
        if liveview:
            flags |= CHDKLVTransfer.VIEWPORT

        if overlay:
            flags |= CHDKLVTransfer.BITMAP

        if palette:
            flags |= CHDKLVTransfer.PALETTE

        recvd_response, recvd_data = self.ptp_transaction(command=PTP_OC_CHDK,
            params=[CHDKOperations.GetDisplayData, flags],
            tx_data=None, receiving=True, timeout=0)

        if recvd_data.type == PTP_CONTAINER_TYPE.DATA:
            lv_data = CHDK_LV_Data(recvd_data.data)

        else:
            lv_data = None

        return recvd_response, lv_data

    def _wait_for_script_return(self, timeout=0):
        '''
        Polls the camera every 50ms.

        Reads queued messages if present, sleeps again if
        a script is currently running.

        Returns read messages when no scripts are running.
        '''
        msg_count = 1
        msgs = []
        t_start = time.time()

        while True:
            STATUS = self.check_script_status()

            if STATUS & CHDKScriptStatus.RUN:
                # log.debug('Script running, sleeping 50ms')
                time.sleep(50e-3)
                if timeout > 0 and timeout > (time.time() - t_start):
                    raise PTPError(StandardResponses.TRANSACTION_CANCELLED, "Timeout waiting for script to return")
            
            elif STATUS & CHDKScriptStatus.MSG:
                msg, msg_buf = self.read_script_message()
                msg_count += 1

                msgs.append((msg, msg_buf))
            
            elif STATUS == CHDKScriptStatus.NONE:
                break
            
            else:
                raise PTPError(StandardResponses.UNDEFINED, "Invalid response for script status: 0x%X" %(STATUS))

        return msgs
