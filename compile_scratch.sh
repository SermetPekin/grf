#!/bin/bash

# Script to compile GRF scratch files
# This sets up all the necessary include paths and links against the GRF library

set -e  # Exit on error

echo "=== Compiling GRF Scratch Files ==="

# Define paths
GRF_ROOT="/Users/sermetpekin/Desktop/git_repos/grf"
CORE_DIR="$GRF_ROOT/core"
SRC_DIR="$CORE_DIR/src"
THIRD_PARTY_DIR="$CORE_DIR/third_party"
BUILD_DIR="$CORE_DIR/build"
LIB_FILE="$BUILD_DIR/libgrf.a"

# Compiler settings
CXX="g++"
CXXFLAGS="-std=c++11 -O2 -Wall"
INCLUDES="-I$SRC_DIR -I$THIRD_PARTY_DIR -I$THIRD_PARTY_DIR/Eigen -I$CORE_DIR/test"
LIBS="-L$BUILD_DIR -lgrf -pthread"

echo "Using GRF library: $LIB_FILE"

# Check if library exists
if [ ! -f "$LIB_FILE" ]; then
    echo "Error: GRF library not found at $LIB_FILE"
    echo "Please build GRF first by running: cd core && make"
    exit 1
fi

# Function to compile a scratch file
compile_scratch() {
    local source_file="$1"
    local output_name="$2"
    
    if [ ! -f "$source_file" ]; then
        echo "Warning: $source_file not found, skipping..."
        return
    fi
    
    echo "Compiling $source_file -> $output_name"
    $CXX $CXXFLAGS $INCLUDES "$source_file" $LIBS -o "$output_name"
    
    if [ $? -eq 0 ]; then
        echo "✓ Successfully compiled $output_name"
    else
        echo "✗ Failed to compile $source_file"
    fi
}

# Compile all scratch files
cd "$GRF_ROOT"

echo ""
echo "Compiling scratch files..."

compile_scratch "scratch_causal_survival.cpp" "scratch_causal_survival"
compile_scratch "scratch_predict_method.cpp" "scratch_predict_method"  
compile_scratch "scratch_data_inspection.cpp" "scratch_data_inspection"
compile_scratch "scratch_causal_survival_simple.cpp" "scratch_causal_survival_simple"
compile_scratch "scratch_predict_simple.cpp" "scratch_predict_simple"

echo ""
echo "=== Compilation Complete ==="
echo ""
echo "To run the compiled programs:"
echo "  ./scratch_causal_survival"
echo "  ./scratch_predict_method"
echo "  ./scratch_data_inspection"
echo "  ./scratch_causal_survival_simple"
echo "  ./scratch_predict_simple"
echo ""