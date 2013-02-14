#!/bin/bash

# This script is responsible for building the libptp++ shared library.  This
# will only be run on the Pi, so we are free to perform build optimizations.

g++ -shared -fPIC libptp++.cpp -o libptp++.so -lusb-1.0
