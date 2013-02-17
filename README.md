senior-design
=============

A repository for work on my senior design project: a submarine with a camera.

The brains of the project are two Raspberry Pis: one on the surface, one on the 
submarine.  The surface Raspberry Pi is connected to a gamepad (USB) and a small
display.  The submarine Raspberry Pi is connected to a camera running CHDK (USB)
and the motor controllers (GPIO).  The Pis are connected to each other via
ethernet.

The Pi on the surface has the hostname "pi-surface", the Pi on the submarine has
the hostname "pi-submarine".  Each Pi is set up with a static IP address, and
each Pis hosts file has an entry for itself and the other Pi.  This way, they
can connect with hostnames in case we decide to change IPs.

Other than the dependencies of this code, this repository contains everything 
needed to initially set up this system, as well as maintain the system going
forward.

Initial Setup
=============

The initial setup of the system is minimal, and most of the setup is done by an
update script run from a USB drive once the initial setup is complete.  The 
initial setup consists of copying utils/90-senior-design.rules to 
/etc/udev/rules.d and utils/sd-update to /usr/sbin on at least one of the Pis.
Additionally, the network setup described above will need to be completed, and 
each Pi will need to be given public key authentication to the other Pi.  
Finally, all dependencies of this project must be installed.

Once this is completed, you can build an update package (build-update.sh), place
it on a thumb drive, and plug the thumb drive into the Pi.  The update script 
will build all necessary parts, copy these parts to the right location, and send
these files over to the other Pi.

Dependencies
============

* libSDL(-dev)
* libusb1.0(-dev)
* [bcm2835](http://www.open.com.au/mikem/bcm2835/)
* C++ compiler (g++)

Any other files necessary should be in this repository. This project uses a copy
of [libptp++](http://github.com/TrueJournals/libptp--), which was developed for
this project.

File Organization
=================

File organization is designed to be as simple as possible:

* common/ -- Contains any code common between the submarine and surface
* libs/ -- Contains any libraries used by submarine, surface, or both that must 
be compiled
* sd-submarine/ -- Contains any code specific to the submarine
* sd-surface/ -- Contains any code specific to the surface
* utils/ -- Contains all scripts that will be used on the submarine, surface, or
both
* build-update.sh -- Builds an update package
* update.sh -- The script ran on the Pi after the update package is extracted

Note that each folder which contains a compilable unit also contains a build-
script to build that part of the project.  These scripts are called by update.sh

Repository Organization
=======================

Bobby will maintain this repository, and dictate its usage.

We'll keep master as good, working code.  Branch organization:
 - (name)-master -- each person's master branch
 - (name)-(feature) -- branch for each person working on a feature
 
Everyone has read/write access, so please keep with creating branches and merging branches on your own.

Once you have good working code, you can send a pull request to master.
