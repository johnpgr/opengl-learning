#!/bin/bash

# Exit on any error
set -e

# Set project directories
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$PROJECT_ROOT/src"
BUILD_DIR="$PROJECT_ROOT/build"
THIRDPARTY_DIR="$PROJECT_ROOT/thirdparty"

# Set GLAD paths
GLAD_DIR="$THIRDPARTY_DIR/glad"
GLAD_INCLUDE_DIR="$GLAD_DIR/include"
GLAD_SRC_DIR="$GLAD_DIR/src"

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Check if GLAD source exists
if [ ! -f "$GLAD_SRC_DIR/glad.c" ]; then
    echo "Error: GLAD source not found at $GLAD_SRC_DIR/glad.c"
    exit 1
fi

# Compile GLAD as C first
echo "Compiling GLAD..."
clang -c -I"$GLAD_INCLUDE_DIR" \
    -o "$BUILD_DIR/glad.o" \
    "$GLAD_SRC_DIR/glad.c" \
    -MD

echo "Compiling application..."

# Compile the application using system SDL3
clang++ -std=c++23 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -Wno-c23-extensions \
    -Wno-language-extension-token \
    -I"$GLAD_INCLUDE_DIR" \
    $(pkg-config --cflags sdl3 2>/dev/null || echo "-I/usr/local/include -I/opt/homebrew/include") \
    -L/usr/local/lib \
    -L/opt/homebrew/lib \
    -o "$BUILD_DIR/main" \
    "$SRC_DIR/main.cpp" \
    "$BUILD_DIR/glad.o" \
    $(pkg-config --libs sdl3 2>/dev/null || echo "-lSDL3") \
    -framework OpenGL \
    -framework Cocoa \
    -framework IOKit \
    -framework CoreVideo \
    -framework CoreFoundation \
    -MD

echo "Build completed successfully!"
echo "Executable: $BUILD_DIR/main"

# Make the executable runnable
chmod +x "$BUILD_DIR/main"
