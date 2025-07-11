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
            normalized.push_back(0.0);  // All values are the same
        }
    }
    return normalized;
}

int main() {
    cout << "=== Enhanced Local Outlier Factor (All Features) ===" << endl;
    
    vector<vector<string>> csvData = readEmbeddedDataset(true);
    
    vector<vector<double>> features;
    vector<bool> isAnomaly;
    
    // Extract ALL numerical features (similar to Isolation Forest approach)
    vector<double> samplings, durations, lens, means, vars, stds, kurts, skews, nPeaks;
    vector<double> smooth10, smooth20, diffPeaks, diff2Peaks, diffVar, diff2Var;
    vector<double> gapsSquared, lenWeighted, varDivDur, varDivLen;
    
    for (const auto& row : csvData) {
        if (row.size() < 22) continue;
        
        isAnomaly.push_back(row[1] == "1");
        
        // Extract ALL numerical features (columns 4-22, skipping categorical ones)
        samplings.push_back(stod(row[4]));      // sampling
        durations.push_back(stod(row[5]));      // duration  
        lens.push_back(stod(row[6]));           // len
        means.push_back(stod(row[7]));          // mean
        vars.push_back(stod(row[8]));           // var
        stds.push_back(stod(row[9]));           // std
        kurts.push_back(stod(row[10]));         // kurtosis
        skews.push_back(stod(row[11]));         // skew
        nPeaks.push_back(stod(row[12]));        // n_peaks
        smooth10.push_back(stod(row[13]));      // smooth10_n_peaks
        smooth20.push_back(stod(row[14]));      // smooth20_n_peaks
        diffPeaks.push_back(stod(row[15]));     // diff_peaks
        diff2Peaks.push_back(stod(row[16]));    // diff2_peaks
        diffVar.push_back(stod(row[17]));       // diff_var
        diff2Var.push_back(stod(row[18]));      // diff2_var
        gapsSquared.push_back(stod(row[19]));   // gaps_squared
        lenWeighted.push_back(stod(row[20]));   // len_weighted
        varDivDur.push_back(stod(row[21]));     // var_div_duration
        varDivLen.push_back(stod(row[22]));     // var_div_len
    }
    
    cout << "Loaded " << isAnomaly.size() << " samples" << endl;
    cout << "Anomalies: " << count(isAnomaly.begin(), isAnomaly.end(), true) << endl;
    cout << "Normal: " << count(isAnomaly.begin(), isAnomaly.end(), false) << endl;
    
    // Normalize ALL features to prevent any single feature from dominating
    cout << "\nNormalizing features..." << endl;
    vector<double> normSamplings = normalizeFeature(samplings);
    vector<double> normDurations = normalizeFeature(durations);
    vector<double> normLens = normalizeFeature(lens);
    vector<double> normMeans = normalizeFeature(means);
    vector<double> normVars = normalizeFeature(vars);
    vector<double> normStds = normalizeFeature(stds);
    vector<double> normKurts = normalizeFeature(kurts);
    vector<double> normSkews = normalizeFeature(skews);
    vector<double> normNPeaks = normalizeFeature(nPeaks);
    vector<double> normSmooth10 = normalizeFeature(smooth10);
    vector<double> normSmooth20 = normalizeFeature(smooth20);
    vector<double> normDiffPeaks = normalizeFeature(diffPeaks);
    vector<double> normDiff2Peaks = normalizeFeature(diff2Peaks);
    vector<double> normDiffVar = normalizeFeature(diffVar);
    vector<double> normDiff2Var = normalizeFeature(diff2Var);
    vector<double> normGapsSquared = normalizeFeature(gapsSquared);
    vector<double> normLenWeighted = normalizeFeature(lenWeighted);
    vector<double> normVarDivDur = normalizeFeature(varDivDur);
    vector<double> normVarDivLen = normalizeFeature(varDivLen);
    
    // Create feature matrix with ALL normalized features
    for (size_t i = 0; i < isAnomaly.size(); i++) {
        vector<double> sample = {
            normSamplings[i], normDurations[i], normLens[i], normMeans[i], normVars[i],
            normStds[i], normKurts[i], normSkews[i], normNPeaks[i], normSmooth10[i],
            normSmooth20[i], normDiffPeaks[i], normDiff2Peaks[i], normDiffVar[i], 
            normDiff2Var[i], normGapsSquared[i], normLenWeighted[i], normVarDivDur[i], 
            normVarDivLen[i]
        };
        features.push_back(sample);
    }
    
    cout << "Using " << features[0].size() << " features per sample" << endl;
    
    // Enhanced LOF parameters
    const int k = 15; // Increased k for better local density estimation
    vector<double> anomalyScores;
    
    cout << "\nCalculating enhanced LOF scores..." << endl;
    
    // Calculate LOF-like scores with progress tracking
    for (size_t i = 0; i < features.size(); i++) {
        if (i % 200 == 0) {
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
        
        // Calculate k-distance (distance to k-th nearest neighbor)
        double kDistance = distances[min(k-1, (int)distances.size()-1)];
        
        // Calculate average distance to k nearest neighbors (reachability distance approximation)
        double avgKDistance = 0.0;
        for (int n = 0; n < min(k, (int)distances.size()); n++) {
            avgKDistance += max(distances[n], kDistance); // Use reachability distance concept
        }
        avgKDistance /= min(k, (int)distances.size());
        
        anomalyScores.push_back(avgKDistance);
    }
    
    cout << "LOF calculation complete!" << endl;
    
    // Analyze score distribution before normalization
    double minScore = *min_element(anomalyScores.begin(), anomalyScores.end());
    double maxScore = *max_element(anomalyScores.begin(), anomalyScores.end());
    double avgScore = accumulate(anomalyScores.begin(), anomalyScores.end(), 0.0) / anomalyScores.size();
    
    cout << "\n=== Score Analysis ===" << endl;
    cout << "Raw score range: [" << minScore << ", " << maxScore << "]" << endl;
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
    
    // Normalize scores to 0-1 range for better threshold selection
    for (double& score : anomalyScores) {
        score = (score - minScore) / (maxScore - minScore);
    }
    
    // Use data-driven threshold selection
    vector<double> sortedScores = anomalyScores;
    sort(sortedScores.begin(), sortedScores.end());
    
    // Try different percentile thresholds
    double threshold75 = sortedScores[static_cast<size_t>(sortedScores.size() * 0.75)];
    double threshold80 = sortedScores[static_cast<size_t>(sortedScores.size() * 0.80)];
    double threshold85 = sortedScores[static_cast<size_t>(sortedScores.size() * 0.85)];
    double threshold90 = sortedScores[static_cast<size_t>(sortedScores.size() * 0.90)];
    
    cout << "\n=== Threshold Options ===" << endl;
    cout << "75th percentile: " << threshold75 << endl;
    cout << "80th percentile: " << threshold80 << endl;
    cout << "85th percentile: " << threshold85 << endl;
    cout << "90th percentile: " << threshold90 << endl;
    
    // Use 80th percentile as optimal balance (20% flagged as anomalies)
    double threshold = threshold80;
    cout << "Selected threshold: " << threshold << endl;
    
    // Evaluate algorithm
    cout << "\n=== Evaluation Results ===" << endl;
    auto results = evaluateAlgorithm(anomalyScores, isAnomaly, "EnhancedLOF", "OPS-SAT", threshold);
    
    return 0;
}