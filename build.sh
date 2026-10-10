#!/bin/bash

if [ ! -d "./build" ]; then
   mkdir build
fi

function compile ()
{
	# debug
	# common_params="-g -O0 -mavx2 -std=c++20 -fno-rtti -fno-exceptions -Wall -Wno-unused-variable -Werror"

	# release
	common_params="-O3 -mavx2 -std=c++20 -fno-rtti -fno-exceptions -Wall -Wno-unused-variable -Werror"

	echo "Compiling $1..."

	clang++ $1.cpp -o build/$1 $common_params
	chmod +x build/BitsetBench
}

compile BitsetBench
compile IteratorsBench
