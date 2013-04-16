#!/bin/bash

# This script is responsible for building the sd-surface.cpp file into an
# executable.  This will only be run on the Pi, so we can perform whatever build
# optimizations we want.

pwd
g++ -o sd-surface -O2 surface.cpp SubJoystick.cpp ../common/SignalHandler.cpp -lSDL -lptp++

echo "g++ status: $?"
