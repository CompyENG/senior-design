import struct

try:
    import numpy as np
    fromstring = lambda byte_str,type_: np.fromstring(byte_str, type_)

except ImportError:
    import array
    fromstring = lambda byte_str, type_: array.array(type_, byte_str)



PTP_USB_COMMAND         = 1
PTP_USB_DATA            = 2
PTP_USB_RESPONSE        = 3
PTP_USB_EVENT           = 4


__all__ = ['PTP_CONTAINER_TYPE','ParamContainer', 'DataContainer', 'CHDK_LV_Data', 
    'CHDK_FrameBuffer', 'CHDK_DataHeader']

class PTP_CONTAINER_TYPE(object):
    COMMAND         = 1
    DATA            = 2
    RESPONSE        = 3
    EVENT           = 4  


class _PyStructure(object):

    def __init__(self, fields, endian='<', bytestr=None):
        
        if endian in '@=<>!':
            self._fmt = endian
        else:
            raise ValueError('Illegal endian type %s, see `struct` pydocs' %(endian))

        self._fields = []

        for (name, type_) in fields:
            #numeric
            if type_[-1] in 'bBhHiIlLqQfd':
                setattr(self, name, 0)
                
            #character types
            elif type_[-1] in 'cs':
                setattr(self, name, '')

            #boolean type
            elif type_[-1] in '?':
                setattr(self, name, False)

            else:
                raise(TypeError('Unsupported type %s' %(type_)))

            self._fmt += type_
            self._fields.append(name)

        self._size = struct.calcsize(self._fmt)

        if bytestr is not None:
            self.unpack(bytestr)

    def unpack(self, bytestr):
        vals = struct.unpack(self.fmt, bytestr[:self.size])
        for n, name in enumerate(self.fields):
            setattr(self, name, vals[n])

    def pack(self):
        vals = []
        for name in self.fields:
            vals.append(getattr(self, name))

        return struct.pack(self.fmt, *vals)

    def __str__(self):
        d = {}
        for name in self.fields:
            d[name] = getattr(self, name)

        return str(d)

    @property
    def fmt(self):
        return self._fmt

    @property 
    def fields(self):
        return self._fields

    @property
    def size(self):
        return self._size




class CHDK_FrameBuffer(_PyStructure):

    def __init__(self, bytestr=None):
        fields = [
                    ('fb_type', 'i'),
                    ('data_start', 'i'),
                    ('buffer_width', 'i'),
                    ('visible_width', 'i'),
                    ('visible_height', 'i'),
                    ('margin_left', 'i'),
                    ('margin_top', 'i'),
                    ('margin_right', 'i'),
                    ('margin_bot', 'i'),
                ]

        _PyStructure.__init__(self, fields, endian='<', bytestr=bytestr)


class CHDK_DataHeader(_PyStructure):

    def __init__(self, bytestr=None):
        fields = [
                    ('version_major', 'i'),
                    ('version_minor', 'i'),
                    ('lcd_aspect_ratio', 'i'),
                    ('palette_type', 'i'),
                    ('palette_data_start', 'i'),
                    ('vp_desc_start', 'i'),
                    ('bm_desc_start', 'i'),
                ]

        _PyStructure.__init__(self, fields, endian='<', bytestr=bytestr)

