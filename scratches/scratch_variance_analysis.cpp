/*
 * Analysis of prediction variance and how to reduce it
 * 
 * This demonstrates techniques to make Random Forest predictions more stable
 */

#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>

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

// Create forest options with different configurations
ForestOptions create_options(uint num_trees, double sample_fraction, uint min_node_size, uint random_seed) {
    size_t ci_group_size = 1;
    uint mtry = 3;
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

double calculate_std_dev(const std::vector<double>& values) {
    double mean = 0.0;
    for (double v : values) mean += v;
    mean /= values.size();
    
    double variance = 0.0;
    for (double v : values) {
        variance += (v - mean) * (v - mean);
    }
    return std::sqrt(variance / values.size());
}

int main() {
    std::cout << "=== Analyzing and Reducing Prediction Variance ===" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    try {
        // Load data
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
        
        std::cout << "Loaded " << data.get_num_rows() << " observations" << std::endl;
        
        // Test different configurations
        struct Config {
            std::string name;
            uint num_trees;
            double sample_fraction;
            uint min_node_size;
        };
        
        std::vector<Config> configs = {
            {"Small Forest (50 trees)", 50, 0.5, 5},
            {"Medium Forest (200 trees)", 200, 0.5, 5},
            {"Large Forest (500 trees)", 500, 0.5, 5},
            {"Conservative (smaller samples)", 200, 0.3, 10},
            {"Aggressive (larger samples)", 200, 0.8, 3}
        };
        
        std::vector<uint> seeds = {42, 123, 456, 789, 999};
        
        for (const auto& config : configs) {
            std::cout << "\n=== " << config.name << " ===" << std::endl;
            std::cout << "Trees: " << config.num_trees 
                      << ", Sample fraction: " << config.sample_fraction
                      << ", Min node size: " << config.min_node_size << std::endl;
            
            std::vector<double> avg_effects;
            std::vector<std::vector<double>> all_predictions;
            
            // Train with different seeds
            for (uint seed : seeds) {
                ForestTrainer trainer = causal_survival_trainer(true);
                ForestOptions options = create_options(config.num_trees, config.sample_fraction, config.min_node_size, seed);
                Forest forest = trainer.train(data, options);
                
                ForestPredictor predictor = causal_survival_predictor(4);
                std::vector<Prediction> predictions = predictor.predict(forest, data, data, false);
                
                // Calculate average effect
                double sum = 0.0;
                for (const auto& pred : predictions) {
                    sum += pred.get_predictions()[0];
                }
                double avg_effect = sum / predictions.size();
                avg_effects.push_back(avg_effect);
                
                // Store predictions for first 5 samples
                std::vector<double> sample_preds;
                for (size_t i = 0; i < 5; ++i) {
                    sample_preds.push_back(predictions[i].get_predictions()[0]);
                }
                all_predictions.push_back(sample_preds);
            }
            
            // Calculate variance in average effects
            double avg_effect_std = calculate_std_dev(avg_effects);
            double min_avg = *std::min_element(avg_effects.begin(), avg_effects.end());
            double max_avg = *std::max_element(avg_effects.begin(), avg_effects.end());
            
            std::cout << "Average effects across seeds: ";
            for (double effect : avg_effects) {
                std::cout << effect << " ";
            }
            std::cout << std::endl;
            std::cout << "Std dev of average effects: " << avg_effect_std << std::endl;
            std::cout << "Range of average effects: " << (max_avg - min_avg) << std::endl;
            
            // Calculate variance for individual predictions
            std::cout << "Individual prediction variance (first 5 samples):" << std::endl;
            for (size_t sample = 0; sample < 5; ++sample) {
                std::vector<double> sample_across_seeds;
                for (size_t seed_idx = 0; seed_idx < seeds.size(); ++seed_idx) {
                    sample_across_seeds.push_back(all_predictions[seed_idx][sample]);
                }
                double sample_std = calculate_std_dev(sample_across_seeds);
                std::cout << "  Sample " << sample << " std dev: " << sample_std << std::endl;
            }
        }
        
        std::cout << "\n=== Analysis of High Variance ===" << std::endl;
        std::cout << "Why we see large prediction differences:" << std::endl;
        std::cout << "1. **Small forest size**: 50 trees is quite small for stable estimates" << std::endl;
        std::cout << "2. **High-dimensional problem**: Causal inference is inherently harder" << std::endl;
        std::cout << "3. **Sample size**: 500 observations may not be enough for stable AIPW" << std::endl;
        std::cout << "4. **Honest trees**: Additional randomness from splitting data" << std::endl;
        std::cout << "5. **Complex target**: Learning numerator/denominator ratios is difficult" << std::endl;
        
        std::cout << "\n=== Solutions to Reduce Variance ===" << std::endl;
        std::cout << "1. **More trees**: 200-500+ trees instead of 50" << std::endl;
        std::cout << "2. **Larger sample fraction**: Use more data per tree (0.6-0.8)" << std::endl;
        std::cout << "3. **Larger minimum node size**: Prevent overfitting with min_node_size=10+" << std::endl;
        std::cout << "4. **Ensemble of forests**: Train multiple forests and average" << std::endl;
        std::cout << "5. **Cross-validation**: Use CV to select stable hyperparameters" << std::endl;
        std::cout << "6. **More data**: Collect larger training datasets when possible" << std::endl;
        
        std::cout << "\n=== Production Recommendations ===" << std::endl;
        std::cout << "- Use 500+ trees for production models" << std::endl;
        std::cout << "- Train 5-10 forests with different seeds and ensemble" << std::endl;
        std::cout << "- Report confidence intervals, not just point estimates" << std::endl;
        std::cout << "- Validate stability on held-out test sets" << std::endl;
        std::cout << "- Consider variance in model selection criteria" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}