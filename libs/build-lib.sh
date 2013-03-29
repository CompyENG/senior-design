#!/bin/bash

# This script is responsible for building the libptp++ shared library.  This
# will only be run on the Pi, so we are free to perform build optimizations.

pwd
g++ -shared -fPIC CameraBase.cpp CHDKCamera.cpp LVData.cpp PTPCamera.cpp PTPContainer.cpp PTPUSB.cpp PTPNetwork.cpp -o libptp++.so -lusb-1.0

echo "g++ status: $?"
