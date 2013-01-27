'''
#------------------------------------------------    
#  CHDK PTP Extension   LAST UPDATED TO 2.4
#------------------------------------------------

CHDK enums and #defines provided by core/ptp.h and core/liveview.h 
in CHDK source.
'''

__all__ = ['PTP_OC_CHDK', 'CHDKOperations', 'CHDKTypes', 'CHDKTempData', 'CHDKScriptLanguage',
    'CHDKScriptStatus', 'CHDKScriptError', 'CHDKMessageStatus', 'CHDKLVTransfer', 
    'CHDKLVAspectRatio', 'CHDKResponses']

PTP_OC_CHDK                         = 0x9999

class CHDKOperations(object):
    Version                         = 0
    GetMemory                       = 1
    SetMemory                       = 2
    CallFunction                    = 3
    TempData                        = 4
    UploadFile                      = 5
    DownloadFile                    = 6
    ExecuteScript                   = 7
    ScriptStatus                    = 8
    ScriptSupport                   = 9
    ReadScriptMsg                   = 10
    WriteScriptMsg                  = 11
    GetDisplayData                  = 12


class CHDKTypes(object):
    UNSUPPORTED                     = 0
    NIL                             = 1
    BOOLEAN                         = 2
    INTEGER                         = 3
    STRING                          = 4
    TABLE                           = 5

class CHDKTempData(object):
    DOWNLOAD                        = 0x1   #download data instead of upload
    CLEAR                           = 0x2   #clear the stored data; with DOWNLOAD this
                                            #means first download, then clear and
                                            #without DOWNLOAD this means no uploading,
                                            #just clear
class CHDKScriptLanguage(object):
    LUA                             = 0
    UBASIC                          = 1


class CHDKScriptStatus(object):
    NONE                            = 0     #no script running
    RUN                             = 0x1   #script running
    MSG                             = 0x2   #messages waiting

class CHDKScriptError(object):
    NONE                            = 0
    COMPILE                         = 1
    RUN                             = 2

class CHDKMessageStatus(object):
    OK                              = 0     #queued ok
    NOTRUN                          = 1     #no script is running
    QFULL                           = 2     #queue is full
    BADID                           = 3     #specified ID is not running

class CHDKLVTransfer(object):
    VIEWPORT                        = 0x01
    BITMAP                          = 0x04
    PALETTE                         = 0x08

class CHDKLVAspectRatio(object):
    ASPECT_4_3                      = 0
    ASPECT_16_9                     = 1

class CHDKResponses(object):
    OK                              = 0x2001
    GeneralError                    = 0x2002
    ParameterNotSupported           = 0x2006
    InvalidParameter                = 0x201D

    message = { OK:  'OK',
                GeneralError:  "General Error",
                ParameterNotSupported:  "Parameter Not Supported",
                InvalidParameter:  "Invalid Parameter"}

