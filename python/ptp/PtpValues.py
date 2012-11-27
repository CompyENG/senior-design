# -*- coding: utf-8 -*-
import inspect



def NameById(vendorId, inst, id):
    # Find the vendor
    if id & 0x8000:
        vendor = VendorNameById(vendorId)
    else:
        vendor = "STANDARD"

    # Try lookup
    result = "%04x" % id
    if inst != None:
        for op in inspect.getmembers(inst, lambda op: type(op) == int and op == id):
            result = op[0]
            break

    return "%s:%s" % (vendor, result)



class SimpleTypes:
    
    INT8                        = (0x0001, False, "b")
    UINT8                       = (0x0002, False, "B")
    INT16                       = (0x0003, False, "h")
    UINT16                      = (0x0004, False, "H")
    INT32                       = (0x0005, False, "i")
    UINT32                      = (0x0006, False, "I")
    INT64                       = (0x0007, False, "q")
    UINT64                      = (0x0008, False, "Q")
    INT128                      = (0x0009, False, "_INT128")
    UINT128                     = (0x000a, False, "_UINT128")
    
    AINT8                       = (0x4001, True, "b")
    AUINT8                      = (0x4002, True, "B")
    AINT16                      = (0x4003, True, "h")
    AUINT16                     = (0x4004, True, "H")
    AINT32                      = (0x4005, True, "i")
    AUINT32                     = (0x4006, True, "I")
    AINT64                      = (0x4007, True, "q")
    AUINT64                     = (0x4008, True, "Q")
    AINT128                     = (0x4009, True, "_INT128")
    AUINT128                    = (0x400a, True, "_UINT128")
    
    STR                         = (0xffff, False, "_STR")



def SimpleTypeDetailsById(typeId):
    result = None
    
    for op in inspect.getmembers(SimpleTypes(), lambda op: type(op) == tuple and len(op) == 3 and op[0] == typeId):
        result = op
        break
    
    return result



class Vendors:

    STANDARD                                = 0
    KODAK                                   = 1
    EPSON                                   = 2
    AGILENT                                 = 3
    POLAROID                                = 4
    AGFA                                    = 5
    MICROSOFT                               = 6
    EQUINOX                                 = 7
    VIEWQUEST                               = 8
    STMICRO                                 = 9
    NIKON                                   = 10
    CANON                                   = 11
    FOTONATION                              = 12
    PENTAX                                  = 13
    FUJIFILM                                = 14

def VendorNameById(vendorId):
    result = "UNKNOWN_%04x" % vendorId

    # Try and find the code
    for op in inspect.getmembers(Vendors(), lambda op: type(op) == int and op == vendorId):
        result = op[0]
        break

    return result




class StandardFunctionalModes:
    
    STANDARD                                = 0x0000
    SLEEP                                   = 0x0001

def FunctionalModeNameById(modeId, vendorId=Vendors.STANDARD):
    inst = None
    if (vendorId == Vendors.STANDARD) or not (modeId & 0x8000):
        inst = StandardFunctionalModes()
    
    return NameById(vendorId, inst, modeId)




class StandardOperations:

    UNDEFINED                               = 0x1000
    GET_DEVICE_INFO                         = 0x1001
    OPEN_SESSION                            = 0x1002
    CLOSE_SESSION                           = 0x1003
    GET_STORAGE_IDS                         = 0x1004
    GET_STORAGE_INFO                        = 0x1005
    GET_NUM_OBJECTS                         = 0x1006
    GET_OBJECT_HANDLES                      = 0x1007
    GET_OBJECT_INFO                         = 0x1008
    GET_OBJECT                              = 0x1009
    GET_THUMB                               = 0x100a
    DELETE_OBJECT                           = 0x100b
    SEND_OBJECT_INFO                        = 0x100c
    SEND_OBJECT                             = 0x100d
    INITIATE_CAPTURE                        = 0x100e
    FORMAT_STORE                            = 0x100f
    RESET_DEVICE                            = 0x1010
    SELF_TEST                               = 0x1011
    SET_OBJECT_PROTECTION                   = 0x1012
    POWER_DOWN                              = 0x1013
    GET_DEVICE_PROP_DESC                    = 0x1014
    GET_DEVICE_PROP_VALUE                   = 0x1015
    SET_DEVICE_PROP_VALUE                   = 0x1016
    RESET_DEVICE_PROP_VALUE                 = 0x1017
    TERMINATE_OPEN_CAPTURE                  = 0x1018
    MOVE_OBJECT                             = 0x1019
    COPY_OBJECT                             = 0x101a
    GET_PARTIAL_OBJECT                      = 0x101b
    INITATE_OPEN_CAPTURE                    = 0x101c

