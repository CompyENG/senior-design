#!/bin/sh
g++ -shared -fPIC libptp2.cpp -o libptp2.so -lusb-1.0
