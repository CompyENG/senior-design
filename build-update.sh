#!/bin/bash

# This script copies all items in libs/ sd-surface/ and sd-submarine/ into a 
# temp folder, and builds an update package.  The result is an
# update-YYYYMMDD.tar.gz file placed in the same directory as this script.

DATE=`date "+%Y%m%d"`

tar cvf update-${DATE}.tar.gz update.sh libs/* sd-submarine/* sd-surface/* utils/*

