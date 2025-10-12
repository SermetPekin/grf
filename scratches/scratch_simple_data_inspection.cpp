/*
 * Simple data inspection without needing full GRF compilation
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <limits>

struct DataRow {
    std::vector<double> features;  // X1, X2, X3, X4, X5
    double treatment;              // Column 5
    double censor;                 // Column 6  
    double numerator;              // Column 7
    double denominator;            // Column 8
};

std::vector<DataRow> load_csv(const std::string& filename) {
    std::vector<DataRow> data;
    std::ifstream file(filename);
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        DataRow row;
        
        // Read 5 feature columns
        for (int i = 0; i < 5; ++i) {
            if (std::getline(iss, token, ' ')) {
                row.features.push_back(std::stod(token));
            }
        }
        
        // Read treatment, censor, numerator, denominator
        if (std::getline(iss, token, ' ')) row.treatment = std::stod(token);
        if (std::getline(iss, token, ' ')) row.censor = std::stod(token);
        if (std::getline(iss, token, ' ')) row.numerator = std::stod(token);
        if (std::getline(iss, token, ' ')) row.denominator = std::stod(token);
        
        data.push_back(row);
    }
    
    return data;
}

int main() {
    std::cout << "=== Real Causal Survival Data Inspection ===" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    try {
        auto data = load_csv("core/test/forest/resources/causal_survival_data.csv");
        std::cout << "Loaded " << data.size() << " rows of data" << std::endl;
        
        // Show first 10 rows
        std::cout << "\nFirst 10 rows (X1, X2, X3, X4, X5, Treatment, Censor, Numerator, Denominator):" << std::endl;
        std::cout << "Row |   X1   |   X2   |   X3   |   X4   |   X5   |  T  | C |  Numerator  | Denominator" << std::endl;
        std::cout << "----|--------|--------|--------|--------|--------|-----|---|-------------|------------" << std::endl;
        
        for (size_t i = 0; i < std::min(10UL, data.size()); ++i) {
            const auto& row = data[i];
            std::cout << std::setw(3) << i << " |";
            for (const auto& feat : row.features) {
                std::cout << std::setw(7) << feat << " |";
            }
            std::cout << std::setw(4) << row.treatment << " |"
                      << std::setw(2) << row.censor << " |"
                      << std::setw(12) << row.numerator << " |"
                      << std::setw(11) << row.denominator << std::endl;
        }
        
        // Statistics
        std::cout << "\n=== Data Statistics ===" << std::endl;
        
        size_t treated = 0, control = 0;
        size_t censored = 0, uncensored = 0;
        double num_sum = 0, den_sum = 0;
        double num_min = std::numeric_limits<double>::max();
        double num_max = std::numeric_limits<double>::lowest();
        double den_min = std::numeric_limits<double>::max();
        double den_max = std::numeric_limits<double>::lowest();
        
        for (const auto& row : data) {
            if (row.treatment > 0.5) treated++; else control++;
            if (row.censor > 0.5) censored++; else uncensored++;
            
            num_sum += row.numerator;
            den_sum += row.denominator;
            
            num_min = std::min(num_min, row.numerator);
            num_max = std::max(num_max, row.numerator);
            den_min = std::min(den_min, row.denominator);
            den_max = std::max(den_max, row.denominator);
        }
        
        std::cout << "Treatment distribution:" << std::endl;
        std::cout << "  Treated: " << treated << " (" << (100.0 * treated / data.size()) << "%)" << std::endl;
        std::cout << "  Control: " << control << " (" << (100.0 * control / data.size()) << "%)" << std::endl;
        
        std::cout << "\nCensoring distribution:" << std::endl;
        std::cout << "  Censored: " << censored << " (" << (100.0 * censored / data.size()) << "%)" << std::endl;
        std::cout << "  Uncensored: " << uncensored << " (" << (100.0 * uncensored / data.size()) << "%)" << std::endl;
        
        std::cout << "\nNumerator statistics:" << std::endl;
        std::cout << "  Mean: " << (num_sum / data.size()) << std::endl;
        std::cout << "  Min:  " << num_min << std::endl;
        std::cout << "  Max:  " << num_max << std::endl;
        
        std::cout << "\nDenominator statistics:" << std::endl;
        std::cout << "  Mean: " << (den_sum / data.size()) << std::endl;
        std::cout << "  Min:  " << den_min << std::endl;
        std::cout << "  Max:  " << den_max << std::endl;
        
        // Simulate overall treatment effect
        std::cout << "\n=== Simulated Overall Treatment Effect ===" << std::endl;
        double overall_effect = (num_sum / data.size()) / (den_sum / data.size());
        std::cout << "Simple average approach: " << overall_effect << std::endl;
        std::cout << "(This is roughly what a causal survival forest would estimate)" << std::endl;
        
        // Look at patterns by treatment group
        std::cout << "\n=== Patterns by Treatment Group ===" << std::endl;
        double treated_num_sum = 0, treated_den_sum = 0;
        double control_num_sum = 0, control_den_sum = 0;
        
        for (const auto& row : data) {
            if (row.treatment > 0.5) {
                treated_num_sum += row.numerator;
                treated_den_sum += row.denominator;
            } else {
                control_num_sum += row.numerator;
                control_den_sum += row.denominator;
            }
        }
        
        std::cout << "Treated group averages:" << std::endl;
        std::cout << "  Avg numerator: " << (treated_num_sum / treated) << std::endl;
        std::cout << "  Avg denominator: " << (treated_den_sum / treated) << std::endl;
        
        std::cout << "Control group averages:" << std::endl;
        std::cout << "  Avg numerator: " << (control_num_sum / control) << std::endl;
        std::cout << "  Avg denominator: " << (control_den_sum / control) << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}