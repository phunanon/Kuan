#!/bin/sh

[ -d build ] && rm -rf build
mkdir build

#Build Kuan
cd build
cmake ..
cd ..
sh make.sh