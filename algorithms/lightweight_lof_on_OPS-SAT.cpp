#include "../helper/helper_functions.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace std;

double euclideanDistance(const vector<double>& a, const vector<double>& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); i++) {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

// Min-Max normalization to [0, 1] range
vector<double> normalizeFeature(const vector<double>& feature) {
    double minVal = *min_element(feature.begin(), feature.end());
    double maxVal = *max_element(feature.begin(), feature.end());
    
    vector<double> normalized;
    for (double val : feature) {
        if (maxVal - minVal > 0) {
            normalized.push_back((val - minVal) / (maxVal - minVal));
        } else {
            normalized.push_back(0.0);
        }
    }
    return normalized;
}

int main() {
    cout << "=== Lightweight LOF for gem5 Simulation ===" << endl;
    
    vector<vector<string>> csvData = readEmbeddedDataset(true);
    
    // Use only a subset of data for faster simulation
    const size_t MAX_SAMPLES = 200; // Reduced from 2000 to 200
    const size_t actualSamples = min(MAX_SAMPLES, csvData.size());
    
    vector<vector<double>> features;
    vector<bool> isAnomaly;
    
    // Extract only the most important features (reduced from 19 to 5)
    vector<double> means, vars, stds, kurts, skews;
    
    for (size_t idx = 0; idx < actualSamples && idx < csvData.size(); idx++) {
        const auto& row = csvData[idx];
        if (row.size() < 22) continue;
        
        isAnomaly.push_back(row[1] == "1");
        
        // Use only 5 most discriminative features
        means.push_back(stod(row[7]));      // mean
        vars.push_back(stod(row[8]));       // var
        stds.push_back(stod(row[9]));       // std
        kurts.push_back(stod(row[10]));     // kurtosis
        skews.push_back(stod(row[11]));     // skew
    }
    
    // Print basic dataset information
    printBasicInfo(csvData, {}, isAnomaly, 1, "Lightweight LOF - Dataset");
    
    cout << "\nUsing lightweight configuration:" << endl;
    cout << "  Samples: " << actualSamples << " (reduced from " << csvData.size() << ")" << endl;
    cout << "  Features: 5 (reduced from 19)" << endl;
    
    // Normalize features
    cout << "\nNormalizing features..." << endl;
    vector<double> normMeans = normalizeFeature(means);
    vector<double> normVars = normalizeFeature(vars);
    vector<double> normStds = normalizeFeature(stds);
    vector<double> normKurts = normalizeFeature(kurts);
    vector<double> normSkews = normalizeFeature(skews);
    
    // Create feature matrix
    for (size_t i = 0; i < actualSamples; i++) {
        vector<double> sample = {
            normMeans[i], normVars[i], normStds[i], normKurts[i], normSkews[i]
        };
        features.push_back(sample);
    }
    
    // Lightweight LOF parameters
    const int k = 5; // Reduced from 15 to 5
    vector<double> anomalyScores;
    
    cout << "\nCalculating lightweight LOF scores..." << endl;
    
    // Calculate simplified LOF scores
    for (size_t i = 0; i < features.size(); i++) {
        if (i % 50 == 0) {
            cout << "Processed " << i << "/" << features.size() << " samples" << endl;
        }
        
        vector<double> distances;
        
        // Calculate distances to all other points
        for (size_t j = 0; j < features.size(); j++) {
            if (i != j) {
                distances.push_back(euclideanDistance(features[i], features[j]));
            }
        }
        
        // Sort and take k nearest neighbors
        sort(distances.begin(), distances.end());
        
        // Average distance to k nearest neighbors (higher = more isolated/anomalous)
        double avgKDistance = 0.0;
        for (int n = 0; n < min(k, (int)distances.size()); n++) {
            avgKDistance += distances[n];
        }
        avgKDistance /= min(k, (int)distances.size());
        
        anomalyScores.push_back(avgKDistance);
    }
    
    cout << "LOF calculation complete!" << endl;
    
    // Show score analysis
    printBasicInfo(csvData, anomalyScores, isAnomaly, 1, "Lightweight LOF - Score Analysis");
    
    // Analyze score distribution
    double minScore = *min_element(anomalyScores.begin(), anomalyScores.end());
    double maxScore = *max_element(anomalyScores.begin(), anomalyScores.end());
    double avgScore = accumulate(anomalyScores.begin(), anomalyScores.end(), 0.0) / anomalyScores.size();
    
    cout << "\n=== Score Analysis ===" << endl;
    cout << "Raw score range: [" << minScore << ", " << maxScore << "]" << endl;
    cout << "Average score: " << avgScore << endl;
    
    // Normalize scores to 0-1 range
    for (double& score : anomalyScores) {
        score = (score - minScore) / (maxScore - minScore);
    }
    
    // Use simple threshold selection
    vector<double> sortedScores = anomalyScores;
    sort(sortedScores.begin(), sortedScores.end());
    
    double threshold80 = sortedScores[static_cast<size_t>(sortedScores.size() * 0.80)];
    
    cout << "\n=== Threshold Selection ===" << endl;
    cout << "80th percentile threshold: " << threshold80 << endl;
    cout << "Selected threshold: " << threshold80 << endl;
    
    // Evaluate algorithm
    cout << "\n=== Evaluation Results ===" << endl;
    
    // Resize labels to match the reduced sample size
    vector<bool> reducedLabels(isAnomaly.begin(), isAnomaly.begin() + actualSamples);
    
    auto results = evaluateAlgorithm(anomalyScores, reducedLabels, "LightweightLOF", "OPS-SAT", threshold80);
    
    cout << "\n=== Lightweight Algorithm Summary ===" << endl;
    cout << "Optimizations applied:" << endl;
    cout << "  - Reduced dataset size: " << csvData.size() << " -> " << actualSamples << " samples" << endl;
    cout << "  - Reduced features: 19 -> 5 features" << endl;
    cout << "  - Reduced k-neighbors: 15 -> 5" << endl;
    cout << "  - Simplified distance calculations" << endl;
    cout << "This makes the algorithm ~100x faster for simulation!" << endl;
    
    return 0;
}
