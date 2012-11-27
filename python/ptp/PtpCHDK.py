# Based on code from the pyptp project (http://code.google.com/p/pyptp/) 

from PtpSession import PtpSession,PtpRequest,PtpPacker,PtpException
import PtpValues

import ctypes


class Enum(object):
	
	def __iter__(self):
		for k,v in self.__dict__.items():
			yield (k,v)
	
	
	def __getitem__(self,i):
		for k,v in self.__dict__.items():
			if v == i:
				return k
		else:
			raise KeyError(i)



class Flags(object):
	
	def __iter__(self):
		for k,v in self.__dict__.items():
			yield (k,v)
	
	
	def __getitem__(self,i):
		for k,v in self.__dict__.items():
			if v == i:
				return k
		else:
			raise KeyError(i)


	def pp(self,flags):
		return '|'.join([k for k,v in self if v&flags])


Operation = Enum()
Operation.Version = 0
Operation.GetMemory = 1
Operation.SetMemory = 2
Operation.CallFunction = 3
Operation.TempData = 4
Operation.UploadFile = 5
Operation.DownloadFile = 6
Operation.ExecuteScript = 7
Operation.ScriptStatus = 8
Operation.ScriptSupport = 9
Operation.ReadScriptMsg = 10
Operation.WriteScriptMsg = 11
Operation.GetLiveData = 12

PTP_OC_CHDK = 0x9999 

# data types as used by ReadScriptMessage
ScriptMessageSubType = Enum()
ScriptMessageSubType.UNSUPPORTED = 0 # type name will be returned in data
ScriptMessageSubType.NIL = 1
ScriptMessageSubType.BOOLEAN = 2
ScriptMessageSubType.INTEGER = 3
ScriptMessageSubType.STRING = 4 # Empty strings are returned with length=0
ScriptMessageSubType.TABLE = 5  # tables are converted to a string by usb_msg_table_to_string, the string may be empty for an empty table

# TempData flags
TempDataFlag = Flags()
TempDataFlag.DOWNLOAD = 0x1  # download data instead of upload
TempDataFlag.CLEAR = 0x2  # clear the stored data; with DOWNLOAD this means first download, then clear and without DOWNLOAD this means no uploading, just clear

# Script Languages - for execution only lua is supported for now
ScriptLanguage = Enum()
ScriptLanguage.LUA = 0
ScriptLanguage.UBASIC = 1

# bit flags for script status
ScriptStatusFlag = Flags()
ScriptStatusFlag.RUN = 0x1 # script running
ScriptStatusFlag.MSG = 0x2 # messages waiting

# bit flags for scripting support
ScriptSupportFlag = Enum()
ScriptSupportFlag.LUA = 0x1

# message types
ScriptMessageType = Enum()
ScriptMessageType.NONE = 0 # no messages waiting
ScriptMessageType.ERR = 1 # error message
ScriptMessageType.RET = 2 # script return value
ScriptMessageType.USER = 3 # message queued by script
	# TODO chdk console data ?

# error subtypes for PTP_CHDK_S_MSGTYPE_ERR and script startup status
ScriptErrorMessageType = Enum()
ScriptErrorMessageType.NONE = 0
ScriptErrorMessageType.COMPILE = 1
ScriptErrorMessageType.RUN = 2

# message status
ScriptMessageStatus = Enum()
ScriptMessageStatus.OK = 0 # queued ok
ScriptMessageStatus.NOTRUN = 1 # no script is running
ScriptMessageStatus.QFULL = 2 # queue is full
ScriptMessageStatus.BADID = 3 # specified ID is not running


# Control flags for determining which data block to transfer
LiveViewFlag = Flags()
LiveViewFlag.VIEWPORT = 0x01
LiveViewFlag.BITMAP = 0x04
LiveViewFlag.PALETTE = 0x08

# Live view aspect ratios
LiveViewAspect = Enum()
LiveViewAspect.LV_ASPECT_4_3 = 0
LiveViewAspect.LV_ASPECT_16_9 = 1

#
# Live view frame buffer data
#
class FramebufDesc(ctypes.Structure):
	_fields_ = [
		('logical_width',ctypes.c_int),
		('logical_height',ctypes.c_int),
		('buffer_width',ctypes.c_int),
		('buffer_logical_xoffset',ctypes.c_int),
		('buffer_logical_yoffset',ctypes.c_int),
		('visible_width',ctypes.c_int),
		('visible_height',ctypes.c_int),
		('data_start',ctypes.c_int),
	]

	def pp(self,indent=''):
		s = indent + '<%s>\n'%(self.__class__.__name__)
		for k,c in self._fields_:
			s += indent + '  %s: %r\n'%(k,getattr(self,k))
		return s

