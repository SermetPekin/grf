/*
 * Demonstration of CausalSurvivalPredictionStrategy predict() method 
 * using patterns from real GRF test data
 */

#include <iostream>
#include <vector>
#include <iomanip>
#include <map>
#include <cmath>

// Simplified version of what the real predict() method does
std::vector<double> predict_causal_survival_effect(
    const std::vector<double>& numerators,
    const std::vector<double>& denominators) {
    
    std::cout << "=== CausalSurvivalPredictionStrategy::predict() Simulation ===" << std::endl;
    
    // This is exactly what happens in CausalSurvivalPredictionStrategy.cpp line 37-39:
    // return {average.at(NUMERATOR) / average.at(DENOMINATOR)};
    
    double numerator_sum = 0.0;
    double denominator_sum = 0.0;
    
    std::cout << "Input data (" << numerators.size() << " observations):" << std::endl;
    std::cout << "Observation | Numerator  | Denominator | Individual Effect" << std::endl;
    std::cout << "------------|------------|-------------|------------------" << std::endl;
    
    for (size_t i = 0; i < numerators.size(); ++i) {
        numerator_sum += numerators[i];
        denominator_sum += denominators[i];
        
        double individual_effect = (denominators[i] != 0.0) ? 
            numerators[i] / denominators[i] : 0.0;
            
        std::cout << std::setw(11) << i 
                  << " | " << std::setw(10) << std::fixed << std::setprecision(6) << numerators[i]
                  << " | " << std::setw(11) << denominators[i] 
                  << " | " << std::setw(16) << individual_effect << std::endl;
    }
    
    double avg_numerator = numerator_sum / numerators.size();
    double avg_denominator = denominator_sum / denominators.size();
    
    std::cout << "\nAveraging step:" << std::endl;
    std::cout << "Average numerator:   " << avg_numerator << std::endl;
    std::cout << "Average denominator: " << avg_denominator << std::endl;
    
    double treatment_effect = avg_numerator / avg_denominator;
    
    std::cout << "\nFinal prediction:" << std::endl;
    std::cout << "Treatment effect = " << avg_numerator << " / " << avg_denominator 
              << " = " << treatment_effect << std::endl;
    
    return {treatment_effect};
}

int main() {
    std::cout << std::fixed << std::setprecision(6);
    
    std::cout << "=== Understanding CausalSurvivalPredictionStrategy::predict() ===" << std::endl;
    std::cout << "Based on real GRF test data patterns\n" << std::endl;
    
    // Scenario 1: Simple case with clear pattern
    std::cout << "Scenario 1: Clear positive treatment effect" << std::endl;
    std::vector<double> nums1 = {0.15, 0.20, 0.18, 0.12, 0.25};
    std::vector<double> dens1 = {0.30, 0.35, 0.25, 0.20, 0.40};
    auto result1 = predict_causal_survival_effect(nums1, dens1);
    std::cout << "Predicted causal effect: " << result1[0] << "\n" << std::endl;
    
    // Scenario 2: Mixed effects (some positive, some negative numerators)
    std::cout << "Scenario 2: Mixed effects (like real data)" << std::endl;
    std::vector<double> nums2 = {0.154707, -0.037250, 0.284678, 0.161853, -0.166457};
    std::vector<double> dens2 = {0.267809, 0.000000, 0.334346, 0.000000, 0.000000};
    auto result2 = predict_causal_survival_effect(nums2, dens2);
    std::cout << "Predicted causal effect: " << result2[0] << "\n" << std::endl;
    
    // Scenario 3: Real data averages (from our inspection)
    std::cout << "Scenario 3: Using real data group averages" << std::endl;
    std::vector<double> nums3 = {0.154707, 0.144416};  // Treated vs Control averages
    std::vector<double> dens3 = {0.267809, 0.234426};  // Treated vs Control averages
    auto result3 = predict_causal_survival_effect(nums3, dens3);
    std::cout << "Predicted causal effect: " << result3[0] << "\n" << std::endl;
    
    std::cout << "=== Mathematical Interpretation ===" << std::endl;
    std::cout << "The predict() method implements the AIPW (Augmented Inverse Propensity Weighting)" << std::endl;
    std::cout << "estimator for causal survival analysis. Each numerator/denominator pair represents:" << std::endl;
    std::cout << "- Numerator: Weighted treatment effect for an observation" << std::endl;
    std::cout << "- Denominator: Propensity score weight for that observation" << std::endl;
    std::cout << "- Final result: Average treatment effect across all observations" << std::endl;
    std::cout << "\nThis is why we see:" << std::endl;
    std::cout << "1. Some negative numerators (negative individual effects)" << std::endl;
    std::cout << "2. Zero denominators (observations with no weight)" << std::endl;
    std::cout << "3. Final averaging that produces the overall causal estimate" << std::endl;
    
    return 0;
}