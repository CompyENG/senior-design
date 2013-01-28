#!/usr/bin/python
# Still need to test this code. After some more research, this might work for getting the liveview data
#  It's possible that chdkimage and/or ptp2 are using outdated versions of the liveview header, which means
#  I'll need to update the code. But, this is a good first attempt.  If nothing else, hopefully this will get
#  me a garbage image -- I'm just hoping to generate SOMETHING...

from ptp2 import *
import chdkimage
import Image

cameras = util.list_ptp_cameras()
cam = CHDKCamera(cameras[0])

cam.execute_lua("switch_mode_usb(1)")

lv_data = cam.get_live_view_data(True, True, True)[1]
width = lv_data.vp_desc.visible_width
height = lv_data.vp_desc.visible_height
vp_rgb = chdkimage.dataToViewportRGB(lv_data.vp_data.tostring(), 0)

vp_im = Image.fromstring("RGB", (width, height), vp_rgb, 'raw', 'RGB')
vp_im.save('/home/bobby/test.jpg')