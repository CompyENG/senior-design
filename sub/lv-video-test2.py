#!/usr/bin/python
## {{{ http://code.activestate.com/recipes/82965/ (r1)
"""
This recipe describes how to handle asynchronous I/O in an environment where
you are running Tkinter as the graphical user interface. Tkinter is safe
to use as long as all the graphics commands are handled in a single thread.
Since it is more efficient to make I/O channels to block and wait for something
to happen rather than poll at regular intervals, we want I/O to be handled
in separate threads. These can communicate in a threasafe way with the main,
GUI-oriented process through one or several queues. In this solution the GUI
still has to make a poll at a reasonable interval, to check if there is
something in the queue that needs processing. Other solutions are possible,
but they add a lot of complexity to the application.

Created by Jacob Hallen, AB Strakt, Sweden. 2001-10-17
"""
import Tkinter
import time
import threading
import random
import Queue
from ptp2 import *
import chdkimage
import Image, ImageTk

class GuiPart:
    def __init__(self, master, queue, endCommand, camActionHandler):
        self.queue = queue
        self.camActionHandler = camActionHandler # A function to which we can pass arbitrary lua commands
        # Set up the GUI
        self.master = master  # self.master is the root window
        
        # Frame for our buttons -- so we can pack left/right and top/bottom
        #  Probably better done by properly using a grid eventually
        ButtonFrame = Tkinter.Frame(master)
        # Set up some simple buttons
        btnDone = Tkinter.Button(ButtonFrame, text='Done', command=endCommand)
        btnDone.pack(side=Tkinter.LEFT)
        btnZoomIn = Tkinter.Button(ButtonFrame, text='Zoom In', command=self.zoomIn)
        btnZoomIn.pack(side=Tkinter.LEFT)
        btnZoomOut = Tkinter.Button(ButtonFrame, text='Zoom Out', command=self.zoomOut)
        btnZoomOut.pack(side=Tkinter.LEFT)
        btnShoot = Tkinter.Button(ButtonFrame, text='Shoot', command=self.shoot)
        btnShoot.pack(side=Tkinter.LEFT)
        
        ButtonFrame.pack()
        
        self.label_image = Tkinter.Label(master, image=None)
        self.label_image.pack()
        
        self.master.image = None
        
    # The way I'm writing these, they may be blocking... perhaps I need another queue?
    def zoomIn(self):
        self.camActionHandler("click('zoom_in')")
        
    def zoomOut(self):
        self.camActionHandler("click('zoom_out')")
        
    def shoot(self):
        self.camActionHandler("shoot()")

    def processIncoming(self):
        """
        Handle all the messages currently in the queue (if any).
        """
        while self.queue.qsize():
            try:
                #self.label_image.destroy()
                self.image = self.queue.get(0)
                #print "Got image: ", self.image
                # Check contents of message and do what it says
                # As a test, we simply print it
                self.master.geometry('%dx%d' % (self.image.size[0],self.image.size[1]))
                self.master.image = ImageTk.PhotoImage(self.image)
                #self.label_image = Tkinter.Label(self.master, image=tkpi)
                #self.label_image.pack()
                #print self.image
                self.label_image.config(image=self.master.image)
                self.label_image.update_idletasks()
                self.master.update_idletasks()
                #print "Attempted image update."
            except Queue.Empty:
                pass

class ThreadedClient:
    """
    Launch the main part of the GUI and the worker thread. periodicCall and
    endApplication could reside in the GUI part, but putting them here
    means that you have all the thread controls in a single place.
    """
    def __init__(self, master):
        """
        Start the GUI and the asynchronous threads. We are in the main
        (original) thread of the application, which will later be used by
        the GUI. We spawn a new thread for the worker.
        """
        self.master = master

        # Create the queue
        self.queue = Queue.Queue()
        
        # Set up camera
        self.cam = CHDKCamera(util.list_ptp_cameras()[0])
        self.cam.execute_lua("switch_mode_usb(1)")
        time.sleep(1)
        self.cam.execute_lua("set_prop(121, 1)") # Set flash to manual adjustment
        time.sleep(0.5)
        self.cam.execute_lua("set_prop(143, 2)") # Property 16 is flash mode, 2 is off
        time.sleep(0.5)
        self.camUsed = False # This will be a little semaphore for us

        # Set up the GUI part
        self.gui = GuiPart(master, self.queue, self.endApplication, self.camActionHandler)

        # Set up the thread to do asynchronous I/O
        # More can be made if necessary
        self.running = 1
    	self.thread1 = threading.Thread(target=self.workerThread1)
        self.thread1.start()

        # Start the periodic call in the GUI to check if the queue contains
        # anything
        self.periodicCall()

    def periodicCall(self):
        """
        Check every 100 ms if there is something new in the queue.
        """
        self.gui.processIncoming()
        if not self.running:
            # This is the brutal stop of the system. You may want to do
            # some cleanup before actually shutting it down.
            print "Should exit"
            import sys
            self.cam.close()
            sys.exit(1)
        self.master.after(75, self.periodicCall)

    def workerThread1(self):
        """
        This is where we handle the asynchronous I/O. For example, it may be
        a 'select()'.
        One important thing to remember is that the thread has to yield
        control.
        """
        while self.running:
            # To simulate asynchronous I/O, we create a random number at
            # random intervals. Replace the following 2 lines with the real
            # thing.
            time.sleep(0.1) # Don't update too quickly
            #msg = rand.random()
            while self.camUsed:
                pass # Wait for camera to become available
            self.camUsed = True
            im = Image.frombuffer("RGB", (360, 240), chdkimage.convertColorspace(self.cam.get_live_view_data()[1].vp_data, 0, 360, 480), 'raw', "RGB", 0, 1)
            self.camUsed = False
            self.queue.put(im)

    def endApplication(self):
        self.running = 0
        self.cam.close()
        
    def camActionHandler(self, act):
        while self.camUsed:
            pass # Wait for camera to become available
        self.camUsed = True
        self.cam.execute_lua(act)
        self.camUsed = False

#rand = random.Random()
root = Tkinter.Tk()

client = ThreadedClient(root)
root.mainloop()
## end of http://code.activestate.com/recipes/82965/ }}}

