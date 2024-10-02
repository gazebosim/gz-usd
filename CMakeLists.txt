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

<<<<<<< HEAD
=======
if("$ENV{GZ_VERSION}" STREQUAL "harmonic")
    find_package(gz-cmake3 REQUIRED)
    #--------------------------------------
    # Find gz-common
    gz_find_package(gz-common5 REQUIRED COMPONENTS graphics)
    set(GZ_COMMON_VER ${gz-common5_VERSION_MAJOR})
    set(GZ_COMMON_LIB gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER})
    set(GZ_COMMON_GRAPHICS_LIB gz-common${GZ_COMMON_VER}::graphics)
    #--------------------------------------
    # Find gz-utils
    gz_find_package(gz-utils2 REQUIRED COMPONENTS cli)
    set(GZ_UTILS_VER ${gz-utils2_VERSION_MAJOR})
    set(GZ_UTILS_LIB gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER})
    set(GZ_UTILS_CLI_LIB gz-utils${GZ_UTILS_VER}::cli)
    # cone shape was added in 14.4
    find_package(sdformat14 14.4 REQUIRED)
    SET(SDFORMAT_VER 14)
else()
    find_package(gz-cmake4 REQUIRED)
    #--------------------------------------
    # Find gz-common
    gz_find_package(gz-common6 REQUIRED COMPONENTS graphics)
    set(GZ_COMMON_VER ${gz-common6_VERSION_MAJOR})
    set(GZ_COMMON_LIB gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER})
    set(GZ_COMMON_GRAPHICS_LIB gz-common${GZ_COMMON_VER}::graphics)
    #--------------------------------------
    # Find gz-utils
    gz_find_package(gz-utils3 REQUIRED COMPONENTS cli)
    set(GZ_UTILS_VER ${gz-utils3_VERSION_MAJOR})
    set(GZ_UTILS_LIB gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER})
    set(GZ_UTILS_CLI_LIB gz-utils${GZ_UTILS_VER}::cli)
    find_package(sdformat15 REQUIRED)
    SET(SDFORMAT_VER 15)
endif()
>>>>>>> 330f1c9 (Add support for cone shapes (#26))
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
