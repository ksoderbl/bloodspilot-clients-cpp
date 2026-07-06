#!/bin/sh

set -e

BUILD_DIR=build-cmake

cmake -S . -B "$BUILD_DIR" "$@"
cmake --build "$BUILD_DIR" -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
