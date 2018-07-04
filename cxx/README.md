# Gitstatus (C++ Implementation)

Default build is debug.

## Requirements

1. `cmake`, version >= 3.1.0
2. C++ compiler implementing c++11 standard, for example GCC version >= 5.0
3. pthread library for google tests

## To Build

1. `mkdir ./build && cd build`
1. `cmake -DCMAKE_BUILD_TYPE=Release .. && make`

For debug version, just omit -DCMAKE_BUILD_TYPE flag.

## To Test

1. `mkdir ./build && cd build`
1. `cmake .. && make tests`
