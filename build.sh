#!/bin/bash
set -e

# Clean build artifacts without removing the volume mount point


if [ ! -d "$BUILD_DIR" ]; then
    cmake --preset=default \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Debug \
        -G "Ninja"
fi


cmake --build build --target my_b -- -j "$(nproc)"


echo "Finished building"
