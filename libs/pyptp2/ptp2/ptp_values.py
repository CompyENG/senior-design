__all__ = ['StandardResponses']

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