class CHDK_LV_Data(object):

    def __init__(self, bytestr=None):

        self.header  = None
        self.vp_desc = None
        self.bm_desc = None
        self.vp_data = None
        self.bm_data = None

        if bytestr is not None:
            self.unpack(bytestr)

    def unpack(self, bytestr):

        lb = 0
        self.header = CHDK_DataHeader()
        ub = self.header.size

        self.header.unpack(bytestr[lb:ub])
        
        lb = self.header.vp_desc_start
        self.vp_desc = CHDK_FrameBuffer()
        ub = lb + self.vp_desc.size
        self.vp_desc.unpack(bytestr[lb:ub])

        lb = self.header.bm_desc_start
        self.bm_desc = CHDK_FrameBuffer()
        ub = lb + self.bm_desc.size
        self.bm_desc.unpack(bytestr[lb:ub])


        if self.vp_desc.data_start > 0 :
            vp_size = self.vp_desc.buffer_width * self.vp_desc.visible_height * 6 / 4
            lb = self.vp_desc.data_start
            ub = lb + vp_size
            self.vp_data = fromstring(bytestr[lb:ub], 'B')

        if self.bm_desc.data_start > 0:
            bm_size = self.bm_desc.buffer_width * self.bm_desc.visible_height
            lb = self.bm_desc.data_start
            ub = lb + bm_size
            self.bm_data = fromstring(bytestr[lb:ub], 'B')

    def pack(self):
        total_size = self.header.size + self.vp_desc.size + self.bm_desc.size
        if self.vp_data is not None:
            total_size += len(self.vp_data) * 4

        if self.bm_data is not None:
            total_size += len(self.bm_data) * 4

        bytestr = ' ' * total_size

        lb = 0
        ub = self.header.size
        #bytestr[lb:ub] = self.header.pack()
        bytestr = bytestr[:lb-1]+self.header.pack()+bytestr[ub+1:]
        
        lb=self.header.vp_desc_start
        ub=lb + self.vp_desc.size
        #bytestr[lb:ub] = self.vp_desc.pack()
        bytestr = bytestr[:lb-1]+self.vp_desc.pack()+bytestr[ub+1:]

        lb = self.header.bm_desc_start
        ub = lb + self.bm_desc.size
        #bytestr[lb:ub] = self.bm_desc.pack()
        bytestr = bytestr[:lb-1]+self.bm_desc.pack()+bytestr[ub+1:]

        if self.vp_data is not None:
            vp_size = self.vp_desc.buffer_width * self.vp_desc.visible_height * 6 / 4
            lb = self.vp_desc.data_start
            ub = lb + vp_size
            fmt = '<%dB' % (len(self.vp_data))
            #bytestr[lb:ub] = struct.pack(fmt, *self.vp_data)
            bytestr = bytestr[:lb-1]+struct.pack(fmt, *self.vp_data)+bytestr[ub+1:]

        if self.bm_data is not None:
            bm_size = self.bm_desc.buffer_width * self.bm_desc.visible_height
            lb = self.bm_desc.data_start
            ub = lb + bm_size
            fmt = '<%dB' % (len(self.bm_data))
            #bytestr[lb:ub] = struct.pack(fmt, *self.bm_data)
            bytestr = bytestr[:lb-1]+struct.pack(fmt, *self.bm_data)+bytestr[ub+1:]

        return bytestr
        


class ParamContainer(_PyStructure):


    def __init__(self, bytestr=None):

        header = [  
                    ('length',            'I'),
                    ('type',              'H'),
                    ('code',              'H'),
                    ('transaction_id',    'I'),
                 ]

        _PyStructure.__init__(self, header, endian='<')

        self._params = []

        if bytestr is not None:
            self.unpack(bytestr)
      
        # else:
        #     for name, value in kwargs.iteritems():
        #         if name == 'params' and isinstance(value, list):
        #             self._params = params.copy()
        #         if hasattr(self, name):
        #             setattr(self, name, value)
        #         else:
        #             raise(AttributeError('%s has no attribute %s'%(self.__class__, name)))


    def unpack(self, bytestr):
        _PyStructure.unpack(self, bytestr[:12])

        num_params = (self.length - 12) / 4
        str_len = len(bytestr)
        exp_len = 12 + num_params * 4
        
        if str_len != exp_len:
            raise(IndexError('Expected string of size %d, got %d' %(exp_len, str_len)))

        if num_params > 0:
            p_fmt = '<%di' %(num_params)
            self._params = list(struct.unpack(p_fmt, bytestr[12:]))
            self._fmt += p_fmt[1:]

    def pack(self):
        header = _PyStructure.pack(self)
        p_fmt = '<%di' % (len(self.params))
        return header + struct.pack(p_fmt, *self.params)

    @property
    def params(self):
        return self._params[:]

    @params.setter
    def params(self, param_list):
        self.length = 12 + 4*len(param_list)
        self._params = param_list



class DataContainer(_PyStructure):


    def __init__(self, bytestr=None):

        header = [  
                    ('length',            'I'),
                    ('type',              'H'),
                    ('code',              'H'),
                    ('transaction_id',    'I'),
                 ]

        _PyStructure.__init__(self, header, endian='<')

        self.type = PTP_CONTAINER_TYPE.DATA
        self._data = ''

        if bytestr is not None:
            self.unpack(bytestr)

    def unpack(self, bytestr):
        
        _PyStructure.unpack(self, bytestr[:12])

        self._data = bytestr[12:]

    def pack(self):
        header = _PyStructure.pack(self)

        return header + self.data

    @property
    def data(self):
        return self._data

    @data.setter
    def data(self, bytestr):
        self.length = 12 + len(bytestr)
        self._data = bytestr
