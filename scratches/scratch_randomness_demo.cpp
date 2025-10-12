/*
 * Demonstration of randomness in Generalized Random Forests
 * 
 * This shows how different random seeds produce different trees and predictions
 */

#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>

// Include GRF headers
#include "commons/Data.h"
#include "forest/ForestTrainers.h"
#include "forest/ForestPredictors.h"

using namespace grf;

// Simple data loader
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

// Create forest options with specific seed
ForestOptions create_options_with_seed(uint random_seed) {
    uint num_trees = 50;
    size_t ci_group_size = 1;
    double sample_fraction = 0.5;
    uint mtry = 3;
    uint min_node_size = 5;
    bool honesty = true;
    double honesty_fraction = 0.5;
    bool honesty_prune_leaves = true;
    double alpha = 0.05;
    double imbalance_penalty = 0.0;
    uint num_threads = 1;
    bool legacy_seed = false;
    std::vector<size_t> sample_clusters;
    uint samples_per_cluster = 0;
    
    return ForestOptions(num_trees, ci_group_size, sample_fraction, mtry,
                        min_node_size, honesty, honesty_fraction, 
                        honesty_prune_leaves, alpha, imbalance_penalty,
                        num_threads, random_seed, legacy_seed,
                        sample_clusters, samples_per_cluster);
}

int main() {
    std::cout << "=== Randomness in Generalized Random Forests ===" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    try {
        // Load data
        std::cout << "\n1. Loading data..." << std::endl;
        std::vector<std::vector<double>> data_vec = load_simple_data("core/test/forest/resources/causal_survival_data.csv");
        
        if (data_vec.empty()) {
            std::cerr << "Error: Could not load data" << std::endl;
            return 1;
        }
        
        // Convert to flat format
        std::vector<double> flat_data;
        size_t num_rows = data_vec.size();
        size_t num_cols = data_vec[0].size();
        
        for (const auto& row : data_vec) {
            for (double val : row) {
                flat_data.push_back(val);
            }
        }
        
        Data data(flat_data, num_rows, num_cols);
        data.set_treatment_index(5);
        data.set_instrument_index(5);
        data.set_censor_index(6);
        data.set_causal_survival_numerator_index(7);
        data.set_causal_survival_denominator_index(8);
        
        std::cout << "   Loaded " << data.get_num_rows() << " observations" << std::endl;
        
        // Train multiple forests with different seeds
        std::vector<uint> seeds = {42, 123, 456, 789, 999};
        std::cout << "\n2. Training forests with different random seeds..." << std::endl;
        
        // Store predictions from each forest
        std::vector<std::vector<double>> all_predictions;
        
        for (uint seed : seeds) {
            std::cout << "\n   Training with seed " << seed << "..." << std::endl;
            
            ForestTrainer trainer = causal_survival_trainer(true);
            ForestOptions options = create_options_with_seed(seed);
            Forest forest = trainer.train(data, options);
            
            ForestPredictor predictor = causal_survival_predictor(4);
            std::vector<Prediction> predictions = predictor.predict(forest, data, data, false);
            
            // Extract prediction values for first 10 samples
            std::vector<double> pred_values;
            for (size_t i = 0; i < std::min(10UL, predictions.size()); ++i) {
                pred_values.push_back(predictions[i].get_predictions()[0]);
            }
            all_predictions.push_back(pred_values);
            
            // Show average treatment effect for this forest
            double sum = 0.0;
            for (const auto& pred : predictions) {
                sum += pred.get_predictions()[0];
            }
            double avg_effect = sum / predictions.size();
            std::cout << "     Average treatment effect: " << avg_effect << std::endl;
        }
        
        // Compare predictions across different seeds
        std::cout << "\n3. Comparing predictions for first 10 samples across different seeds:" << std::endl;
        std::cout << "Sample |";
        for (uint seed : seeds) {
            std::cout << "  Seed " << std::setw(3) << seed << " |";
        }
        std::cout << " Std Dev |  Range" << std::endl;
        std::cout << "-------|";
        for (size_t i = 0; i < seeds.size(); ++i) {
            std::cout << "----------|";
        }
        std::cout << "---------|--------" << std::endl;
        
        for (size_t sample = 0; sample < 10; ++sample) {
            std::cout << std::setw(6) << sample << " |";
            
            // Collect predictions for this sample across all seeds
            std::vector<double> sample_preds;
            for (size_t seed_idx = 0; seed_idx < seeds.size(); ++seed_idx) {
                double pred = all_predictions[seed_idx][sample];
                sample_preds.push_back(pred);
                std::cout << std::setw(9) << pred << " |";
            }
            
            // Calculate standard deviation and range
            double mean = 0.0;
            for (double p : sample_preds) mean += p;
            mean /= sample_preds.size();
            
            double variance = 0.0;
            for (double p : sample_preds) {
                variance += (p - mean) * (p - mean);
            }
            double std_dev = std::sqrt(variance / sample_preds.size());
            
            double min_pred = *std::min_element(sample_preds.begin(), sample_preds.end());
            double max_pred = *std::max_element(sample_preds.begin(), sample_preds.end());
            double range = max_pred - min_pred;
            
            std::cout << std::setw(8) << std_dev << " |" << std::setw(7) << range << std::endl;
        }
        
        std::cout << "\n=== Sources of Randomness in Random Forests ===" << std::endl;
        std::cout << "1. **Bootstrap Sampling**: Each tree uses a random subsample of data" << std::endl;
        std::cout << "2. **Feature Subsampling**: At each split, randomly select mtry features" << std::endl;
        std::cout << "3. **Honest Splitting**: Random split of data into splitting/estimation sets" << std::endl;
        std::cout << "4. **Tree Structure**: Different random choices lead to different tree shapes" << std::endl;
        
        std::cout << "\n=== Why This Randomness is Good ===" << std::endl;
        std::cout << "1. **Reduces Overfitting**: Each tree sees different data/features" << std::endl;
        std::cout << "2. **Improves Generalization**: Ensemble averages out individual tree biases" << std::endl;
        std::cout << "3. **Provides Uncertainty**: Variation across seeds gives confidence intervals" << std::endl;
        std::cout << "4. **Robust Predictions**: Final estimate is average across many random trees" << std::endl;
        
        std::cout << "\n=== Controlling Randomness ===" << std::endl;
        std::cout << "- **Fixed Seed**: Reproducible results for debugging/testing" << std::endl;
        std::cout << "- **Different Seeds**: Multiple runs to assess prediction stability" << std::endl;
        std::cout << "- **More Trees**: Larger forests reduce variance between runs" << std::endl;
        std::cout << "- **Ensemble of Forests**: Train multiple forests with different seeds" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}