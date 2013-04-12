#!/bin/bash

# This script copies all items in libs/ sd-surface/ and sd-submarine/ into a 
# temp folder, and builds an update package.  The result is an
# update-YYYYMMDD.tar.gz file placed in the same directory as this script.

DATE=`date "+%Y%m%d"`
BRANCH=`git rev-parse --abbrev-ref HEAD`

# The git archive will contain some files we don't care about (build-update.sh,
#  README.md, .gitattributes, .gitignore), but that's OK. Our update script will
#  only deal with the ones we want, and the added files aren't much overhead
git archive --format=tar.gz -o update-${DATE}.tar.gz ${BRANCH}
