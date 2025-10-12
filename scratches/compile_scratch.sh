#!/bin/bash

# Script to compile GRF scratch files from the scratches directory
# This sets up all the necessary include paths and links against the GRF library

set -e  # Exit on error

echo "=== Compiling GRF Scratch Files from scratches/ directory ==="

# Define paths
GRF_ROOT="/Users/sermetpekin/Desktop/git_repos/grf"
CORE_DIR="$GRF_ROOT/core"
SRC_DIR="$CORE_DIR/src"
THIRD_PARTY_DIR="$CORE_DIR/third_party"
BUILD_DIR="$CORE_DIR/build"
LIB_FILE="$BUILD_DIR/libgrf.a"
SCRATCHES_DIR="$GRF_ROOT/scratches"

# Compiler settings
CXX="g++"
CXXFLAGS="-std=c++11 -O2 -Wall"
INCLUDES="-I$SRC_DIR -I$THIRD_PARTY_DIR -I$THIRD_PARTY_DIR/Eigen -I$CORE_DIR/test"
LIBS="-L$BUILD_DIR -lgrf -pthread"

echo "Using GRF library: $LIB_FILE"
echo "Working in: $SCRATCHES_DIR"

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
    local needs_grf="$3"
    
    if [ ! -f "$SCRATCHES_DIR/$source_file" ]; then
        echo "Warning: $source_file not found in scratches/, skipping..."
        return
    fi
    
    echo "Compiling $source_file -> $output_name"
    
    if [ "$needs_grf" = "true" ]; then
        $CXX $CXXFLAGS $INCLUDES "$SCRATCHES_DIR/$source_file" $LIBS -o "$SCRATCHES_DIR/$output_name"
    else
        $CXX $CXXFLAGS "$SCRATCHES_DIR/$source_file" -o "$SCRATCHES_DIR/$output_name"
    fi
    
    if [ $? -eq 0 ]; then
        echo "✓ Successfully compiled $output_name"
    else
        echo "✗ Failed to compile $source_file"
    fi
}

# Change to scratches directory
cd "$SCRATCHES_DIR"

echo ""
echo "Compiling scratch files..."

# Simple files (no GRF library needed)
compile_scratch "scratch_simple_demo.cpp" "scratch_simple_demo" "false"
compile_scratch "scratch_explain_indices.cpp" "scratch_explain_indices" "false"
compile_scratch "scratch_final_demo.cpp" "scratch_final_demo" "false"

# GRF-dependent files
compile_scratch "scratch_super_simple.cpp" "scratch_super_simple" "true"
compile_scratch "scratch_predict_simple.cpp" "scratch_predict_simple" "true"
compile_scratch "scratch_simple_data_inspection.cpp" "scratch_simple_data_inspection" "true"
compile_scratch "scratch_randomness_demo.cpp" "scratch_randomness_demo" "true"
compile_scratch "scratch_variance_analysis.cpp" "scratch_variance_analysis" "true"

# These might have dependency issues, try them but don't fail if they don't work
echo ""
echo "Attempting to compile more complex files (may have dependency issues):"
compile_scratch "scratch_causal_survival.cpp" "scratch_causal_survival" "true" || echo "  (Expected - needs test utilities)"
compile_scratch "scratch_causal_survival_simple.cpp" "scratch_causal_survival_simple" "true" || echo "  (May need adjustments)"
compile_scratch "scratch_data_inspection.cpp" "scratch_data_inspection" "true" || echo "  (May need adjustments)"
compile_scratch "scratch_predict_method.cpp" "scratch_predict_method" "true" || echo "  (May need adjustments)"

echo ""
echo "=== Compilation Complete ==="
echo ""
echo "To run the compiled programs (from scratches/ directory):"
echo "  cd scratches"
echo "  ./scratch_simple_demo"
echo "  ./scratch_explain_indices"
echo "  ./scratch_super_simple"
echo "  ./scratch_predict_simple"
echo "  ./scratch_simple_data_inspection"
echo "  ./scratch_randomness_demo"
echo "  ./scratch_variance_analysis"
echo ""
echo "Or from project root:"
echo "  ./scratches/scratch_super_simple"
echo "  etc."
echo ""