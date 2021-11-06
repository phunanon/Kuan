#!/bin/sh

[ -d build ] && rm -rf build
mkdir build

#Download immer
git clone --depth 1 --no-checkout https://github.com/arximboldi/immer.git
cd immer
git checkout origin/master -- immer
mkdir ../src/immer
mv immer ../src/immer/immer
cd ..
rm -rf immer

#Build Kuan
cd build
cmake ..
cd ..
sh make.sh