#!/bin/bash

BUILD_DIR="build"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Making build dir"
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G "Ninja"
fi

cmake --build build -- -j "$(nproc)"
echo "Finished building"
