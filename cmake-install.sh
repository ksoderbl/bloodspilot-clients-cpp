#!/bin/sh

set -e

BUILD_DIR=build-cmake

cmake -S . -B "$BUILD_DIR" "$@"
cmake --build "$BUILD_DIR"
cmake --install "$BUILD_DIR"
