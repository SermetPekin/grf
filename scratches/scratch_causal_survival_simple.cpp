/*
 * Simplified scratch file to explore Causal Survival Forests
 * 
 * This demonstrates the CausalSurvivalPredictionStrategy without test utilities
 */

#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <limits>

// Include GRF headers
#include "commons/Data.h"
#include "commons/utility.h"
#include "forest/ForestTrainers.h"
#include "forest/ForestPredictors.h"

using namespace grf;

// Create default forest options manually
ForestOptions create_default_options() {
    ForestOptions options;
    options.set_num_trees(50);           // Reasonable number of trees
    options.set_sample_size(0.5);        // Sample half the data for each tree
    options.set_sample_fraction(0.5);    // Sample fraction
    options.set_mtry(3);                 // Try 3 variables at each split
    options.set_min_node_size(5);        // Minimum 5 observations per leaf
    options.set_honesty(true);           // Use honest trees
    options.set_honesty_fraction(0.5);   // Split data 50/50 for honesty
    options.set_alpha(0.05);            // Significance level for confidence intervals
    options.set_imbalance_penalty(0.0);  // No imbalance penalty
    return options;
}

// Simple data loader - reads space-separated values
std::vector<std::vector<double>> load_simple_data(const std::string& filename) {
    std::vector<std::vector<double>> data;
    std::ifstream file(filename);
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<double> row;
        double value;
        
        while (iss >> value) {
            row.push_back(value);
        }
        
        if (!row.empty()) {
            data.push_back(row);
        }
    }
    
    return data;
}

int main() {
    std::cout << "=== Simplified Causal Survival Forest Exploration ===" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    try {
        // 1. Load the causal survival test data
        std::cout << "\n1. Loading causal survival data..." << std::endl;
        
        auto data_vec = load_simple_data("core/test/forest/resources/causal_survival_data.csv");
        
        if (data_vec.empty()) {
            std::cerr << "Error: Could not load data file" << std::endl;
            return 1;
        }
        
        Data data(data_vec);
        
        // Set up data indices (from the test)
        data.set_treatment_index(5);
        data.set_instrument_index(5);
        data.set_censor_index(6);
        data.set_causal_survival_numerator_index(7);
        data.set_causal_survival_denominator_index(8);
        
        std::cout << "   Data rows: " << data.get_num_rows() << std::endl;
        std::cout << "   Data cols: " << data.get_num_cols() << std::endl;
        
        // 2. Examine the raw data structure
        std::cout << "\n2. Examining first few rows of raw data:" << std::endl;
        std::cout << "   Row | Treat | Censor | Numerator | Denominator" << std::endl;
        std::cout << "   ----|-------|--------|-----------|------------" << std::endl;
        
        for (size_t i = 0; i < std::min(5UL, data.get_num_rows()); ++i) {
            std::cout << "   " << std::setw(3) << i 
                      << " | " << std::setw(5) << data.get(i, 5)  // treatment
                      << " | " << std::setw(6) << data.get(i, 6)  // censor
                      << " | " << std::setw(9) << data.get_causal_survival_numerator(i)
                      << " | " << std::setw(11) << data.get_causal_survival_denominator(i)
                      << std::endl;
        }
        
        // 3. Train the causal survival forest
        std::cout << "\n3. Training causal survival forest..." << std::endl;
        ForestTrainer trainer = causal_survival_trainer(true);
        ForestOptions options = create_default_options();
        Forest forest = trainer.train(data, options);
        
        std::cout << "   Forest trained with " << forest.get_trees().size() << " trees" << std::endl;
        
        // 4. Create predictor and make predictions
        std::cout << "\n4. Making predictions..." << std::endl;
        ForestPredictor predictor = causal_survival_predictor(4);
        std::vector<Prediction> predictions = predictor.predict(forest, data, data, false); // estimate_variance=false for simplicity
        
        // 5. Examine the predictions
        std::cout << "\n5. Examining predictions for first few samples:" << std::endl;
        std::cout << "   Sample | Treatment Effect" << std::endl;
        std::cout << "   -------|------------------" << std::endl;
        
        for (size_t i = 0; i < std::min(10UL, predictions.size()); ++i) {
            const Prediction& pred = predictions[i];
            std::cout << "   " << std::setw(6) << i
                      << " | " << std::setw(16) << pred.get_predictions()[0]  // Treatment effect
                      << std::endl;
        }
        
        // 6. Statistics about the predictions
        std::cout << "\n6. Prediction statistics:" << std::endl;
        
        double sum_effects = 0.0;
        size_t valid_predictions = 0;
        double min_effect = std::numeric_limits<double>::max();
        double max_effect = std::numeric_limits<double>::lowest();
        
        for (const auto& pred : predictions) {
            if (!std::isnan(pred.get_predictions()[0])) {
                double effect = pred.get_predictions()[0];
                sum_effects += effect;
                min_effect = std::min(min_effect, effect);
                max_effect = std::max(max_effect, effect);
                valid_predictions++;
            }
        }
        
        if (valid_predictions > 0) {
            std::cout << "   Average treatment effect: " << (sum_effects / valid_predictions) << std::endl;
            std::cout << "   Min treatment effect: " << min_effect << std::endl;
            std::cout << "   Max treatment effect: " << max_effect << std::endl;
            std::cout << "   Valid predictions: " << valid_predictions << "/" << predictions.size() << std::endl;
        }
        
        std::cout << "\n=== Exploration Complete ===" << std::endl;
        std::cout << "This demonstrates the CausalSurvivalPredictionStrategy::predict() method in action!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}