cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(gz_usd0 VERSION 0.0.1)

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
    find_package(sdformat14 REQUIRED)
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
