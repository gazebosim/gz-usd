cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(ignition-usd0 VERSION 0.0.1)

#============================================================================
# Find ignition-cmake
#============================================================================
# If you get an error at this line, you need to install ignition-cmake
find_package(ignition-cmake2 REQUIRED)

#--------------------------------------
# Find ignition-common
ign_find_package(ignition-common4 REQUIRED COMPONENTS graphics)
set(IGN_COMMON_VER ${ignition-common4_VERSION_MAJOR})

#--------------------------------------
# Find ignition-utils
ign_find_package(ignition-utils1 REQUIRED COMPONENTS cli)
set(IGN_UTILS_VER ${ignition-utils1_VERSION_MAJOR})

########################################
# Find PXR
ign_find_package(pxr REQUIRED)

find_package(sdformat12 REQUIRED)

#============================================================================
# Configure the project
#============================================================================
set(c++standard 17)
set (CMAKE_CXX_STANDARD 17)
ign_configure_project(VERSION_SUFFIX)

#============================================================================
# Configure the build
#============================================================================
ign_configure_build(QUIT_IF_BUILD_ERRORS)

#============================================================================
# Create package information
#============================================================================
ign_create_packages()
