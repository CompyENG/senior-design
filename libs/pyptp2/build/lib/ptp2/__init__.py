'''
Python package for interacting with digital cameras using the
Picture Transfer Protocol (PTP) over USB.  Similar to pyptp
(see references), but simpler and faster.  

Camera Objects:
	PTPCamera   Bare-bones PTP camera object provides basic
				communications functions.  You will need to
				provide values for operation codes and 
				parameters yourself, as well as interperate
				returned data streams.

	CHDKCamera 	Communicate with Canon cameras using the
				custom CHDK firmware.  See CHDK references 
				below for more information.  

Helpful functions:
	the 'util' module contains a few helpful functions:

	util.list_ptp_cameras() 
		Returns a list of usb devices for connected PTP cameras

	util.find_camera_by_serial(serial, partial_ok)
		Returns the usb device for a camera with a specific
		serial number. (Though, probably NOT the S/N stamped
		on the camera itself.  In linux, can be found with
		lsusb -v)


References and Thanks:
* Official PTP-USB specification:  http://www.usb.org/developers/devclass_docs/usb_still_img10.pdf
* pyptp: http://code.google.com/p/pyptp/
* libptp2: http://libptp.sourceforge.net/
* CHDK PTP Wiki entry: http://chdk.wikia.com/wiki/PTP_Extension
* CHDK PTP thread:  http://chdk.setepontos.com/index.php/topic,4338.0.html



Zachary Berkowitz
zac.berkowitz@gmail.com
'''

from camera import *
import util

class PTPError(Exception):

	def __init__(self, err_code, err_msg=''):
		self.value = err_code
		self.msg   = err_msg

	def __str__(self):
		return "PTPError(%04x: %s)" % (self.value, self.msg)