class KodakOperations:
    
    SEND_FILE_OBJECT_INFO                   = 0x9005
    SEND_FILE_OBJECT                        = 0x9006

class CanonOperations:

    GET_OBJECT_SIZE                         = 0x9001
    START_SHOOTING_MODE                     = 0x9008
    END_SHOOTING_MODE                       = 0x9009
    VIEW_FINDER_ON                          = 0x900b
    VIEW_FINDER_OFF                         = 0x900c
    REFLECT_CHANGES                         = 0x900d
    CHECK_EVENT                             = 0x9013
    FOCUS_LOCK                              = 0x9014
    FOCUS_UNLOCK                            = 0x9015
    INITIATE_CAPTURE_IN_MEMORY              = 0x901a
    GET_PARTIAL_OBJECT                      = 0x901b
    GET_VIEWFINDER_IMAGE                    = 0x901d
    GET_CHANGES                             = 0x9020
    GET_FOLDER_ENTRIES                      = 0x9021

class NikonOperations:

    RAM_CAPTURE                             = 0x90c0
    AUTO_FOCUS                              = 0x90c1
    DISABLE_BODY_CONTROLS                   = 0x90c2
    GET_THUMB_MEDIUM                        = 0x90c4
    GET_CURVE                               = 0x90c5
    SET_CURVE                               = 0x90c6
    CHECK_EVENTS                            = 0x90c7
    POLL_STATUS                             = 0x90c8

    # Unknown Operations: 90c3, 90c9

def OperationNameById(operationId, vendorId=Vendors.STANDARD):
    inst = None
    if (vendorId == Vendors.STANDARD) or not (operationId & 0x8000):
        inst = StandardOperations()
    elif vendorId == Vendors.KODAK:
        inst = KodakOperations()
    elif vendorId == Vendors.CANON:
        inst = CanonOperations()
    elif vendorId == Vendors.NIKON:
        inst = NikonOperations()

    return NameById(vendorId, inst, operationId)




class StandardResponses:

    UNDEFINED                               = 0x2000
    OK                                      = 0x2001
    GENERAL_ERROR                           = 0x2002
    SESSION_NOT_OPEN                        = 0x2003
    INVALID_TRANSACTION_ID                  = 0x2004
    OPERATION_NOT_SUPPORTED                 = 0x2005
    PARAMETER_NOT_SUPPORTED                 = 0x2006
    INCOMPLETE_TRANSFER                     = 0x2007
    INVALID_STORAGE_ID                      = 0x2008
    INVALID_OBJECT_HANDLE                   = 0x2009
    DEVICE_PROP_NOT_SUPPORTED               = 0x200a
    INVALID_OBJECT_FORMAT_CODE              = 0x200b
    STORE_FULL                              = 0x200c
    OBJECT_WRITE_PROTECTED                  = 0x200d
    STORE_READ_ONLY                         = 0x200e
    ACCESS_DENIED                           = 0x200f
    NO_THUMBNAIL_PRESENT                    = 0x2010
    SELF_TEST_FAILED                        = 0x2011
    PARTIAL_DELETION                        = 0x2012
    STORE_NOT_AVAILABLE                     = 0x2013
    SPECIFICATION_BY_FORMAT_NOT_SUPPORTED   = 0x2014
    NO_VALID_OBJECT_INFO                    = 0x2015
    INVALID_CODE_FORMAT                     = 0x2016
    UNKNOWN_VENDOR_CODE                     = 0x2017
    CAPTURE_ALREADY_TERMINATED              = 0x2018
    DEVICE_BUSY                             = 0x2019
    INVALID_PARENT_OBJECT                   = 0x201a
    INVALID_DEVICE_PROP_FORMAT              = 0x201b
    INVALID_DEVICE_PROP_VALUE               = 0x201c
    INVALID_PARAMETER                       = 0x201d
    SESSION_ALREADY_OPEN                    = 0x201e
    TRANSACTION_CANCELLED                   = 0x201f
    SPECIFICATION_OF_DESTINATION_UNSUPPORTED= 0x2020

