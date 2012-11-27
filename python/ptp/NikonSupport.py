# -*- coding: utf-8 -*-
import PtpSession
import PtpValues
import PtpAbstractTransport
import struct


# Special objectid for an item in the camera's RAM buffer
OBJECTID_RAM = 0xffff0001

class NikonCurveInfo:
    
    def __init__(self, raw):
        unpacker = PtpSession.PtpUnpacker(raw)
        
        (data_length, unknown, self.xaxis_start, self.xaxis_end, self.yaxis_start, self.yaxis_end,
        self.midpoint_integer, self.midpoint_decimal, coordcount) = unpacker.unpack("<IHBBBBBBB")
        
        self.coordinates = ()
        while coordcount:
            self.coordinates += (unpacker.unpack("<BB"), )
            coordcount -= 1
        
        # FIXME: The rest of the file contents are unknown
        
    def pack(self):
        packer = PtpSession.PtpPacker()
        
        packer.pack("<IHBBBBBBB", 2116, 24393, self.xaxis_start, self.xaxis_end, self.yaxis_start, self.yaxis_end,
                    self.midpoint_integer, self.midpoint_decimal, len(self.coordinates))
        for c in self.coordinates:
            packer.pack("<BB", c[0], c[1])
        
        # FIXME: The rest of the file is unknown
        
        return packer.raw


# Values for DisableBodyControls
BODY_CONTROL_ENABLED = 0
BODY_CONTROL_DISABLED = 1

def DisableBodyControls(session, body_control):
    """Disable/enable camera body controls
    
    Arguments:
    session -- The PtpSession
    control_mode -- The camera control mode to set."""
        
    ptp_request = PtpSession.PtpRequest(PtpValues.NikonOperations.DISABLE_BODY_CONTROLS, session.sessionid, session.NewTransaction(), (body_control,))
    (ptp_response, rx) = session.transport.ptp_simple_transaction(ptp_request, receiving=False)
    if ptp_response.respcode != PtpValues.StandardResponses.OK:
        raise PtpSession.PtpException(ptp_response.respcode)

def AutoFocus(session):
    """Automatically focus the device.
    
    Arguments:
    session -- The PtpSession."""
        
    ptp_request = PtpSession.PtpRequest(PtpValues.NikonOperations.AUTO_FOCUS, session.sessionid, session.NewTransaction())
    (ptp_response, rx) = session.transport.ptp_simple_transaction(ptp_request, receiving=False)
    if ptp_response.respcode != PtpValues.StandardResponses.OK:
        raise PtpSession.PtpException(ptp_response.respcode)

def GetThumbMedium(session, objectHandle, stream=None):
    """Get a medium sized thumbnail.
    
    Arguments:
    session -- The PtpSession.
    objectid -- The objectid concerned."""
    
    ptp_request = PtpSession.PtpRequest(PtpValues.NikonOperations.GET_THUMB_MEDIUM, session.sessionid, session.NewTransaction(), (objectHandle, ))
    session.transport.send_ptp_request(ptp_request)
    rx_data = session.transport.get_ptp_data(ptp_request, stream)
    ptp_response = session.transport.get_ptp_response(ptp_request)
    if ptp_response.respcode != PtpValues.StandardResponses.OK:
        raise PtpSession.PtpException(ptp_response.respcode)
    return rx_data

def CheckEvents(session):
    """Check events on the camera -- e.g. Nikon doesn't appear to report property change events over the standard IRQ pipe."""
    
    ptp_request = PtpSession.PtpRequest(PtpValues.NikonOperations.CHECK_EVENTS, session.sessionid, session.NewTransaction())
    (ptp_response, rx) = session.transport.ptp_simple_transaction(ptp_request, receiving=True)
    if ptp_response.respcode != PtpValues.StandardResponses.OK:
        raise PtpSession.PtpException(ptp_response.respcode)
    
    unpacker = PtpSession.PtpUnpacker(rx[1])
    events = ()
    for i in xrange(0, unpacker.unpack("<H")[0]):
        (code, param) = unpacker.unpack("<HI")
        events += (PtpAbstractTransport.PtpEvent(code, 0xffffffff, 0xffffffff, (param, )), )
    return events