class LiveDataHeader(ctypes.Structure):
	_fields_ = [
		('version_major',ctypes.c_int),
		('version_minor',ctypes.c_int),
		('lcd_aspect_ratio',ctypes.c_int), # physical aspect ratio of LCD
		('palette_type',ctypes.c_int),
		('palette_data_start',ctypes.c_int),
		('viewport',FramebufDesc),
		('bitmap',FramebufDesc),
	]

	def pp(self,indent=''):
		s = indent + '<%s>\n'%(self.__class__.__name__)
		for k,c in self._fields_:
			v = getattr(self,k)
			if hasattr(v,'pp'):
				s += indent + '  %s:\n'%(k)
				s += v.pp(indent=indent+'    ')
			else:
				s += indent + '  %s: %r\n'%(k,v)
		return s

import numpy

class LiveViewFrame(object):
	"""
	This class is far from complete
	
	Still to do:
	
	Convert the YUVA/YUV palettes into RGBA palettes and use these to transform the image data to RGB. This will replicate the work of liveimg.c in chdkptp.
	
	The best way to handle this may be to make liveimg.c into a Python extension (removing all the LUA stuff and including
	just a simple function that takes the address of frame data as returned from the camera and returns a pair of RGBA strings
	which we can pass to PIL/OpenCV/Qt.
	"""
	
	
	def __init__(self,raw):
		headerBuffer = ctypes.create_string_buffer(raw[:ctypes.sizeof(LiveDataHeader)])
		header = LiveDataHeader.from_buffer(headerBuffer)
		print header.pp()
		if header.palette_data_start:
			paletteData = raw[header.palette_data_start:header.viewport.data_start]
		else:
			paletteData = '\x00\x00\x00\x00\xff\xe0\x00\x00\xff`\xeeb\xff\xb9\x00\x00\x7f\x00\x00\x00\xff~\xa1\xb3\xff\xcc\xb8^\xff_\x00\x00\xff\x94\xc5]\xff\x8aP\xb0\xffK=\xd4\x7f(\x00\x00\x7f\x00{\xe2\xff0\x00\x00\xffi\x00\x00\xff\x00\x00\x00'
		palette = numpy.fromstring(paletteData,numpy.int8)
		rgbPalette = yuv2rgb(palette)
		
		if header.viewport.data_start:
			vpData = raw[header.viewport.data_start:header.bitmap.data_start]
			print 'vp:',len(vpData),float(len(vpData))/header.viewport.logical_width
			vp = numpy.fromstring(vpData,numpy.uint8)
			vp = vp.reshape(header.viewport.logical_width,-1)
			print vp[:10]
		if header.bitmap.data_start:
			bmData = raw[header.bitmap.data_start:]
			print 'bp:',len(bmData),float(len(bmData))/header.bitmap.logical_width
			bm = numpy.fromstring(bmData,numpy.uint8)
			bm = bm.reshape(header.bitmap.logical_width,-1)
			print bm[:10]
			
		print 'len vp: %d',len(vpData),
		print 'len bm: %d',len(bmData)
		
		import Image
		image = Image.fromstring("L",(header.bitmap.logical_width,header.bitmap.logical_height),bmData,'raw','L')
		image.save(r'd:\temp\test-bm.jpg')

		image = Image.fromstring("RGB",(header.bitmap.logical_width,header.bitmap.logical_height),vpData,'raw','RGB')
		image.save(r'd:\temp\test-vp.jpg')
		


class ScriptMessage(object):
	def __init__(self,type,subType,data=None,scriptId=None):
		self.type = type
		self.subType = subType
		self.data = data
		self.scriptId = scriptId
	def __repr__(self):
		types = ScriptMessageType[self.type]
		if self.type == ScriptMessageType.ERR:
			subs = ScriptErrorMessageType[self.subType]
		elif self.type in (ScriptMessageType.RET,ScriptMessageType.USER):
			subs = ScriptMessageSubType[self.subType]
		elif self.type == ScriptMessageType.NONE:
			subs = 'N/A'
		else:
			subs = '[UNKNOWN]'
		ScriptMessageSubType
		s = '<%s type=%d (%s) subType=%d (%s) scriptId=%s data=%r>'%(self.__class__.__name__,self.type,types,self.subType,subs,self.scriptId,self.data)
		return s


