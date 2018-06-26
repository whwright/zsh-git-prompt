#!/bin/sh
BUILD_D="$(dirname $(readlink -f "$0"))/build"

echo "This will build the c++ based version of gitstatus."
echo "To build, both cmake and a C++11 compliant compiler are required."

command rm -rf "$BUILD_D"
command mkdir -p "$BUILD_D"
cd "$BUILD_D"
cmake .. -DCMAKE_BUILD_TYPE=Release
make
