#!/usr/bin/env bash
echo "Configuring Eternum Engine for Ninja/Makefiles..."

# Prefer Ninja if installed
if command -v ninja &> /dev/null; then
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
else
    cmake -B build -DCMAKE_BUILD_TYPE=Debug
fi

echo "Done. Use 'cmake --build build' to compile."
