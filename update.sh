#!/bin/bash

# This script is the update script which will be run on the Pi after the update
# package is extracted.  This script should compile all the items, and copy the
# files into their correct locations.  Additionally, this script can communicate
# with the other Pi to copy files to it over SFTP.

# Build all necessary files
bash ./libs/build-lib.sh
bash ./sd-submarine/build-submarine.sh
bash ./sd-surface/build-surface.sh

# Copy files to correct locations
# Library files
cp ./libs/libptp++.so /usr/lib/
ldconfig # Run ldconfig to update the library cache
cp ./libs/libptp++.hpp /usr/include/
# Submarine file
cp ./sd-submarine/sd-submarine /usr/bin/
# Surface file
cp ./sd-surface/sd-surface /usr/bin/
# util files
cp ./utils/90-senior-design.rules /etc/udev/rules.d/
cp ./utils/sd-start /usr/sbin/
cp ./utils/sd-stop /usr/sbin/
cp ./utils/sd-update /usr/sbin/
cp ./utils/sd-startup /etc/init.d/

# Update init.d files
update-rc.d sd-startup defaults

# Restart udev to pickup new rules
/etc/init.d/udev restart

MY_HOSTNAME=`hostname -s`
if [ $MY_HOSTNAME -eq "pi-surface" ]; then
    OTHER_HOSTNAME="pi-submarine"
else
    OTHER_HOSTNAME="pi-surface"
fi

# TODO: SFTP. An SFTP batch file will simply run SFTP commands.
# I will also need to set up public key authentication so that we can SFTP
# without providing credentials