#!/bin/bash
set -e

# Clean build artifacts without removing the volume mount point


if [ ! -d "$BUILD_DIR" ]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G "Ninja"
fi


cmake --build build --target my_b -- -j "$(nproc)"


echo "Finished building"
