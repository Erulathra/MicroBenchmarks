#!/bin/bash

if [ ! -d "./build" ]; then
   mkdir build
fi

common_params="-O2 -mavx2 -std=c++20 -fno-rtti -fno-exceptions -Wall -Wno-unused-variable -Werror"

# compile bitset benchmark
clang++ BitsetBench.cpp -o build/BitsetBench $common_params
chmod +x build/BitsetBench