class KodakResponses:
    
    FILENAME_REQUIRED                       = 0xa001
    FILENAME_CONFLICTS                      = 0xa002
    FILENAME_INVALID                        = 0xa003

class NikonResponses:
    
    ASYNC_OPERATION_FAILED                  = 0xa002
    PROPERTY_READ_ONLY                      = 0xa005

def ResponseNameById(responseId, vendorId=Vendors.STANDARD):
    inst = None
    if (vendorId == Vendors.STANDARD) or not (responseId & 0x8000):
        inst = StandardResponses()
    elif vendorId == Vendors.KODAK:
        inst = KodakResponses()
    elif vendorId == Vendors.NIKON:
        inst = NikonResponses()

    return NameById(vendorId, inst, responseId)




class StandardEvents:

    UNDEFINED                               = 0x4000
    CANCEL_TRANSACTION                      = 0x4001
    OBJECT_ADDED                            = 0x4002
    OBJECT_REMOVED                          = 0x4003
    STORE_ADDED                             = 0x4004
    STORE_REMOVED                           = 0x4005
    DEVICE_PROP_CHANGED                     = 0x4006
    OBJECT_INFO_CHANGED                     = 0x4007
    DEVICE_INFO_CHANGED                     = 0x4008
    REQUEST_OBJECT_TRANSFER                 = 0x4009
    STORE_FULL                              = 0x400a
    DEVICE_RESET                            = 0x400b
    STORAGE_INFO_CHANGED                    = 0x400c
    CAPTURE_COMPLETE                        = 0x400d
    UNREPORTED_STATUS                       = 0x400e

class CanonEvents:
    
    DEVICE_INFO_CHANGED                     = 0xc008
    REQUEST_OBJECT_TRANSFER                 = 0xc009
    CAMERA_MODE_CHANGED                     = 0xc00c

class NikonEvents:
    
    OBJECT_READY                            = 0xc101
    CAPTURE_OVERFLOW                        = 0xc102

def EventNameById(eventId, vendorId=Vendors.STANDARD):
    inst = None
    if (vendorId == Vendors.STANDARD) or not (eventId & 0x8000):
        inst = StandardEvents()
    elif vendorId == Vendors.CANON:
        inst = CanonEvents()
    elif vendorId == Vendors.NIKON:
        inst = NikonEvents()

    return NameById(vendorId, inst, eventId)




class StandardProperties:

    UNDEFINED                               = 0x5000
    BATTERY_LEVEL                           = 0x5001
    FUNCTIONAL_MODE                         = 0x5002
    IMAGE_SIZE                              = 0x5003
    COMPRESSION_SETTING                     = 0x5004
    WHITE_BALANCE                           = 0x5005
    RGB_GAIN                                = 0x5006
    F_NUMBER                                = 0x5007
    FOCAL_LENGTH                            = 0x5008
    FOCUS_DISTANCE                          = 0x5009
    FOCUS_MODE                              = 0x500a
    EXPOSURE_METERING_MODE                  = 0x500b
    FLASH_MODE                              = 0x500c
    EXPOSURE_TIME                           = 0x500d
    EXPOSURE_PROGRAM_MODE                   = 0x500e
    EXPOSURE_INDEX                          = 0x500f
    EXPOSURE_BIAS_COMPENSATION              = 0x5010
    DATE_TIME                               = 0x5011
    CAPTURE_DELAY                           = 0x5012
    STILL_CAPTURE_MODE                      = 0x5013
    CONTRAST                                = 0x5014
    SHARPNESS                               = 0x5015
    DIGITAL_ZOOM                            = 0x5016
    EFFECT_MODE                             = 0x5017
    BURST_NUMBER                            = 0x5018
    BURST_INTERVAL                          = 0x5019
    TIMELAPSE_NUMBER                        = 0x501a
    TIMELAPSE_INTERVAL                      = 0x501b
    FOCUS_METERING_MODE                     = 0x501c
    UPLOAD_URL                              = 0x501d
    ARTIST                                  = 0x501e
    COPYRIGHT_INFO                          = 0x501f

