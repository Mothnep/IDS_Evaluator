#include "../helper/helper_functions.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace std;

double euclideanDistance(const vector<double> &a, const vector<double> &b)
{
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); i++)
    {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

// Min-Max normalization to [0, 1] range
vector<double> normalizeFeature(const vector<double> &feature)
{
    double minVal = *min_element(feature.begin(), feature.end());
    double maxVal = *max_element(feature.begin(), feature.end());

    vector<double> normalized;
    for (double val : feature)
    {
        if (maxVal - minVal > 0)
        {
            normalized.push_back((val - minVal) / (maxVal - minVal));
        }
        else
        {
            normalized.push_back(0.0); // All values are the same
        }
    }
    return normalized;
}

int main()
{
    cout << "=== Lightweight LOF for NSL-KDD Dataset ===" << endl;

    vector<vector<string>> csvData = readEmbeddedDataset(false); // No header for NSL-KDD

    // Use only a subset of data for faster simulation
    const size_t MAX_SAMPLES = 200;                                // Reduced from 2000 to 200
    const size_t actualSamples = min(MAX_SAMPLES, csvData.size()); // Pick the smallest

    vector<vector<double>> features; // will be filled after normalization
    vector<bool> isAnomaly;

    // Extract only the most important numerical features (8 features for better discrimination)
    // NSL-KDD has 43 columns: 0-40 are features, 41 is attack type, 42 is anomaly label
    vector<double> duration, src_bytes, dst_bytes, wrong_fragment, urgent, hot, num_failed_logins, num_compromised;

    // Process all samples to demonstrate imbalanced dataset effects
    for (size_t idx = 0; idx < actualSamples && idx < csvData.size(); idx++)
    {
        const auto &row = csvData[idx];
        if (row.size() < 43)
            continue;

        // Last column (index 42) contains anomaly labels
        bool isAnom = (row[42] == "1");
        isAnomaly.push_back(isAnom);
        
        // Extract all 8 features for each sample
        duration.push_back(stod(row[0]));           // duration
        src_bytes.push_back(stod(row[4]));          // src_bytes
        dst_bytes.push_back(stod(row[5]));          // dst_bytes
        wrong_fragment.push_back(stod(row[7]));     // wrong_fragment
        urgent.push_back(stod(row[8]));             // urgent
        hot.push_back(stod(row[9]));                // hot
        num_failed_logins.push_back(stod(row[10])); // num_failed_logins
        num_compromised.push_back(stod(row[13]));   // num_compromised
    }

    // Count and display the natural imbalance
    size_t normalCount = count(isAnomaly.begin(), isAnomaly.end(), false);
    size_t anomalyCount = count(isAnomaly.begin(), isAnomaly.end(), true);
    double anomalyPercentage = (double)anomalyCount / isAnomaly.size() * 100.0;
    
    cout << "\nUsing imbalanced dataset (natural distribution):" << endl;
    cout << "  Total samples: " << isAnomaly.size() << endl;
    cout << "  Normal samples: " << normalCount << " (" << (100.0 - anomalyPercentage) << "%)" << endl;
    cout << "  Anomaly samples: " << anomalyCount << " (" << anomalyPercentage << "%)" << endl;
    cout << "  Class imbalance ratio: " << (double)anomalyCount / normalCount << ":1 (anomaly:normal)" << endl;

    cout << "\nUsing lightweight configuration:" << endl;
    cout << "  Samples: " << isAnomaly.size() << " (imbalanced from " << csvData.size() << ")" << endl;
    cout << "  Features: 8 (duration, src_bytes, dst_bytes, wrong_fragment, urgent, hot, num_failed_logins, num_compromised)" << endl;

    // Normalize features
    cout << "\nNormalizing features..." << endl;
    vector<double> normDuration = normalizeFeature(duration);
    vector<double> normSrcBytes = normalizeFeature(src_bytes);
    vector<double> normDstBytes = normalizeFeature(dst_bytes);
    vector<double> normWrongFragment = normalizeFeature(wrong_fragment);
    vector<double> normUrgent = normalizeFeature(urgent);
    vector<double> normHot = normalizeFeature(hot);
    vector<double> normNumFailedLogins = normalizeFeature(num_failed_logins);
    vector<double> normNumCompromised = normalizeFeature(num_compromised);

    // Create feature matrix
    for (size_t i = 0; i < isAnomaly.size(); i++)
    {
        vector<double> sample = {
            normDuration[i], normSrcBytes[i], normDstBytes[i], normWrongFragment[i],
            normUrgent[i], normHot[i], normNumFailedLogins[i], normNumCompromised[i]};
        features.push_back(sample);
    }

    // Lightweight LOF parameters
    const int k = 5; // Reduced from 15 to 5
    vector<double> anomalyScores;

    cout << "\nCalculating lightweight LOF scores..." << endl;

    // Calculate simplified LOF scores
    for (size_t i = 0; i < features.size(); i++)
    {
        // progress tracking
        if (i % 50 == 0)
        {
            cout << "Processed " << i << "/" << features.size() << " samples" << endl;
        }

        vector<double> distances;

        // Calculate distances to all other points
        for (size_t j = 0; j < features.size(); j++)
        {
            if (i != j)
            {
                distances.push_back(euclideanDistance(features[i], features[j]));
            }
        }

        // Sort and take k nearest neighbors
        sort(distances.begin(), distances.end());

        // Average distance to k nearest neighbors (higher = more isolated/anomalous)
        double avgKDistance = 0.0;
        for (int n = 0; n < min(k, (int)distances.size()); n++)
        {
            avgKDistance += distances[n];
        }
        avgKDistance /= min(k, (int)distances.size());

        // LOF should give higher scores to anomalies (outliers)
        // Use inverse distance: closer neighbors = lower anomaly score, farther = higher anomaly score
        anomalyScores.push_back(avgKDistance);
    }

    cout << "LOF calculation complete!" << endl;

    // Show score analysis
    printBasicInfo(csvData, anomalyScores, isAnomaly, 42, "Lightweight LOF - NSL-KDD Score Analysis");

    // Analyze score distribution
    double minScore = *min_element(anomalyScores.begin(), anomalyScores.end());
    double maxScore = *max_element(anomalyScores.begin(), anomalyScores.end());
    double avgScore = accumulate(anomalyScores.begin(), anomalyScores.end(), 0.0) / anomalyScores.size();

    cout << "\n=== Score Analysis ===" << endl;
    cout << "Raw score range: [" << minScore << ", " << maxScore << "]" << endl;
    cout << "Average score: " << avgScore << endl;

    // Normalize scores to 0-1 range
    for (double &score : anomalyScores)
    {
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

    // Use the balanced labels that match our reduced sample size
    auto results = evaluateAlgorithm(anomalyScores, isAnomaly, "LightweightLOF", "NSL-KDD", threshold80);

    cout << "\n=== Lightweight Algorithm Summary ===" << endl;
    cout << "Optimizations applied:" << endl;
    cout << "  - Reduced dataset size: " << csvData.size() << " -> " << isAnomaly.size() << " samples" << endl;
    cout << "  - Imbalanced dataset: " << (100.0 - anomalyPercentage) << "% normal, " << anomalyPercentage << "% anomaly samples" << endl;
    cout << "  - Enhanced features: 8 discriminative features" << endl;
    cout << "  - Reduced k-neighbors: 15 -> 5" << endl;
    cout << "  - Fixed LOF score interpretation (higher score = more anomalous)" << endl;
    cout << "This demonstrates the impact of class imbalance on anomaly detection!" << endl;

    return 0;
}
