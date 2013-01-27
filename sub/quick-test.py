#!/usr/bin/env python
from ptp2 import *
import chdkimage
import Image
import time

print "Connecting to camera"
cam = CHDKCamera(util.list_ptp_cameras()[0])

print "Switching to record mode"
cam.execute_lua("switch_mode_usb(1)")
time.sleep(1)

print "Getting image..."
start = time.clock()
im = Image.frombuffer("RGB", (360, 240), chdkimage.convertColorspace(cam.get_live_view_data()[1].vp_data, 0, 360, 480), 'raw', "RGB", 0, 1)
end = time.clock()
print "Took %f to get and convert" % (end-start,)
im.show()
