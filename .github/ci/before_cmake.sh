#!/bin/sh -l

set -x

BUILD_DIR=`pwd`

cd /tmp

# check that we can compile USD from sources (only Focal)
# see https://github.com/ignitionrobotics/sdformat/issues/869
return_code=0
if [ "$(lsb_release -r -s)" == "20.04" ]; then
  USD_VERSION="21.11"
elif [ "$(lsb_release -r -s)" == "22.04" ]; then
  USD_VERSION="22.11"
else
  return_code=$(($return_code + 1))
fi

mkdir cmake_test
cd cmake_test

echo "cmake_minimum_required(VERSION 3.12)" > CMakeLists.txt

cmake . || return_code=$(($return_code + $?))
if [ $return_code -eq 0 ]
then
  # compile USD from sources
  cd /tmp
  mkdir usd_binaries
  cd usd_binaries

  apt-get install libboost-all-dev p7zip-full -y

  wget https://github.com/oneapi-src/oneTBB/archive/refs/tags/2019_U6.tar.gz
  tar -xvf 2019_U6.tar.gz
  cd oneTBB-2019_U6/
  make -j 2
  mkdir -p /tmp/install_tbb/lib
  mkdir -p /tmp/install_tbb/include
  cp build/*_release/libtbb* /tmp/install_tbb/lib
  cp -r include/serial /tmp/install_tbb/include/
  cp -r include/tbb /tmp/install_tbb/include/

  cd ..

  wget https://github.com/PixarAnimationStudios/USD/archive/refs/tags/v$USD_VERSION.zip
  unzip v$USD_VERSION.zip
  cd USD-$USD_VERSION
  mkdir build
  cd build

  cmake -DCMAKE_INSTALL_PREFIX="/tmp/USD" -DCMAKE_PREFIX_PATH="/tmp/USD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DPXR_PREFER_SAFETY_OVER_SPEED=ON \
    -DPXR_ENABLE_PYTHON_SUPPORT=OFF \
    -DBUILD_SHARED_LIBS=ON  \
    -DTBB_USE_DEBUG_BUILD=OFF  \
    -DTBB_ROOT=/tmp/install_tbb \
    -DPXR_BUILD_DOCUMENTATION=OFF  \
    -DPXR_BUILD_TESTS=OFF  \
    -DPXR_BUILD_EXAMPLES=OFF  \
    -DPXR_BUILD_TUTORIALS=OFF  \
    -DPXR_BUILD_USD_TOOLS=OFF  \
    -DPXR_BUILD_IMAGING=OFF  \
    -DPXR_BUILD_USD_IMAGING=OFF  \
    -DPXR_BUILD_USDVIEW=OFF  \
    -DPXR_BUILD_ALEMBIC_PLUGIN=OFF  \
    -DPXR_BUILD_DRACO_PLUGIN=OFF  \
    -DPXR_ENABLE_MATERIALX_SUPPORT=OFF  \
    -DBoost_INCLUDE_DIR=/usr/include  \
    -DBoost_NO_BOOST_CMAKE=FALSE \
    ..

  make -j$(nproc) install
fi

cd $BUILD_DIR
