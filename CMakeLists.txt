cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(ignition-usd0 VERSION 0.0.1)

#============================================================================
# Find ignition-cmake
#============================================================================
# If you get an error at this line, you need to install ignition-cmake
find_package(gz-cmake3 REQUIRED)
#--------------------------------------
# Find ignition-common
gz_find_package(gz-common5 REQUIRED COMPONENTS graphics)
set(GZ_COMMON_VER ${gz-common5_VERSION_MAJOR})
set(GZ_COMMON_LIB gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER})
set(GZ_COMMON_GRAPHICS_LIB gz-common${GZ_COMMON_VER}::graphics)
#--------------------------------------
# Find ignition-utils
gz_find_package(gz-utils2 REQUIRED COMPONENTS cli)
set(GZ_UTILS_VER ${gz-utils2_VERSION_MAJOR})
set(GZ_UTILS_LIB gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER})
set(GZ_UTILS_CLI_LIB gz-utils${GZ_UTILS_VER}::cli)
find_package(sdformat13 REQUIRED)
SET(SDFORMAT_VER 13)

########################################
# Find PXR
gz_find_package(pxr REQUIRED)

#============================================================================
# Configure the project
#============================================================================
set(c++standard 17)
set (CMAKE_CXX_STANDARD 17)
gz_configure_project(VERSION_SUFFIX)

#============================================================================
# Configure the build
#============================================================================
gz_configure_build(QUIT_IF_BUILD_ERRORS)

#============================================================================
# Create package information
#============================================================================
gz_create_packages()
