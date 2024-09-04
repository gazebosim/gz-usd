#!/bin/sh -l

set -x

BUILD_DIR=`pwd`

cd /tmp

# check that we can compile USD from sources (only Focal)
# see https://github.com/ignitionrobotics/sdformat/issues/869
return_code=0
USD_VERSION="24.08"

mkdir cmake_test
cd cmake_test

echo "cmake_minimum_required(VERSION 3.12)" > CMakeLists.txt

cmake . || return_code=$(($return_code + $?))
if [ $return_code -eq 0 ]
then
  # compile USD from sources
  cd /tmp

  apt install libpyside2-dev python3-opengl cmake libglu1-mesa-dev freeglut3-dev mesa-common-dev libx11-dev

  wget https://github.com/PixarAnimationStudios/USD/archive/refs/tags/v$USD_VERSION.zip
  unzip v$USD_VERSION.zip
  cd OpenUSD-$USD_VERSION
  python3 build_scripts/build_usd.py --build-variant release --no-tests --no-imaging --no-examples --no-tutorials --no-docs --onetbb --no-python /tmp/USD
fi

cd $BUILD_DIR