class PtpCHDKSession(PtpSession):

	
	def chdkRequest(self,params):
		if type(params) is int:
			params = (params,)
			
		request = PtpRequest(PTP_OC_CHDK, self.sessionid, self.NewTransaction(), params)
		
		self.transport.send_ptp_request(request)
		response = self.transport.get_ptp_response(request)
		if response.respcode != PtpValues.StandardResponses.OK:
			raise PtpException(response.respcode)
		return response
	

	def chdkRequestWithSend(self,params,fmt=None,args=None,buffer=None):
		if type(params) is int:
			params = (params,)
			
		request = PtpRequest(PTP_OC_CHDK, self.sessionid, self.NewTransaction(), params)
		
		if fmt:
			assert args
			packer = PtpPacker()
			packer.pack(fmt,*args)
			tx_data = packer.raw
			if buffer:
				tx_data += buffer
		elif buffer:
			tx_data = buffer
		else:
			tx_data = None

		self.transport.send_ptp_request(request)
		self.transport.send_ptp_data(request, tx_data)
		response = self.transport.get_ptp_response(request)
		if response.respcode != PtpValues.StandardResponses.OK:
			raise PtpException(response.respcode)
		return response


	def chdkRequestWithReceive(self,params,stream=None):
		if type(params) is int:
			params = (params,)
			
		request = PtpRequest(PTP_OC_CHDK, self.sessionid, self.NewTransaction(), params)
		
		# send the tx
		rx_data = None
		response = None
		
		self.transport.send_ptp_request(request)
		if stream:
			self.transport.get_ptp_data(request,stream)
		else:
			data = self.transport.get_ptp_data(request)
		response = self.transport.get_ptp_response(request)
		if response.respcode != PtpValues.StandardResponses.OK:
			raise PtpException(response.respcode)
		if not stream:
			response.data = data
		return response
	
	
	def GetMemory(self,start,num):
		response = self.chdkRequestWithReceive((Operation.GetMemory,start,num))
		return response.data

	def SetMemory(self,addr,buffer):
		self.chdkRequestWithSend((Operation.SetMemory,addr,len(buffer)),buffer=buffer)
		return True
	
	def CallFunction(self,args):
		response = self.chdkRequestWithSend(Operation.CallFunction,args=args,fmt='I'*len(args))
		return response.params

	def Upload(self,remoteFilename,data):
		self.chdkRequestWithSend(
			Operation.UploadFile,
			fmt = 'I',
			args = (len(remoteFilename),),
			buffer = remoteFilename+data,
		)
		return True

	def Download(self,remoteFilename,stream=None):
		self.chdkRequestWithSend(
			(Operation.TempData,0),
			buffer = remoteFilename,
		)
		response = self.chdkRequestWithReceive(
			Operation.DownloadFile,
			stream = stream
		)
		if stream is None:
			return response.data

	def ExecuteScript(self,script):
		response = self.chdkRequestWithSend((Operation.ExecuteScript,ScriptLanguage.LUA,0),buffer=script+'\x00')
		return response.params

	def GetVersion(self):
		response = self.chdkRequest(Operation.Version)
		return response.params

	def GetScriptStatus(self,scriptId):
		response = self.chdkRequest(Operation.ScriptStatus)
		assert len(response.params) == 1
		return response.params[0]
	
	def GetScriptSupport(self,scriptId):
		response = self.chdkRequest(Operation.ScriptSupport)
		assert len(response.params) == 1
		return response.params[0]
	
	def WriteScriptMessage(self,scriptId,message):
		assert message
		response = self.chdkRequestWithSend((Operation.WriteScriptMsg,scriptId),buffer=message)
		assert len(response.params) == 1
		return response.params[0]
	
	def ReadScriptMessage(self,scriptId):
		response = self.chdkRequestWithReceive(Operation.ReadScriptMsg)
		sm = ScriptMessage(type=response.params[0],subType=response.params[1],scriptId=response.params[2],data=response.data)
		if sm.type == ScriptMessageType.NONE:
			return None
		else:
			return sm
	
	def GetLiveData(self,flags=LiveViewFlag.VIEWPORT|LiveViewFlag.BITMAP|LiveViewFlag.PALETTE):
		response = self.chdkRequestWithReceive((Operation.GetLiveData,flags))
		frame = LiveViewFrame(response.data[1])
		return frame

