#!/bin/bash
set -e

# Clean build artifacts without removing the volume mount point


if [ ! -d "$BUILD_DIR" ]; then
    echo "Making build dir"
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G "Ninja"
fi


cmake --build build -- -j "$(nproc)"

# Symlink compile_commands.json to root so clangd can find it
ln -sf build/compile_commands.json .

echo "Finished building"
