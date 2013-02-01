#!/usr/bin/env python

"""
This program sends joystick data to the server
"""
import pygame
import socket
import sys

#setup socket
#host = 'localhost'
host = '192.168.1.1'
port = 50000
size = 1024

#test socket
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host,port))
except socket.error, (value,message):
    if s:
        s.close()
    print "Could not open socket: " + message
    sys.exit(1) 
print 'connected to port', port, 'on', host
s.close()
pygame.init()

# For "live" video output
size = (0,0)
screen = pygame.display.set_mode(size, pygame.FULLSCREEN)

# Used to manage how fast the screen updates
clock=pygame.time.Clock()

done=False

# Count the joysticks the computer has
joystick_count=pygame.joystick.get_count()
if joystick_count == 0:
    # No joysticks!
    print ("Error, I didn't find any joysticks.")
    sys.exit(1)
else:
    # Use joystick #0 and initialize it
    my_joystick = pygame.joystick.Joystick(0)
    my_joystick.init()
    buttons = my_joystick.get_numbuttons()
    hats = my_joystick.get_numhats()
    axis = my_joystick.get_numaxes()
    print "Buttons:",buttons,"Hats:",hats,"Axis:",axis
    z = 0
while done==False:
    # ALL EVENT PROCESSING SHOULD GO BELOW THIS COMMENT
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            done=True
    # ALL EVENT PROCESSING SHOULD GO ABOVE THIS COMMENT
    # ALL GAME LOGIC SHOULD GO BELOW THIS COMMENT
    # As long as there is a joystick
    if joystick_count != 0:
        command = []
        #forward/backward
        command.append(round(my_joystick.get_axis(1)))
        #right/left
        command.append(round(my_joystick.get_axis(3)))
        #pitch
        command.append(round(my_joystick.get_axis(4)))
        #zoom in
        command.append(max(0,round(my_joystick.get_axis(2))))
        #zoom out
        command.append(max(0,round(my_joystick.get_axis(5))))
        #descend
        command.append(my_joystick.get_button(0))
        #ascend
        command.append(my_joystick.get_button(3))
        #take picture
        command.append(my_joystick.get_button(5))
        
        command.append("E")
        
        #print command
        
        #for i in range(0,11):
        #    command.append("B"+str(i)+":"+str(my_joystick.get_button(i)))
        
        #don't send data if no commands
        zfound = 0
        for i in range(0,len(command)-1):
            if command[i] != 0:
                zfound = 1
        if zfound==1 or z==0 or True:
            if zfound==0:
                z=1
            else:
                z=0
            comm = ""
            for i in range(0,len(command)-1):
                comm += str(command[i])+"||"
            comm += str(command[len(command)-1])
            
            try:
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.connect((host,port))
            except socket.error, (value,message):
                if s:
                    s.close()
                print "Could not open socket: " + message
                sys.exit(1) 
            s.sendall(comm)
            #data = []
            #while True:
            #    r = s.recv(size)
            #    if not r:
            #        break
            #    else:
            #        data.append(r)
            #i=0
            # Check if we're going to get live view data
            r = recv(1)
            if r == "1":
                r = ""
                # TODO: Look at recvfrom_into/recv_into -- it'll give me the number of bytes received
                while len(r) < 259200:
                    r = r+s.recv(4096)
                    #i = i+1
                #r = s.recv(259200) # We should only actually receive 259200
                #print len(r)
                # TODO: Make this happen in a different thread?
                # This is horribly inefficient -- but PyGame didn't want to read the image data
                #im = Image.frombuffer("RGB", (360, 240), r, 'raw', "RGB", 0, 1)
                #im.save("/tmp/live.jpg", 'jpeg')
                #sys.stderr.write("".join(data))
                #img = pygame.image.frombuffer(r, (720, 240), "RGB")
                #img = pygame.image.load("/tmp/live.jpg")
                #img = pygame.transform.scale(img, screen.get_size())
                #screen.blit(img, (0,0))
                #print "Length: %d ; Last character: %d" % (len(r), ord(r[-1]))
                img_data = r
                if len(img_data) == 259200: # TODO: Do we need this anymore since the receive loop was changed?
                    img = pygame.image.frombuffer(r, (360, 240), "RGB")
                    new_img = pygame.transform.scale(img, screen.get_size())
                    screen.blit(new_img, (0,0))
                    #pygame.display.flip()
                    pygame.display.update()
            s.close()
    
    # ALL GAME LOGIC SHOULD GO ABOVE THIS COMMENT
    clock.tick(40)
pygame.quit ()
