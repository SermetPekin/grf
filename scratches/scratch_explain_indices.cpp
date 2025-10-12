/*
 * Example showing what typical causal survival data looks like
 * vs the test data with pre-computed AIPW components
 */

#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Understanding Causal Survival Data Structure ===" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    
    std::cout << "\n1. TYPICAL real-world dataset would have:" << std::endl;
    std::cout << "   Columns: [X1, X2, X3, X4, X5, Treatment, Censoring, SurvivalTime]" << std::endl;
    std::cout << "   Example row: [0.91, 0.14, 0.85, 0.05, 0.99, 1, 0, 24.5]" << std::endl;
    std::cout << "   - X1-X5: Patient characteristics (age, gender, biomarkers, etc.)" << std::endl;
    std::cout << "   - Treatment: 0=control, 1=treated" << std::endl;
    std::cout << "   - Censoring: 0=event observed, 1=censored" << std::endl;
    std::cout << "   - SurvivalTime: Time to event or censoring" << std::endl;
    
    std::cout << "\n2. GRF TEST dataset has:" << std::endl;
    std::cout << "   Columns: [X1, X2, X3, X4, X5, Treatment, Censoring, Numerator, Denominator]" << std::endl;
    std::cout << "   Example row: [0.91, 0.14, 0.85, 0.05, 0.99, 0, 1, 0.050, 0.329]" << std::endl;
    std::cout << "   - X1-X5: Same features" << std::endl;
    std::cout << "   - Treatment: Same treatment indicator" << std::endl;
    std::cout << "   - Censoring: Same censoring indicator" << std::endl;
    std::cout << "   - Numerator: Pre-computed AIPW numerator component" << std::endl;
    std::cout << "   - Denominator: Pre-computed AIPW denominator component" << std::endl;
    
    std::cout << "\n3. What the indices tell GRF:" << std::endl;
    std::cout << "   set_treatment_index(5)    → Column 5 = treatment assignment" << std::endl;
    std::cout << "   set_instrument_index(5)   → Column 5 = instrumental variable (same as treatment)" << std::endl;
    std::cout << "   set_censor_index(6)       → Column 6 = censoring indicator" << std::endl;
    std::cout << "   set_causal_survival_numerator_index(7)   → Column 7 = AIPW numerator" << std::endl;
    std::cout << "   set_causal_survival_denominator_index(8) → Column 8 = AIPW denominator" << std::endl;
    
    std::cout << "\n4. How the forest uses this:" << std::endl;
    std::cout << "   - Features (0-4): Used to build decision trees" << std::endl;
    std::cout << "   - Treatment (5): Identifies which group each subject belongs to" << std::endl;
    std::cout << "   - Censoring (6): Handles incomplete observations in survival analysis" << std::endl;
    std::cout << "   - Numerator/Denominator (7-8): Used to train the AIPW estimator" << std::endl;
    
    std::cout << "\n5. The magic of CausalSurvivalPredictionStrategy::predict():" << std::endl;
    std::cout << "   - Each tree learns to estimate numerator/denominator from features" << std::endl;
    std::cout << "   - predict() averages these estimates across all trees" << std::endl;
    std::cout << "   - Returns: average_numerator / average_denominator" << std::endl;
    std::cout << "   - This gives the causal treatment effect!" << std::endl;
    
    return 0;
}