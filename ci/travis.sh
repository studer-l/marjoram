#!/bin/sh
# script ran by travis inside docker container
set -ex

BUILDDIR=/marjoram/ci_build

nuke_build_dir() {
  cd /
  [ -d $BUILDDIR ] && rm -rf $BUILDDIR
  mkdir -p $BUILDDIR
  cd $BUILDDIR
}

sanitizer_build() {
  nuke_build_dir
  # Build with address and undefined behavior sanitizer
  export CXX=clang++
  cmake -DSANITIZE_ADDRESS=ON -DSANITIZE_UNDEFINED=ON /marjoram
  make testrun
  cd ..
}

# coverage has to be done with gcc, dont ask
coveralls() {
  nuke_build_dir
  export COVERALLS_REPO_TOKEN=nxEShAdWvkxe8TUHMWWuHsqWlp6EixHOL
  export CXX=g++
  cmake -DCOVERALLS=ON /marjoram
  make testrun
  make coveralls
}

sanitizer_build && coveralls
