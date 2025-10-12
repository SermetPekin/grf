# GRF Exploration Scratch Files

This directory contains various scratch files created during our exploration of the GRF (Generalized Random Forests) codebase, specifically focusing on understanding the `CausalSurvivalPredictionStrategy::predict()` method and the overall architecture.

## 📁 File Overview

### Core Understanding Files
- **`scratch_super_simple.cpp`** - The simplest working example of causal survival forests
- **`scratch_predict_simple.cpp`** - Deep dive into the predict() method with real data
- **`scratch_causal_survival_simple.cpp`** - Simplified causal survival exploration without test utilities
- **`scratch_causal_survival.cpp`** - Full causal survival exploration (requires test utilities)

### Data Analysis Files
- **`scratch_simple_data_inspection.cpp`** - Analyzes the real test data structure and statistics
- **`scratch_data_inspection.cpp`** - More detailed data inspection and analysis
- **`scratch_explain_indices.cpp`** - Explains what the data index settings mean

### Advanced Analysis Files
- **`scratch_randomness_demo.cpp`** - Demonstrates randomness in RF predictions across different seeds
- **`scratch_variance_analysis.cpp`** - Analyzes prediction variance and shows how to reduce it
- **`scratch_final_demo.cpp`** - Final demonstration of predict() method with real data patterns
- **`scratch_simple_demo.cpp`** - Simple demonstration of predict() logic
- **`scratch_predict_method.cpp`** - Exploration of the predict method specifically

## 🚀 How to Use

### Compilation
We have a compilation script that should work (if still in root directory):
```bash
./compile_scratch.sh
```

Or compile individually:
```bash
# Basic files (no GRF library needed)
g++ -std=c++11 scratch_simple_demo.cpp -o scratch_simple_demo
g++ -std=c++11 scratch_explain_indices.cpp -o scratch_explain_indices

# GRF-dependent files (need library)
g++ -std=c++11 -O2 -Wall -Icore/src -Icore/third_party -Icore/third_party/Eigen scratch_super_simple.cpp -Lcore/build -lgrf -pthread -o scratch_super_simple
```

### Recommended Exploration Order
1. **Start here**: `scratch_explain_indices.cpp` - Understand data structure
2. **Basic demo**: `scratch_simple_demo.cpp` - See predict() logic in isolation
3. **Real GRF**: `scratch_super_simple.cpp` - Working causal survival forest
4. **Deep dive**: `scratch_predict_simple.cpp` - Detailed predict() analysis
5. **Data insights**: `scratch_simple_data_inspection.cpp` - Understand the test data
6. **Advanced topics**: `scratch_randomness_demo.cpp` and `scratch_variance_analysis.cpp`

## 🔍 Key Insights Discovered

### The predict() Method
- Located in `core/src/prediction/CausalSurvivalPredictionStrategy.cpp` (lines 37-39)
- Simply does: `return {average.at(NUMERATOR) / average.at(DENOMINATOR)};`
- Implements AIPW (Augmented Inverse Propensity Weighting) estimator
- Each tree contributes numerator/denominator estimates, final result is their ratio

### Test Data Structure
```
Column 0-4: Features (X1, X2, X3, X4, X5)
Column 5:   Treatment assignment (0=control, 1=treated)
Column 6:   Censoring indicator (0=observed, 1=censored)
Column 7:   Pre-computed AIPW numerator (for testing)
Column 8:   Pre-computed AIPW denominator (for testing)
```

### Randomness in Random Forests
- Different seeds produce different trees and predictions
- Variance can be reduced with more trees (500+ recommended)
- For production: use ensemble of forests with different seeds

### Data Index Settings
```cpp
data.set_treatment_index(5);                        // Column 5 = treatment
data.set_instrument_index(5);                       // Column 5 = instrument (same as treatment for RCT)
data.set_censor_index(6);                          // Column 6 = censoring
data.set_causal_survival_numerator_index(7);       // Column 7 = AIPW numerator
data.set_causal_survival_denominator_index(8);     // Column 8 = AIPW denominator
```

## 🧮 Mathematical Foundation

The causal survival forests implement AIPW estimation:
- **Doubly robust**: Consistent if either outcome model OR propensity model is correct
- **Efficient**: Achieves semiparametric efficiency bound
- **Flexible**: Works with random forests' nonparametric approach

## 🏗️ Architecture Insights

- **Strategy Pattern**: Different prediction strategies for different forest types
- **Clean API**: Complex mathematics hidden behind simple interfaces
- **Test-Driven**: Comprehensive test data with known expected results
- **Modular Design**: Separate concerns for training vs prediction vs data handling

## 📊 Production Recommendations

1. **Use 500+ trees** for stable predictions
2. **Ensemble multiple forests** with different seeds
3. **Report confidence intervals**, not just point estimates
4. **Validate stability** on held-out test sets
5. **Monitor prediction variance** in model selection

## 🔧 Compilation Dependencies

- **GRF Library**: Most files require linking against `core/build/libgrf.a`
- **Headers**: Need include paths to `core/src`, `core/third_party`, etc.
- **Threading**: Requires `-pthread` flag
- **C++11**: Minimum standard required

This collection of scratch files provides a comprehensive hands-on exploration of GRF's causal survival functionality!