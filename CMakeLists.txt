cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project("Kuan")

add_executable(kuan src/classes.cpp src/KuanVM.cpp src/Parser.cpp src/linenoise/linenoise.c src/main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -Wno-switch -Wno-unused-function -Wno-unused-variable")