#!/bin/bash

# This script is responsible for building the sd-submarine.cpp file into an
# executable.  This will only be run on the Pi, so we can perform whatever build
# optimizations we want.

g++ -o sd-submarine submarine.cpp -lusb-1.0 -lptp++ 
