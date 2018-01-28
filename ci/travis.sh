#!/bin/sh
# script ran by travis inside docker container
set -ex

# Build with address and undefinedb behavior sanitizer
BUILDDIR=/build
mkdir -p $BUILDDIR
cd $BUILDDIR
cmake -DSANITIZE_ADDRESS=ON -DSANITIZE_UNDEFINED=ON /marjoram
make testrun -j4
