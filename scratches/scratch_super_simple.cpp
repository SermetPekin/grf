/*
 * Super simple scratch file - just demonstrate basic GRF functionality
 */

#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>

// Core GRF includes
#include "commons/Data.h"
#include "forest/ForestTrainers.h"
#include "forest/ForestPredictors.h"

using namespace grf;

int main() {
    std::cout << "=== Simple GRF Test ===" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    try {
        // Read the data file line by line
        std::ifstream file("core/test/forest/resources/causal_survival_data.csv");
        std::vector<double> flat_data;
        size_t num_rows = 0;
        size_t num_cols = 0;
        
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::vector<double> row;
            double value;
            
            while (iss >> value) {
                row.push_back(value);
            }
            
            if (!row.empty()) {
                if (num_cols == 0) {
                    num_cols = row.size();
                }
                
                for (double val : row) {
                    flat_data.push_back(val);
                }
                num_rows++;
            }
        }
        
        std::cout << "Loaded " << num_rows << " rows, " << num_cols << " columns" << std::endl;
        
        // Create Data object
        Data data(flat_data, num_rows, num_cols);
        
        // Set up indices
        data.set_treatment_index(5);
        data.set_instrument_index(5);
        data.set_censor_index(6);
        data.set_causal_survival_numerator_index(7);
        data.set_causal_survival_denominator_index(8);
        
        // Show first few raw values
        std::cout << "\nFirst 5 rows:" << std::endl;
        std::cout << "Row | Treatment | Censor | Numerator | Denominator" << std::endl;
        std::cout << "----|-----------|--------|-----------|------------" << std::endl;
        
        for (size_t i = 0; i < std::min(5UL, num_rows); ++i) {
            std::cout << std::setw(3) << i 
                      << " | " << std::setw(9) << data.get(i, 5)
                      << " | " << std::setw(6) << data.get(i, 6)
                      << " | " << std::setw(9) << data.get_causal_survival_numerator(i)
                      << " | " << std::setw(11) << data.get_causal_survival_denominator(i)
                      << std::endl;
        }
        
        // Create proper ForestOptions 
        uint num_trees = 10;
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
        uint random_seed = 42;
        bool legacy_seed = false;
        std::vector<size_t> sample_clusters;
        uint samples_per_cluster = 0;
        
        ForestOptions options(num_trees, ci_group_size, sample_fraction, mtry,
                            min_node_size, honesty, honesty_fraction, 
                            honesty_prune_leaves, alpha, imbalance_penalty,
                            num_threads, random_seed, legacy_seed,
                            sample_clusters, samples_per_cluster);
        
        std::cout << "\nTraining causal survival forest..." << std::endl;
        
        // Train forest
        ForestTrainer trainer = causal_survival_trainer(true);
        Forest forest = trainer.train(data, options);
        
        std::cout << "Trained forest with " << forest.get_trees().size() << " trees" << std::endl;
        
        // Make predictions
        std::cout << "\nMaking predictions..." << std::endl;
        ForestPredictor predictor = causal_survival_predictor(4);
        std::vector<Prediction> predictions = predictor.predict(forest, data, data, false);
        
        std::cout << "\nPredictions for first 10 samples:" << std::endl;
        std::cout << "Sample | Treatment Effect" << std::endl;
        std::cout << "-------|------------------" << std::endl;
        
        for (size_t i = 0; i < std::min(10UL, predictions.size()); ++i) {
            std::cout << std::setw(6) << i 
                      << " | " << std::setw(16) << predictions[i].get_predictions()[0]
                      << std::endl;
        }
        
        // Summary statistics
        double sum = 0.0;
        size_t count = 0;
        for (const auto& pred : predictions) {
            if (!std::isnan(pred.get_predictions()[0])) {
                sum += pred.get_predictions()[0];
                count++;
            }
        }
        
        std::cout << "\nSummary:" << std::endl;
        std::cout << "Average treatment effect: " << (sum / count) << std::endl;
        std::cout << "Valid predictions: " << count << "/" << predictions.size() << std::endl;
        
        std::cout << "\n✓ CausalSurvivalPredictionStrategy::predict() working!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}