#!/bin/sh -l

set -x

BUILD_DIR=`pwd`

cd /tmp

USD_VERSION="24.08"

# compile USD from sources
apt install libpyside2-dev python3-opengl cmake libglu1-mesa-dev freeglut3-dev mesa-common-dev libx11-dev -y
wget https://github.com/PixarAnimationStudios/USD/archive/refs/tags/v$USD_VERSION.zip
unzip v$USD_VERSION.zip
cd OpenUSD-$USD_VERSION
python3 build_scripts/build_usd.py --build-variant release --no-tests --no-imaging --no-examples --no-tutorials --no-docs --onetbb --no-python /tmp/USD

cd $BUILD_DIR
