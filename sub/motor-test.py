#!/usr/bin/python

import RPi.GPIO as GPIO
import time

# Motor 1: 18, 23
# Motor 2: 24, 25
# Motor 3: 8, 7
# Motor 4: 17, 27

OFF = 0
FORWARD = 1
REVERSE = 2

for pin in (18, 23, 24, 25, 8, 7, 17, 27):
	GPIO.setup(pin, GPIO.OUT)
	
motors = {(18,23): OFF, (24, 25): FORWARD, (8, 7): REVERSE, (17, 27): OFF}
while True:
	for pins in motors:
		if state == OFF:
			for pin in pins:
				GPIO.output(pin, False) # Turn both outputs off
		elif state == FORWARD:
			GPIO.output(pins[0], True)
			GPIO.output(pins[1], False)
		elif state == REVERSE:
			GPIO.output(pins[0], False)
			GPIO.output(pins[1], True)
		motors[pins] = (motors[pins]+1)%3
	time.sleep(1) #Sleep for one second