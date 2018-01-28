#!/bin/sh
# script ran by travis inside docker container
set -ex

# Build with address and undefinedb behavior sanitizer
BUILDDIR=/marjoram/ci_build
mkdir -p $BUILDDIR
cd $BUILDDIR
export COVERALLS_REPO_TOKEN=nxEShAdWvkxe8TUHMWWuHsqWlp6EixHOL
cmake -DSANITIZE_ADDRESS=ON -DSANITIZE_UNDEFINED=ON -DCOVERALLS=ON /marjoram
make testrun -j4
make coveralls
