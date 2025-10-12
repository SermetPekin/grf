/*
 * Data inspection scratch file
 * 
 * This examines the actual CSV data to understand what the numerator
 * and denominator values look like in practice.
 */

#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>

#include "commons/Data.h"
#include "utilities/FileTestUtilities.h"

using namespace grf;

int main() {
    std::cout << "=== Causal Survival Data Inspection ===" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    try {
        // Load the test data
        std::cout << "Loading causal survival test data..." << std::endl;
        auto data_vec = load_data("test/forest/resources/causal_survival_data.csv");
        Data data(data_vec);
        
        // Set up the indices
        data.set_treatment_index(5);
        data.set_censor_index(6);
        data.set_causal_survival_numerator_index(7);
        data.set_causal_survival_denominator_index(8);
        
        std::cout << "Data dimensions: " << data.get_num_rows() << " rows x " << data.get_num_cols() << " cols" << std::endl;
        
        // Examine column headers (first few columns)
        std::cout << "\nFirst few columns of data:" << std::endl;
        std::cout << "Row |   X1   |   X2   |   X3   |   X4   |   X5   | Treat | Censor | Numer. | Denom." << std::endl;
        std::cout << "----|--------|--------|--------|--------|--------|-------|--------|--------|--------" << std::endl;
        
        for (size_t i = 0; i < std::min(10UL, data.get_num_rows()); ++i) {
            std::cout << std::setw(3) << i << " |";
            
            // Show first 5 feature columns
            for (size_t j = 0; j < std::min(5UL, data.get_num_cols()); ++j) {
                std::cout << std::setw(7) << data.get(i, j) << " |";
            }
            
            // Show treatment, censor, numerator, denominator
            std::cout << std::setw(6) << data.get(i, 5) << " |";  // treatment
            std::cout << std::setw(7) << data.get(i, 6) << " |";  // censor
            std::cout << std::setw(7) << data.get_causal_survival_numerator(i) << " |";
            std::cout << std::setw(7) << data.get_causal_survival_denominator(i);
            std::cout << std::endl;
        }
        
        // Statistics about the numerator and denominator
        std::cout << "\n=== Numerator and Denominator Statistics ===" << std::endl;
        
        double num_sum = 0, num_sum_sq = 0;
        double den_sum = 0, den_sum_sq = 0;
        size_t n = data.get_num_rows();
        
        double num_min = std::numeric_limits<double>::max();
        double num_max = std::numeric_limits<double>::lowest();
        double den_min = std::numeric_limits<double>::max();
        double den_max = std::numeric_limits<double>::lowest();
        
        for (size_t i = 0; i < n; ++i) {
            double num = data.get_causal_survival_numerator(i);
            double den = data.get_causal_survival_denominator(i);
            
            num_sum += num;
            num_sum_sq += num * num;
            den_sum += den;
            den_sum_sq += den * den;
            
            num_min = std::min(num_min, num);
            num_max = std::max(num_max, num);
            den_min = std::min(den_min, den);
            den_max = std::max(den_max, den);
        }
        
        double num_mean = num_sum / n;
        double num_var = (num_sum_sq / n) - (num_mean * num_mean);
        double num_std = std::sqrt(num_var);
        
        double den_mean = den_sum / n;
        double den_var = (den_sum_sq / n) - (den_mean * den_mean);
        double den_std = std::sqrt(den_var);
        
        std::cout << "Numerator statistics:" << std::endl;
        std::cout << "  Mean: " << num_mean << std::endl;
        std::cout << "  Std:  " << num_std << std::endl;
        std::cout << "  Min:  " << num_min << std::endl;
        std::cout << "  Max:  " << num_max << std::endl;
        
        std::cout << "\nDenominator statistics:" << std::endl;
        std::cout << "  Mean: " << den_mean << std::endl;
        std::cout << "  Std:  " << den_std << std::endl;
        std::cout << "  Min:  " << den_min << std::endl;
        std::cout << "  Max:  " << den_max << std::endl;
        
        // Simulate what the overall prediction would be
        std::cout << "\n=== Simulated Overall Treatment Effect ===" << std::endl;
        std::cout << "If we used simple averages:" << std::endl;
        std::cout << "  Average numerator: " << num_mean << std::endl;
        std::cout << "  Average denominator: " << den_mean << std::endl;
        std::cout << "  Overall effect: " << (num_mean / den_mean) << std::endl;
        
        // Treatment distribution
        std::cout << "\n=== Treatment Distribution ===" << std::endl;
        size_t treated = 0, control = 0;
        for (size_t i = 0; i < n; ++i) {
            if (data.get(i, 5) > 0.5) {  // assuming treatment is 0/1
                treated++;
            } else {
                control++;
            }
        }
        std::cout << "Treated: " << treated << " (" << (100.0 * treated / n) << "%)" << std::endl;
        std::cout << "Control: " << control << " (" << (100.0 * control / n) << "%)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}