class KodakProperties:
    
    COLOUR_TEMPERATURE                      = 0xd001
    DATE_TIME_STAMP_FORMAT                  = 0xd002
    BEEP_MODE                               = 0xd003
    VIDEO_OUT                               = 0xd004
    POWER_SAVING                            = 0xd005
    UI_LANGUAGE                             = 0xd006

class CanonProperties:
    BEEP_MODE                               = 0xd001
    VIDEFINDER_MODE                         = 0xd003
    IMAGE_QUALITY                           = 0xd006
    IMAGE_SIZE                              = 0xd008
    FLASH_MODE                              = 0xd00a
    TV_AV_SETTING                           = 0xd00c
    METERING_MODE                           = 0xd010
    MACRO_MODE                              = 0xd011
    FOCUSING_POINT                          = 0xd012
    WHITE_BALANCE                           = 0xd013
    ISO_SPEED                               = 0xd01c
    APERTURE                                = 0xd01d
    SHUTTER_SPEED                           = 0xd01e
    EXPOSURE_COMPENSATION                   = 0xd01f
    ZOOM                                    = 0xd02a
    SIZE_QUALITY_MODE                       = 0xd02c
    FLASH_MEMORY                            = 0xd031
    CAMERA_MODEL                            = 0xd032
    CAMERA_OWNER                            = 0xd033
    UNIX_TIME                               = 0xd034
    VIEWFINDER_OUTPUT                       = 0xd036
    REAL_IMAGE_WIDTH                        = 0xd039
    PHOTO_EFFECT                            = 0xd040
    ASSIST_LIGHT                            = 0xd041

