#!/bin/sh

# Build USD with MaterialX support on macOS with Metal backend
# first argument is the path to the USD source code

# if two arguments are supplied, the first is USD_ROOT, the second is
# MATERIALX_CMAKE. If one argument is supplied, deduce MATERIALX_CMAKE
# by appending /lib/cmake to USD_ROOT, and testing for the existence of
# MaterialX/MaterialXConfig.cmake in that directory. If it's not there,
# print help and exit, otherwise print out that we're using the deduced
# MATERIALX_CMAKE. If no arguments are supplied, print help and exit.
# If more than two arguments are supplied, print help and exit.

if [ $# -gt 2 ]; then
    echo "Usage: mac_metal.sh <path_to_usd_source> <path_to_materialx_cmake>"
    exit 1
fi
if [ $# -eq 2 ]; then
    USD_SOURCE=$1
    MATERIALX_CMAKE=$2
    if [ ! -f $MATERIALX_CMAKE/MaterialX/MaterialXConfig.cmake ]; then
        echo "MaterialXConfig.cmake not found in $MATERIALX_CMAKE/MaterialX"
        echo "Please supply the path to MaterialX cmake directory as the second argument"
        exit 1
    fi
    echo "Using supplied MATERIALX_CMAKE: $MATERIALX_CMAKE"
fi
if [ $# -eq 1 ]; then
    USD_SOURCE=$1
    MATERIALX_CMAKE=$USD_SOURCE/lib/cmake
    if [ ! -f $MATERIALX_CMAKE/MaterialX/MaterialXConfig.cmake ]; then
        echo "MaterialXConfig.cmake not found in $MATERIALX_CMAKE/MaterialX"
        echo "Please supply the path to MaterialX cmake directory as the second argument"
        exit 1
    fi
    echo "Using deduced MATERIALX_CMAKE: $MATERIALX_CMAKE"
fi
if [ $# -eq 0 ]; then
    echo "Usage: mac_metal.sh <path_to_usd_source> <path_to_materialx_cmake>"
    echo "       if matterialx's cmake files are at <usd_source>/lib/cmake, then"
    echo "       <path_to_materialx_cmake> can be omitted"
    exit 1
fi

# second argument is the path to the MaterialX cmake directory
# get the arguments into a variable:
USD_SOURCE=$1
MATERIALX_CMAKE=$2

mkdir -p build_mac_metal
cd build_mac_metal

cmake .. -Dpxr_DIR=$USD_SOURCE -DMaterialX_DIR=$MATERIALX_CMAKE \
-DCMAKE_INSTALL_PREFIX=../install_mac_metal \
-DCMAKE_PREFIX_PATH=$MATERIALX_CMAKE\;$USD_SOURCE \
-DLABAPP_GRAPHICS_SYSTEM=metal \
-DLABAPP_WINDOWING_SYSTEM=cocoa \
-G Xcode

cmake --build . --config RelWithDebInfo --target install

# who knows how to get cmake to properly install an rpath, the docs are insane
install_name_tool -add_rpath "$USD_SOURCE/lib" ../install_mac_metal/bin/LabUsd

# and codesign because getting cmake to do it is a pain
codesign --force --deep -s - ../install_mac_metal/bin/LabUsd
