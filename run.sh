#!/usr/bin/env sh

set -eu

cd "$(dirname "$0")"
rm -rf build
cmake -S . -B build
cmake --build build
./build/DirimEngine