class NikonProperties:
    SHOOTING_BANK                           = 0xD010
    SHOOTING_BANK_NAME_A                    = 0xD011
    SHOOTING_BANK_NAME_B                    = 0xD012
    SHOOTING_BANK_NAME_C                    = 0xD013
    SHOOTING_BANK_NAME_D                    = 0xD014
    RAW_COMPRESSION                         = 0xD016
    WHITE_BALANCE_AUTO_BIAS                 = 0xD017
    WHITE_BALANCE_TUNGSTEN_BIAS             = 0xD018
    WHITE_BALANCE_FLUORESCENT_BIAS          = 0xD019
    WHITE_BALANCE_DAYLIGHT_BIAS             = 0xD01A
    WHITE_BALANCE_FLASH_BIAS                = 0xD01B
    WHITE_BALANCE_CLOUDY_BIAS               = 0xD01C
    WHITE_BALANCE_SHADE_BIAS                = 0xD01D
    WHITE_BALANCE_COLOUR_TEMPERATURE        = 0xD01E
    IMAGE_SHARPENING                        = 0xD02A
    TONE_COMPENSATION                       = 0xD02B
    COLOUR_MODE                             = 0xD02C
    HUE_ADJUSTMENT                          = 0xD02D
    NON_CPU_LENS_DATA_FOCAL_LENGTH          = 0xD02E
    NON_CPU_LENS_DATA_MAX_APERTURE          = 0xD02F
    CSM_MENU_BANK_SELECT                    = 0xD040
    MENU_BANK_NAME_A                        = 0xD041
    MENU_BANK_NAME_B                        = 0xD042
    MENU_BANK_NAME_C                        = 0xD043
    MENU_BANK_NAME_D                        = 0xD044
    A1AFC_MODE_PRIORITY                     = 0xD048
    A2AFS_MODE_PRIROITY                     = 0xD049
    A3_GROUP_DYNAMIC_AF                     = 0xD04A
    A4_AF_ACTIVATION                        = 0xD04B
    A5_FOCUS_AREA_ILLUM_MANUAL_FOCUS        = 0xD04C
    FOCUS_AREA_ILLUM_CONTINUOUS             = 0xD04D
    FOCUS_AREA_ILLUM_WHEN_SELECTED          = 0xD04E
    FOCUS_AREA_WRAP                         = 0xD04F
    A7_VERTICAL_AFON                        = 0xD050
    ISO_AUTO                                = 0xD054
    B2_ISO_STEP                             = 0xD055
    EV_STEP                                 = 0xD056
    B4_EXPOSURE_COMP_EV                     = 0xD057
    EXPOSURE_COMPENSATION                   = 0xD058
    CENTRE_WEIGHT_AREA                      = 0xD059
    AE_LOCK_MODE                            = 0xD05E
    AELAFL_MODE                             = 0xD05F
    METER_OFF                               = 0xD062
    SELF_TIMER                              = 0xD063
    MONITOR_OFF                             = 0xD064
    D1_SHOOTING_SPEED                       = 0xD068
    D2_MAX_SHOTS                            = 0xD069
    D3_EXP_DELAY_MODE                       = 0xD06A
    LONG_EXPOSURE_NOISE_REDUCTION           = 0xD06B
    FILE_NUMBER_SEQUENCE                    = 0xD06C
    D6_CONTROL_PANEL_FINDER_REAR_CONTROL    = 0xD06D
    CONTROL_PANEL_FINDER_VIEW_FINDER        = 0xD06E
    D7_ILLUMINATION                         = 0xD06F
    E1_FLASH_SYNC_SPEED                     = 0xD074
    FLASH_SHUTTER_SPEED                     = 0xD075
    E3_AA_FLASH_MODE                        = 0xD076
    E4_MODELING_FLASH                       = 0xD077 
    BRACKET_SET                             = 0xD078
    E6_MANUAL_MODE_BRACKETING               = 0xD079
    BRACKET_ORDER                           = 0xD07A
    E8_AUTO_BRACKET_SELECTION               = 0xD07B
    BRACKETING_SET                          = 0xD07C
    F1_CENTER_BUTTON_SHOOTING_MODE          = 0xD080
    CENTER_BUTTON_PLAYBACK_MODE             = 0xD081
    F2_MULTI_SELECTOR                       = 0xD082
    F3_PHOTO_INFO_PLAYBACK                  = 0xD083
    F4_ASSIGN_FUNC_BUTTON                   = 0xD084
    F5_CUSTOMISE_COMMS_DIALS                = 0xD085
    REVERSE_COMMAND_DIAL                    = 0xD086
    APERTURE_SETTING                        = 0xD087
    MENUS_AND_PLAYBACK                      = 0xD088
    F6_BUTTONS_AND_DIALS                    = 0xD089
    NO_CF_CARD                              = 0xD08A
    IMAGE_COMMENT_STRING                    = 0xD090
    IMAGE_COMMENT_ATTACH                    = 0xD091
    IMAGE_ROTATION                          = 0xD092
    BRACKETING                              = 0xD0C0
    EXPOSURE_BRACKETING_INTERVAL_DIST       = 0xD0C1
    BRACKETING_PROGRAM                      = 0xD0C2
    WHITE_BALANCE_BRACKET_STEP              = 0xD0C4
    LENS_ID                                 = 0xD0E0
    FOCAL_LENGTH_MIN                        = 0xD0E3
    FOCAL_LENGTH_MAX                        = 0xD0E4
    MAX_AP_AT_MIN_FOCAL_LENGTH              = 0xD0E5
    MAX_AP_AT_MAX_FOCAL_LENGTH              = 0xD0E6
    EXPOSURE_TIME                           = 0xD100
    AC_POWER                                = 0xD101
    MAXIMUM_SHOTS                           = 0xD103
    AFL_LOCK                                = 0xD104
    AUTO_EXPOSURE_LOCK                      = 0xD105
    AUTO_FOCUS_LOCK                         = 0xD106
    AUTO_FOCUS_LCD_TOP_MODE2                = 0xD107
    AUTO_FOCUS_AREA                         = 0xD108
    LIGHT_METER                             = 0xD10A
    CAMERA_ORIENTATION                      = 0xD10E
    EXPOSURE_APERTURE_LOCK                  = 0xD111
    FLASH_EXPOSURE_COMPENSATION             = 0xD126
    OPTIMIZE_IMAGE                          = 0xD140
    SATURATION                              = 0xD142
    BEEP_OFF                                = 0xD160
    AUTO_FOCUS_MODE                         = 0xD161
    AF_ASSIST                               = 0xD163
    PA_DVP_MODE                             = 0xD164
    IMAGE_REVIEW                            = 0xD165
    AF_AREA_ILLUMINATION                    = 0xD166
    FLASH_MODE                              = 0xD167
    FLASH_COMMANDER_MODE                    = 0xD168
    FLASH_SIGN                              = 0xD169
    REMOTE_TIMEOUT                          = 0xD16B
    GRID_DISPLAY                            = 0xD16C
    FLASH_MODE_MANUAL_POWER                 = 0xD16D
    FLASH_MODE_COMMANDER_POWER              = 0xD16E
    CSM_MENU                                = 0xD180
    BRACKETING_INCREMENT                    = 0xD190
    LOW_LIGHT_INDICATOR                     = 0xD1B0
    FLASH_OPEN                              = 0xD1C0
    FLASH_CHARGED                           = 0xD1C1

