#!/bin/bash
set -e
ROOT="$(pwd)"
CC=gcc
CXX=g++

# SR
echo -e "[+] SR"
cd src/sr
mkdir build
cd build
cmake .. --preset linux
cmake --build . --target install
cd $ROOT

# CoD4x
echo -e "[+] CoD4x"
make
