/*
 * Simplified scratch file to explore the predict() method specifically
 * 
 * This focuses on understanding how CausalSurvivalPredictionStrategy::predict() works
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
#include "prediction/CausalSurvivalPredictionStrategy.h"

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

// Create minimal forest options
ForestOptions minimal_options() {
    uint num_trees = 10;
    size_t ci_group_size = 1;
    double sample_fraction = 0.6;
    uint mtry = 2;
    uint min_node_size = 10;
    bool honesty = true;
    double honesty_fraction = 0.5;
    bool honesty_prune_leaves = true;
    double alpha = 0.05;
    double imbalance_penalty = 0.0;
    uint num_threads = 1;
    uint random_seed = 42;
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
    std::cout << "=== CausalSurvivalPredictionStrategy::predict() Deep Dive ===" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    try {
        // Load data
        std::cout << "\n1. Loading data..." << std::endl;
        std::vector<std::vector<double>> data_vec = load_simple_data("core/test/forest/resources/causal_survival_data.csv");
        
        if (data_vec.empty()) {
            std::cerr << "Error: Could not load data" << std::endl;
            return 1;
        }
        
        // Convert 2D vector to flat vector for Data constructor
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
        
        // Look at raw numerator/denominator values
        std::cout << "\n2. Raw numerator/denominator values (first 10 rows):" << std::endl;
        std::cout << "   Row | Numerator  | Denominator | Individual Ratio" << std::endl;
        std::cout << "   ----|------------|-------------|------------------" << std::endl;
        
        double total_num = 0, total_den = 0;
        for (size_t i = 0; i < std::min(10UL, data.get_num_rows()); ++i) {
            double num = data.get_causal_survival_numerator(i);
            double den = data.get_causal_survival_denominator(i);
            double ratio = (den != 0) ? num / den : 0.0;
            
            total_num += num;
            total_den += den;
            
            std::cout << "   " << std::setw(3) << i 
                      << " | " << std::setw(10) << num
                      << " | " << std::setw(11) << den
                      << " | " << std::setw(16) << ratio
                      << std::endl;
        }
        
        std::cout << "\n   Simple average approach: " << (total_num/10) << " / " << (total_den/10) 
                  << " = " << (total_num/total_den) << std::endl;
        
        // Train forest
        std::cout << "\n3. Training forest..." << std::endl;
        ForestTrainer trainer = causal_survival_trainer(true);
        ForestOptions options = minimal_options();
        Forest forest = trainer.train(data, options);
        
        std::cout << "   Trained " << forest.get_trees().size() << " trees" << std::endl;
        
        // Make predictions
        std::cout << "\n4. Making predictions..." << std::endl;
        ForestPredictor predictor = causal_survival_predictor(4);
        
        // Make predictions for the full dataset (this is the correct way)
        std::vector<Prediction> predictions = predictor.predict(forest, data, data, false);
        
        // Make predictions for just first few samples to see the process
        std::vector<size_t> sample_indices = {0, 1, 2, 3, 4};
        
        std::cout << "\n5. Individual predictions (this is what predict() returns):" << std::endl;
        std::cout << "   Sample | Forest Prediction | Raw Numerator | Raw Denominator" << std::endl;
        std::cout << "   -------|-------------------|---------------|----------------" << std::endl;
        
        for (size_t idx : sample_indices) {
            std::cout << "   " << std::setw(6) << idx 
                      << " | " << std::setw(17) << predictions[idx].get_predictions()[0]
                      << " | " << std::setw(13) << data.get_causal_survival_numerator(idx)
                      << " | " << std::setw(15) << data.get_causal_survival_denominator(idx)
                      << std::endl;
        }
        
        std::cout << "\n=== Key Insight ===" << std::endl;
        std::cout << "The predict() method in CausalSurvivalPredictionStrategy.cpp does:" << std::endl;
        std::cout << "1. Each tree contributes numerator/denominator estimates" << std::endl;
        std::cout << "2. Average all numerators across trees" << std::endl;
        std::cout << "3. Average all denominators across trees" << std::endl;
        std::cout << "4. Return: average_numerator / average_denominator" << std::endl;
        std::cout << "\nThis implements the AIPW estimator for causal survival effects!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}