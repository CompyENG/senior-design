#!/usr/bin/python

import time
import threading
import random
from ptp2 import *
import socket
import chdkimage
import pygame
from PIL import Image

# Set up camera
cam = CHDKCamera(util.list_ptp_cameras()[0])
cam.execute_lua("switch_mode_usb(1)")
time.sleep(1)
cam.execute_lua("set_prop(121, 1)") # Set flash to manual adjustment
time.sleep(0.5)
cam.execute_lua("set_prop(143, 2)") # Property 16 is flash mode, 2 is off
time.sleep(0.5)

img = chdkimage.convertColorspace(cam.get_live_view_data()[1].vp_data, 0, 360, 480)
print len(img) # Image length is always 518400 bytes = 506.25 kb

img_pil = Image.frombuffer("RGB", (360, 240), img, "raw", "RGB", 0, 1)
img_pil.save("/home/bobby/Desktop/test-pil.jpg", "jpeg")
print "Saved with PIL"

pygame.init()
img_pygame = pygame.image.frombuffer(img[:len(img)/2], (360, 240), "RGB")
windowSurface = pygame.display.set_mode((0,0))
#pygame.transform.scale(img_pygame, windowSurface.get_size(), windowSurface) -- Doesn't work -- not same format?
#pygame.transform.smoothscale(img_pygame, windowSurface.get_size(), windowSurface) -- Kinda works? Doesn't seem to scale correctly, changes to B/W?
# Seems to be the only way to scale successfully:
new_img = pygame.transform.scale(img_pygame, windowSurface.get_size())
windowSurface.blit(new_img, (0,0))

pygame.display.update()
time.sleep(1)
print "Displayed with PyGame"

cam.close()


# TODO: Ensure that we always get length of 518400 out of convertColorspace BEFORE sending?
# TODO: Grab and process image in different thread? This will make the SERVER threaded.
#        Have a shared variable which contains the latest image.  That way, we should be able to
#        process the joystick right away, but always have an image ready to send
# TODO: Since our send length is predictable, change our receive length in the client
