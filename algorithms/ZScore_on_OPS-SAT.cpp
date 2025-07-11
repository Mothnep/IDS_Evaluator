#include "../helper/helper_functions.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace std;

double calculateMean(const vector<double>& data) {
    return accumulate(data.begin(), data.end(), 0.0) / data.size();
}

double calculateStdDev(const vector<double>& data, double mean) {
    double variance = 0.0;
    for (double val : data) {
        variance += (val - mean) * (val - mean);
    }
    return sqrt(variance / data.size());
}

// Min-Max normalization to ensure all features contribute equally
vector<double> normalizeFeature(const vector<double>& feature) {
    double minVal = *min_element(feature.begin(), feature.end());
    double maxVal = *max_element(feature.begin(), feature.end());
    
    vector<double> normalized;
    for (double val : feature) {
        if (maxVal - minVal > 0) {
            normalized.push_back((val - minVal) / (maxVal - minVal));
        } else {
            normalized.push_back(0.0);  // All values are the same
        }
    }
    return normalized;
}

int main() {
    cout << "=== Improved Z-Score Anomaly Detection ===" << endl;
    
    // Read the embedded dataset
    vector<vector<string>> csvData = readEmbeddedDataset(true);
    
    vector<double> anomalyScores;
    vector<bool> isAnomaly;
    
    // Extract ALL available features (not just 3)
    vector<double> means, vars, stds, kurts, skews, nPeaks, smooth10, smooth20;
    
    for (const auto& row : csvData) {
        if (row.size() < 22) continue;
        
        // Extract anomaly label
        isAnomaly.push_back(row[1] == "1");
        
        // Extract MORE statistical features for better discrimination
        means.push_back(stod(row[7]));     // mean
        vars.push_back(stod(row[8]));      // variance  
        stds.push_back(stod(row[9]));      // std deviation
        kurts.push_back(stod(row[10]));    // kurtosis
        skews.push_back(stod(row[11]));    // skewness
        nPeaks.push_back(stod(row[12]));   // number of peaks
        smooth10.push_back(stod(row[13])); // smooth10_n_peaks
        smooth20.push_back(stod(row[14])); // smooth20_n_peaks
    }
    
    cout << "Loaded " << isAnomaly.size() << " samples" << endl;
    cout << "Anomalies: " << count(isAnomaly.begin(), isAnomaly.end(), true) << endl;
    cout << "Normal: " << count(isAnomaly.begin(), isAnomaly.end(), false) << endl;
    
    // Normalize all features to prevent one feature from dominating
    vector<double> normMeans = normalizeFeature(means);
    vector<double> normVars = normalizeFeature(vars);
    vector<double> normStds = normalizeFeature(stds);
    vector<double> normKurts = normalizeFeature(kurts);
    vector<double> normSkews = normalizeFeature(skews);
    vector<double> normPeaks = normalizeFeature(nPeaks);
    vector<double> normSmooth10 = normalizeFeature(smooth10);
    vector<double> normSmooth20 = normalizeFeature(smooth20);
    
    // Calculate statistics for each NORMALIZED feature
    double meanMean = calculateMean(normMeans);
    double stdMean = calculateStdDev(normMeans, meanMean);
    
    double meanVar = calculateMean(normVars);
    double stdVar = calculateStdDev(normVars, meanVar);
    
    double meanStd = calculateMean(normStds);
    double stdStd = calculateStdDev(normStds, meanStd);
    
    double meanKurt = calculateMean(normKurts);
    double stdKurt = calculateStdDev(normKurts, meanKurt);
    
    double meanSkew = calculateMean(normSkews);
    double stdSkew = calculateStdDev(normSkews, meanSkew);
    
    double meanPeaks = calculateMean(normPeaks);
    double stdPeaks = calculateStdDev(normPeaks, meanPeaks);
    
    // Calculate IMPROVED anomaly scores using MORE features and weighted combination
    for (size_t i = 0; i < normMeans.size(); i++) {
        // Calculate Z-scores for each feature
        double zMean = abs((normMeans[i] - meanMean) / (stdMean + 1e-10));
        double zVar = abs((normVars[i] - meanVar) / (stdVar + 1e-10));
        double zStd = abs((normStds[i] - meanStd) / (stdStd + 1e-10));
        double zKurt = abs((normKurts[i] - meanKurt) / (stdKurt + 1e-10));
        double zSkew = abs((normSkews[i] - meanSkew) / (stdSkew + 1e-10));
        double zPeaks = abs((normPeaks[i] - meanPeaks) / (stdPeaks + 1e-10));
        
        // Weighted combination - statistical features often more important for anomalies
        double score = (zMean * 0.2 + zVar * 0.25 + zStd * 0.15 + 
                       zKurt * 0.2 + zSkew * 0.15 + zPeaks * 0.05);
        
        anomalyScores.push_back(score);
    }
    
    // Analyze score distribution
    double minScore = *min_element(anomalyScores.begin(), anomalyScores.end());
    double maxScore = *max_element(anomalyScores.begin(), anomalyScores.end());
    double avgScore = calculateMean(anomalyScores);
    
    cout << "\n=== Score Distribution Analysis ===" << endl;
    cout << "Score range: [" << minScore << ", " << maxScore << "]" << endl;
    cout << "Average score: " << avgScore << endl;
    
    // Calculate separate averages for normal vs anomaly samples
    double avgNormalScore = 0.0, avgAnomalyScore = 0.0;
    int normalCount = 0, anomalyCount = 0;
    
    for (size_t i = 0; i < anomalyScores.size(); i++) {
        if (isAnomaly[i]) {
            avgAnomalyScore += anomalyScores[i];
            anomalyCount++;
        } else {
            avgNormalScore += anomalyScores[i];
            normalCount++;
        }
    }
    
    avgNormalScore /= normalCount;
    avgAnomalyScore /= anomalyCount;
    
    cout << "Average normal score: " << avgNormalScore << endl;
    cout << "Average anomaly score: " << avgAnomalyScore << endl;
    
    // Use a data-driven threshold (e.g., 75th percentile)
    vector<double> sortedScores = anomalyScores;
    sort(sortedScores.begin(), sortedScores.end());
    
    // Try different thresholds for better performance
    double threshold75 = sortedScores[static_cast<size_t>(sortedScores.size() * 0.75)];
    double threshold90 = sortedScores[static_cast<size_t>(sortedScores.size() * 0.90)];
    double threshold95 = sortedScores[static_cast<size_t>(sortedScores.size() * 0.95)];
    
    cout << "\n=== Threshold Options ===" << endl;
    cout << "75th percentile: " << threshold75 << endl;
    cout << "90th percentile: " << threshold90 << endl;
    cout << "95th percentile: " << threshold95 << endl;
    
    // Use the 90th percentile as a balanced threshold
    double threshold = threshold90;
    cout << "Selected threshold: " << threshold << endl;
    
    // Evaluate algorithm
    cout << "\n=== Evaluation Results ===" << endl;
    auto results = evaluateAlgorithm(anomalyScores, isAnomaly, "ImprovedZScore", "OPS-SAT", threshold);
    
    return 0;
}