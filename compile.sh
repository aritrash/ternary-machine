#!/usr/bin/env bash

echo "Compiling..."
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure

