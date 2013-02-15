#!/bin/bash

# This script is the update script which will be run on the Pi after the update
# package is extracted.  This script should compile all the items, and copy the
# files into their correct locations.  Additionally, this script can communicate
# with the other Pi to copy files to it over SFTP.

echo "Running update script"

# Build all necessary files
cd ./libs/
bash ./build-lib.sh
# Copy these files first, as they're necessary for later builds
cp ./libptp++.so /usr/lib/
cp ./libptp++.hpp /usr/include/
cp ./live_view.h /usr/include/

cd ../sd-submarine/
bash ./build-submarine.sh
cd ../sd-surface/
bash ./sd-surface/build-surface.sh
cd ../

# Copy files to correct locations
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

# Restart the UI
/etc/init.d/sd-startup stop
/etc/init.d/sd-startup start

MY_HOSTNAME=`hostname -s`
if [ "${MY_HOSTNAME}" = "pi-surface" ]; then
    OTHER_HOSTNAME="pi-submarine"
else
    OTHER_HOSTNAME="pi-surface"
fi

ssh pi@${OTHER_HOSTNAME} << EOF
mkdir -p /tmp/update/usr/lib/
mkdir -p /tmp/update/usr/include/
mkdir -p /tmp/update/usr/bin/
mkdir -p /tmp/update/etc/udev/rules.d/
mkdir -p /tmp/update/usr/sbin/
mkdir -p /tmp/update/etc/init.d/
EOF

# TODO: Set up public key authentication so that we can SFTP/SSH
# without providing credentials
# TODO: I'll need to SFTP into root to have permissions to copy into the right
#  locations, won't I?
#  Alternatively, let's copy these to /tmp, then run a script to move them
#  to the right place :/
sftp pi@${OTHER_HOSTNAME} << EOF
put /usr/lib/libptp++.so /tmp/update/usr/lib/
put /usr/include/libptp++.hpp /tmp/update/usr/include/
put /usr/bin/sd-submarine /tmp/update/usr/bin/
put /usr/bin/sd-surface /tmp/update/usr/bin/
put /etc/udev/rules.d/90-senior-design.rules /tmp/update/etc/udev/rules.d/
put /usr/sbin/sd-start /tmp/update/usr/sbin/
put /usr/sbin/sd-stop /tmp/update/usr/sbin/
put /usr/sbin/sd-update /tmp/update/usr/sbin/
put /etc/init.d/sd-startup /tmp/update/etc/init.d/
EOF

# User pi will need sudo access with NOPASSWD, which is a security risk.
# We're OK with this because neither pi is connected to the Internet, and anyone
# with access has physical access anyway.

# This copy line might not be ideal, but it's fine as long as we set things up
# correctly above.  We can always reimage the SD card if something goes wrong.
ssh pi@${OTHER_HOSTNAME} << EOF
sudo cp -r /tmp/update/* /
sudo /etc/init.d/sd-startup stop
sudo /etc/init.d/sd-startup start
EOF
