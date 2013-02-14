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
# TODO: I'll need to SFTP into root to have permissions to copy into the right
#  locations, won't I?
#  Alternatively, let's copy these to /tmp, then run a script to move them
#  to the right place :/
sftp pi@${OTHER_HOSTNAME} << EOF
put /usr/lib/libptp++.so /usr/include/
put /usr/include/libptp++.hpp /usr/include/
put /usr/bin/sd-submarine /usr/bin/
put /usr/bin/sd-surface /usr/bin/
put /etc/udev/rules.d/90-senior-design.rules /etc/udev/rules.d/
put /usr/sbin/sd-start /usr/sbin/
put /usr/sbin/sd-stop /usr/sbin/
put /usr/sbin/sd-update /usr/sbin/
put /etc/init.d/sd-startup /etc/init.d/
EOF