def PropertyNameById(propertyId, vendorId=Vendors.STANDARD):
    inst = None
    if (vendorId == Vendors.STANDARD) or not (propertyId & 0x8000):
        inst = StandardProperties()
    elif vendorId == Vendors.KODAK:
        inst = KodakProperties()
    elif vendorId == Vendors.CANON:
        inst = CanonProperties()
    elif vendorId == Vendors.NIKON:
        inst = NikonProperties()
        
    return NameById(vendorId, inst, propertyId)




class StandardObjectFormats:
    ASSOCIATION                             = 0x3001
    SCRIPT                                  = 0x3002
    EXECUTABLE                              = 0x3003
    TEXT                                    = 0x3004
    HTML                                    = 0x3005
    DPOF                                    = 0x3006
    AIFF                                    = 0x3007
    WAV                                     = 0x3008
    MP3                                     = 0x3009
    AVI                                     = 0x300A
    MPEG                                    = 0x300B
    ASF                                     = 0x300C
    EXIF_JPEG                               = 0x3801
    TIFF_EP                                 = 0x3802
    FLASH_PIX                               = 0x3803
    BMP                                     = 0x3804
    CIFF                                    = 0x3805
    GIF                                     = 0x3807
    JFIF                                    = 0x3808
    PCD                                     = 0x3809
    PICT                                    = 0x380A
    PNG                                     = 0x380B
    TIFF                                    = 0x380D
    TIFF_IT                                 = 0x380E
    JP2                                     = 0x380F
    JPX                                     = 0x3810

class KodakObjectFormats:
    M3U                                     = 0xb002

def ObjectFormatNameById(formatId, vendorId=Vendors.STANDARD):
    inst = None
    if (vendorId == Vendors.STANDARD) or not (formatId & 0x8000):
        inst = StandardObjectFormats()
    elif vendorId == Vendors.KODAK:
        inst = KodakObjectFormats()
        
    return NameById(vendorId, inst, formatId)



class StandardStorageTypes:
    UNDEFINED                               = 0x0000
    FIXED_ROM                               = 0x0001
    REMOVABLE_ROM                           = 0x0002
    FIXED_RAM                               = 0x0003
    REMOVABLE_RAM                           = 0x0004

def StorageTypeNameById(typeId, vendorId=Vendors.STANDARD):
    inst = StandardStorageTypes()
        
    return NameById(vendorId, inst, typeId)




class StandardFilesystemTypes:
    UNDEFINED                               = 0x0000
    GENERIC_FLAT                            = 0x0001
    GENERIC_HIERARCHICAL                    = 0x0002
    DCF                                     = 0x0003

def FilesystemTypeNameById(typeId, vendorId=Vendors.STANDARD):
    inst = None
    if (vendorId == Vendors.STANDARD) or not (typeId & 0x8000):
        inst = StandardFilesystemTypes()
        
    return NameById(vendorId, inst, typeId)




class StandardAccessCapability:
    RW                                      = 0x0000
    RO_WITHOUT_DELETION                     = 0x0001
    RO_WITH_DELETION                        = 0x0002

def AccessCapabilityNameById(capId, vendorId=Vendors.STANDARD):
    inst = StandardAccessCapability()
        
    return NameById(vendorId, inst, capId)


class StandardProtectionStatus:
    NO_PROTECTION                           = 0x0000
    READ_ONLY                               = 0x0001

def ProtectionStatusNameById(capId, vendorId=Vendors.STANDARD):
    inst = StandardProtectionStatus()
        
    return NameById(vendorId, inst, capId)



class StandardAssociationTypes:
    UNDEFINED                               = 0x0000
    GENERIC_FOLDER                          = 0x0001
    ALBUM                                   = 0x0002
    TIME_SEQUENCE                           = 0x0003
    HORIZONTAL_PANORAMIC                    = 0x0004
    VERTICAL_PANORAMIC                      = 0x0005
    TWOD_PANORAMIC                          = 0x0006
    ANCILLARY_DATA                          = 0x0007

