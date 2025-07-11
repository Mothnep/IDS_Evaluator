#include "helper/helper_functions.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

using namespace std;

double euclideanDistance(const vector<double>& a, const vector<double>& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); i++) {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

int main() {
    vector<vector<string>> csvData = readEmbeddedDataset(true);
    
    vector<vector<double>> features;
    vector<bool> isAnomaly;
    
    // Extract normalized features
    for (const auto& row : csvData) {
        if (row.size() < 22) continue;
        
        isAnomaly.push_back(row[1] == "1");
        
        vector<double> sample = {
            stod(row[7]),   // mean
            stod(row[8]),   // var
            stod(row[9]),   // std
            stod(row[10]),  // kurtosis
            stod(row[11])   // skew
        };
        features.push_back(sample);
    }
    
    const int k = 10; // Number of nearest neighbors
    vector<double> anomalyScores;
    
    // Calculate LOF-like scores
    for (size_t i = 0; i < features.size(); i++) {
        vector<double> distances;
        
        // Calculate distances to all other points
        for (size_t j = 0; j < features.size(); j++) {
            if (i != j) {
                distances.push_back(euclideanDistance(features[i], features[j]));
            }
        }
        
        // Sort and take k nearest neighbors
        sort(distances.begin(), distances.end());
        
        // Average distance to k nearest neighbors (higher = more isolated)
        double avgKDistance = 0.0;
        for (int n = 0; n < min(k, (int)distances.size()); n++) {
            avgKDistance += distances[n];
        }
        avgKDistance /= min(k, (int)distances.size());
        
        anomalyScores.push_back(avgKDistance);
    }
    
    // Normalize scores to 0-1 range
    double minScore = *min_element(anomalyScores.begin(), anomalyScores.end());
    double maxScore = *max_element(anomalyScores.begin(), anomalyScores.end());
    
    for (double& score : anomalyScores) {
        score = (score - minScore) / (maxScore - minScore);
    }
    
    double threshold = 0.7; // Top 30% are considered anomalies
    
    auto results = evaluateAlgorithm(anomalyScores, isAnomaly, "SimpleLOF", "OPS-SAT", threshold);
    
    return 0;
}