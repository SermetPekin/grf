/*
 * Scratch file to explore Causal Survival Forests
 * 
 * This demonstrates how the CausalSurvivalPredictionStrategy::predict() method works
 * by loading real data and examining the intermediate values.
 */

#include <iostream>
#include <vector>
#include <iomanip>

// Include GRF headers
#include "commons/Data.h"
#include "commons/utility.h"
#include "forest/ForestTrainers.h"
#include "forest/ForestPredictors.h"
#include "utilities/FileTestUtilities.h"
#include "utilities/ForestTestUtilities.h"

using namespace grf;

int main() {
    std::cout << "=== Causal Survival Forest Exploration ===" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    try {
        // 1. Load the causal survival test data
        std::cout << "\n1. Loading causal survival data..." << std::endl;
        auto data_vec = load_data("test/forest/resources/causal_survival_data.csv");
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
        ForestOptions options = ForestTestUtilities::default_options();
        Forest forest = trainer.train(data, options);
        
        std::cout << "   Forest trained with " << forest.get_trees().size() << " trees" << std::endl;
        
        // 4. Create predictor and make predictions
        std::cout << "\n4. Making predictions..." << std::endl;
        ForestPredictor predictor = causal_survival_predictor(4);
        std::vector<Prediction> predictions = predictor.predict(forest, data, data, true); // estimate_variance=true
        
        // 5. Examine the predictions
        std::cout << "\n5. Examining predictions for first few samples:" << std::endl;
        std::cout << "   Sample | Treatment Effect | Variance | Error" << std::endl;
        std::cout << "   -------|------------------|----------|-------" << std::endl;
        
        for (size_t i = 0; i < std::min(10UL, predictions.size()); ++i) {
            const Prediction& pred = predictions[i];
            std::cout << "   " << std::setw(6) << i
                      << " | " << std::setw(16) << pred.get_predictions()[0];  // Treatment effect
                      
            if (!pred.get_variance_estimates().empty()) {
                std::cout << " | " << std::setw(8) << pred.get_variance_estimates()[0];
            } else {
                std::cout << " | " << std::setw(8) << "N/A";
            }
            
            if (!pred.get_error_estimates().empty()) {
                std::cout << " | " << std::setw(5) << pred.get_error_estimates()[0];
            } else {
                std::cout << " | " << std::setw(5) << "N/A";
            }
            std::cout << std::endl;
        }
        
        // 6. Statistics about the predictions
        std::cout << "\n6. Prediction statistics:" << std::endl;
        
        double sum_effects = 0.0;
        double sum_variances = 0.0;
        size_t valid_predictions = 0;
        
        for (const auto& pred : predictions) {
            if (!std::isnan(pred.get_predictions()[0])) {
                sum_effects += pred.get_predictions()[0];
                if (!pred.get_variance_estimates().empty() && !std::isnan(pred.get_variance_estimates()[0])) {
                    sum_variances += pred.get_variance_estimates()[0];
                }
                valid_predictions++;
            }
        }
        
        if (valid_predictions > 0) {
            std::cout << "   Average treatment effect: " << (sum_effects / valid_predictions) << std::endl;
            std::cout << "   Average variance: " << (sum_variances / valid_predictions) << std::endl;
            std::cout << "   Valid predictions: " << valid_predictions << "/" << predictions.size() << std::endl;
        }
        
        // 7. Let's also try out-of-bag predictions
        std::cout << "\n7. Out-of-bag predictions (first 5):" << std::endl;
        std::vector<Prediction> oob_predictions = predictor.predict_oob(forest, data, false);
        
        std::cout << "   Sample | OOB Treatment Effect" << std::endl;
        std::cout << "   -------|---------------------" << std::endl;
        
        for (size_t i = 0; i < std::min(5UL, oob_predictions.size()); ++i) {
            std::cout << "   " << std::setw(6) << i
                      << " | " << std::setw(19) << oob_predictions[i].get_predictions()[0]
                      << std::endl;
        }
        
        std::cout << "\n=== Exploration Complete ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}