def AssociationTypeNameById(typeId, vendorId=Vendors.STANDARD):
    inst = None
    if (vendorId == Vendors.STANDARD) or not (typeId & 0x8000):
        inst = StandardFilesystemTypes()
        
    return NameById(vendorId, inst, typeId)



class GetSet:
    GET                                     = 0x0000
    GET_SET                                 = 0x0001
    
def GetSetNameById(typeId, vendorId=Vendors.STANDARD):
    inst = GetSet()        
    return NameById(vendorId, inst, typeId)


class StandardWhiteBalanceTypes:
    UNDEFINED                               = 0x0000
    MANUAL                                  = 0x0001
    AUTOMATIC                               = 0x0002
    ONE_PUSH_AUTOMATIC                      = 0x0003
    DAYLIGHT                                = 0x0004
    FLUORESCENT                             = 0x0005
    TUNGSTEN                                = 0x0006
    FLASH                                   = 0x0007
    
class NikonWhiteBalanceTypes:
    CLOUDY                                  = 0x8010
    SHADE                                   = 0x8011
    PRESET                                  = 0x8013

# FIXME


class StandardFocusModes:
    UNDEFINED                               = 0x0000
    MANUAL                                  = 0x0001
    AUTOMATIC                               = 0x0002
    AUTOMATIC_MACRO                         = 0x0003
    
class NikonFocusModes:
    AF_S                                    = 0x8010
    AF_C                                    = 0x8011
    AF_A                                    = 0x8012

# FIXME



class StandardExposureMeteringModes:
    UNDEFINED                               = 0x0000
    AVERAGE                                 = 0x0001
    CENTER                                  = 0x0002
    MULTI_SPOT                              = 0x0003
    CENTER_SPOT                             = 0x0004
    
# FIXME


class StandardFlashModes:
    UNDEFINED                               = 0x0000
    AUTO_FLASH                              = 0x0001
    FLASH_OFF                               = 0x0002
    FILL_FLASH                              = 0x0003
    RED_EYE_AUTO                            = 0x0004
    RED_EYE_FILL                            = 0x0005
    EXTERNAL_SYNC                           = 0x0006
    
class NikonFlashModes:
    DEFAULT                                 = 0x8010
    SLOW                                    = 0x8011
    REAR                                    = 0x8012
    SLOW_RED_EYE                            = 0x8013
    
# FIXME

class StandardExposureProgramModes:
    UNDEFINED                               = 0x0000
    MANUAL                                  = 0x0001
    AUTOMATIC                               = 0x0002
    APERTURE_PRIORITY                       = 0x0003
    SHUTTER_PRIORITY                        = 0x0004
    PROGRAM_CREATIVE                        = 0x0005
    PROGRAM_ACTION                          = 0x0006
    PORTRAIT                                = 0x0007
    
class NikonExposureProgramModes:
    POINT_AND_SHOOT                         = 0x8010
    PORTRAIT                                = 0x8011
    LANDSCAPE                               = 0x8012
    CLOSE_UP                                = 0x8013
    ACTION                                  = 0x8014
    NIGHT                                   = 0x8015
    CHILDREN                                = 0x8017

# FIXME

class StandardStillCaptureModes:
    UNDEFINED                               = 0x0000
    NORMAL                                  = 0x0001
    BURST                                   = 0x0002
    TIMELAPSE                               = 0x0003

class NikonStillCaptureModes:
    TIMER                                   = 0x8011
    REMOTE                                  = 0x8013
    TIMER_REMOTE                            = 0x8014

# FIXME


class StandardFocusMeteringModes:
    UNDEFINED                               = 0x0000
    CENTER_SPOT                             = 0x0001
    MULTI_SPOT                              = 0x0002
    
class NikonFocusMeteringModes:
    SINGLE                                  = 0x8010
    CLOSEST_SUBJECT                         = 0x8011

# FIXME


class NikonLowLightIndicatorValues:
    OK                                      = 0
    APERTURE_LOW                            = 1
    APERTURE_HIGH                           = 2
    SHUTTER_HIGH                            = 3
    SHUTTER_HIGH                            = 4

# FIXME

class NikonCameraOrientations:
    LEVEL                                   = 0
    LEFT                                    = 1
    RIGHT                                   = 2
