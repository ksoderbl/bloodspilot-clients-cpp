#!/bin/sh

set -e

BUILD_DIR=build-cmake-x11
BINARY="$BUILD_DIR/src/client/x11/bloodspilot-client-x11"

if test -e "$BINARY"; then
    mv "$BINARY" "$BINARY.old"
fi

cmake -S . -B "$BUILD_DIR" \
    -DBLOODSPILOT_BUILD_454_CLIENT=OFF \
    -DBLOODSPILOT_BUILD_X11_CLIENT=ON \
    -DBLOODSPILOT_BUILD_REPLAY=OFF \
    "$@"

cmake --build "$BUILD_DIR" -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"

echo ""
if test -e "$BINARY"; then
    echo "Built $BINARY successfully."
else
    echo "Failed to build $BINARY !!!"
    exit 1
fi
echo ""