def PollStatus(session):
    """Poll the status of the device (e.g. for monitoring ASYNC operations such as autofocus or capture).
    
    Arguments:
    session -- The PtpSession."""
        
    ptp_request = PtpSession.PtpRequest(PtpValues.NikonOperations.POLL_STATUS, session.sessionid, session.NewTransaction())
    (ptp_response, rx) = session.transport.ptp_simple_transaction(ptp_request, receiving=False)
    return ptp_response.respcode

def RamCapture(session):
    """Capture a picture and leave it in the camera's RAM. Does not autofocus.
    
    Arguments:
    session -- The PtpSession."""
    
    # The meaning of the parameter is unknown: 0xffffffff captures into RAM. 0 captures, but doesn't appear to actually save the image
    ptp_request = PtpSession.PtpRequest(PtpValues.NikonOperations.RAM_CAPTURE, session.sessionid, session.NewTransaction(), (0xffffffff, ))
    (ptp_response, rx) = session.transport.ptp_simple_transaction(ptp_request, receiving=False)
    if ptp_response.respcode != PtpValues.StandardResponses.OK:
        raise PtpSession.PtpException(ptp_response.respcode)

def GetCurve(session):
    """Retrieve the curve from the camera
    
    Arguments:
    session -- The PtpSession."""
    
    ptp_request = PtpSession.PtpRequest(PtpValues.NikonOperations.GET_CURVE, session.sessionid, session.NewTransaction())
    (ptp_response, rx) = session.transport.ptp_simple_transaction(ptp_request, receiving=True)
    if ptp_response.respcode != PtpValues.StandardResponses.OK:
        raise PtpSession.PtpException(ptp_response.respcode)
    
    return NikonCurveInfo(rx[1])

def SetCurve(session, curve):
    """Send the curve to the camera
    
    Arguments:
    session -- The PtpSession."""
    
    ptp_request = PtpSession.PtpRequest(PtpValues.NikonOperations.SET_CURVE, session.sessionid, session.NewTransaction())
    (ptp_response, rx) = session.transport.ptp_simple_transaction(ptp_request, tx_data=curve.pack())
    if ptp_response.respcode != PtpValues.StandardResponses.OK:
        raise PtpSession.PtpException(ptp_response.respcode)

def GetLightMeter(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.LIGHT_METER, False, "b")

def GetLensId(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.LENS_ID, False, "B")

def GetFocalLengthMin(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.FOCAL_LENGTH_MIN, False, "L")

def GetFocalLengthMax(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.FOCAL_LENGTH_MAX, False, "L")

def GetACPowerPresent(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.AC_POWER, False, "B")

def GetFlashOpen(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.FLASH_OPEN, False, "B") == 1

def GetFlashCharged(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.FLASH_CHARGED, False, "B") == 1

def GetMenuMode(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.CSM_MENU, False, "B")

def SetMenuMode(session, value):
    session.SetDevicePropValue(PtpValues.NikonProperties.CSM_MENU, False, "B", value)

def GetBeepEnabled(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.BEEP_OFF, False, "B") == 0

def SetBeepEnabled(session, value):
    session.SetDevicePropValue(PtpValues.NikonProperties.BEEP_OFF, False, "B", value == False)

def GetLowLightIndicator(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.LOW_LIGHT_INDICATOR, False, "B")

def GetMaxApertureAtMinFocalLength(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.MAX_AP_AT_MIN_FOCAL_LENGTH, False, "H")

def GetMinApertureAtMaxFocalLength(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.MIN_AP_AT_MAX_FOCAL_LENGTH, False, "H")

def GetCameraOrientation(session):
    return session.GetDevicePropValue(PtpValues.NikonProperties.CAMERA_ORIENTATION, False, "B")
