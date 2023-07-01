#!/bin/sh

# Build USD with MaterialX support on macOS with Metal backend
# first argument is the path to the USD source code

# if two arguments are not supplied, print help
if [ $# -ne 2 ]; then
    echo "Usage: mac_metal.sh <path_to_usd_source> <path_to_materialx_cmake>"
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
-DLABAPP_GRAPHICS_SYSTEM=metal \
-DLABAPP_WINDOWING_SYSTEM=cocoa

make install -j8
