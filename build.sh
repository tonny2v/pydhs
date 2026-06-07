#!/bin/bash

# Exit on error
set -e

# --- C++ Build ---
echo "===================================================="
echo "Building C++ implementation (dhs.so) using CMake..."
echo "===================================================="

# Clear old cache to avoid Python version mismatch
rm -f CMakeCache.txt

# Use CONDA_PREFIX if available to ensure correct Python/Boost version
if [ -n "$CONDA_PREFIX" ]; then
    echo "[Info] Detected Conda environment: $CONDA_PREFIX"
    cmake -DPython3_ROOT_DIR="$CONDA_PREFIX" -DPython3_FIND_STRATEGY=LOCATION .
else
    echo "[Warning] CONDA_PREFIX not set. Using default CMake search."
    cmake .
fi

make

if [ -f "dhs.so" ]; then
    echo "[Success] C++ build complete: dhs.so"
else
    echo "[Error] C++ build failed to produce dhs.so"
    exit 1
fi

# --- Rust Build ---
if [ -d "rust_impl" ]; then
    echo ""
    echo "===================================================="
    echo "Building Rust implementation..."
    echo "===================================================="
    
    cd rust_impl
    if command -v cargo &> /dev/null; then
        cargo build --release
        
        # Copy the resulting library for easier access if it exists
        if [ -f "target/release/libdhs.so" ]; then
            cp target/release/libdhs.so ./dhs.so
            echo "[Success] Rust build complete: rust_impl/dhs.so"
        fi
    else
        echo "[Warning] cargo not found. Skipping Rust build."
    fi
    cd ..
fi

echo ""
echo "All builds finished."
