#include "../lib/LibIsolationForest/cpp/IsolationForest.h"
#include "../helper/helper_functions.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>

using namespace IsolationForest;
using namespace std;


int main() { //example implementation
    // Parameters for the forest
    const uint32_t NUM_TREES = 100;
    const uint32_t SUB_SAMPLING_SIZE = 256; //number of rows a single tree sees
    
    // Read the dataset
    vector<vector<string>> csvData = readCSV(true, "../datasets/OPS-SAT-AD/data/dataset.csv");
    
    // Create the forest
    Forest forest(NUM_TREES, SUB_SAMPLING_SIZE);
    
    // Prepare samples and get anomaly labels
    vector<Sample> allSamples; //rows
    vector<bool> isAnomaly;
    
    // Iterate through the dataset and create samples with features for IF
    for (size_t i = 0; i < csvData.size(); i++) {
        const auto& row = csvData[i];
        if (row.size() < 22) {
            cerr << "Warning: Row " << i << " has insufficient columns. Skipping." << endl;
            continue;
        }
        
        // Extract anomaly label (column index 1)
        bool anomaly = (row[1] == "1");
        isAnomaly.push_back(anomaly);
        
        // Create sample with meaningful ID
        Sample sample("sample_" + row[0]);
        FeaturePtrList features;
        
        // Add features from dataset
        features.push_back(new Feature("mean", stod(row[7])));
        features.push_back(new Feature("var", stod(row[8])));
        features.push_back(new Feature("std", stod(row[9])));
        features.push_back(new Feature("kurtosis", stod(row[10])));
        features.push_back(new Feature("skew", stod(row[11])));
        features.push_back(new Feature("n_peaks", stod(row[12])));
        features.push_back(new Feature("smooth10_n_peaks", stod(row[13])));
        features.push_back(new Feature("smooth20_n_peaks", stod(row[14])));
        features.push_back(new Feature("diff_peaks", stod(row[15])));
        features.push_back(new Feature("diff2_peaks", stod(row[16])));
        
        sample.AddFeatures(features);
        allSamples.push_back(sample);
        forest.AddSample(sample);
    }
    
    // Counting anomalies in the dataset
    int anomalyCount = count(isAnomaly.begin(), isAnomaly.end(), true);
    cout << "Dataset contains " << anomalyCount << " anomalies and " 
            << (isAnomaly.size() - anomalyCount) << " normal samples.\n\n";
    forest.Create();
    cout << "Forest creation complete.\n\n";
    
    // Calculate anomaly scores for all samples
    vector<double> scores;
    for (const auto& sample : allSamples) {
        double score = 1.0 - forest.NormalizedScore(sample); //Library is wrong
        scores.push_back(score);
    }
    
    // Calculate average scores for normal and anomaly samples
    double avgNormalScore = 0.0, avgAnomalyScore = 0.0;
    int normalCount = 0;

    
    // Find a good threshold by calculating distributions
    for (size_t i = 0; i < allSamples.size(); i++) {
        if (isAnomaly[i]) {
            avgAnomalyScore += scores[i];
        } else {
            avgNormalScore += scores[i];
            normalCount++;
        }
    }
    //get avg score
    avgNormalScore /= normalCount;
    avgAnomalyScore /= anomalyCount;
    
    // Set threshold between normal and anomaly avg
    double ANOMALY_THRESHOLD = (avgNormalScore + avgAnomalyScore) / 2.0;
    cout << "\nAverage scores:" << endl;
    cout << "Normal samples: " << avgNormalScore << endl;
    cout << "Anomaly samples: " << avgAnomalyScore << endl;
    cout << "Calculated threshold: " << ANOMALY_THRESHOLD << endl;

    // Evaluate algorithm and print results
    auto results = evaluateAlgorithm(scores, isAnomaly, "IsolationForest", "OPS-SAT", ANOMALY_THRESHOLD);
    printEvaluationResults(results);
    
    return 0;
}