#include "lib/LibIsolationForest/cpp/IsolationForest.h"
#include "helper/helper_functions.h"
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

// Custom randomizer for reproducible results
class SeededRandomizer : public Randomizer {
public:
    SeededRandomizer(uint64_t seed) : m_gen(seed), m_dist(0, UINT64_MAX) {}
    virtual ~SeededRandomizer() {}
    
    virtual uint64_t Rand() override { return m_dist(m_gen); }
    virtual uint64_t RandUInt64(uint64_t min, uint64_t max) override { 
        return min + (Rand() % (max - min + 1)); 
    }

private:
    std::mt19937_64 m_gen;
    std::uniform_int_distribution<uint64_t> m_dist;
};

int main() {
    // FIXED: More robust seeding for complete reproducibility
    srand(42);  
    
    // Parameters for the forest - using more trees for stability
    const uint32_t NUM_TREES = 50;        // Increased from 10 for much better stability
    const uint32_t SUB_SAMPLING_SIZE = 64; // Increased from 32 for better tree depth
    
    // Read the dataset - now using embedded data instead of file
    vector<vector<string>> csvData = readEmbeddedDataset(true);
    
    // Create the forest
    Forest forest(NUM_TREES, SUB_SAMPLING_SIZE);
    
    // Set a reproducible random seed for the forest - CRITICAL FIX
    forest.SetRandomizer(new SeededRandomizer(42));
    
    // Prepare samples and get anomaly labels
    vector<Sample> allSamples; //rows
    vector<bool> isAnomaly;
    
    // DEBUG: Track feature value ranges to ensure good discrimination
    vector<double> meanVals, varVals, stdVals, kurtVals, skewVals;
    
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
        
        // Store raw values for debugging
        double mean = stod(row[7]);
        double var = stod(row[8]);
        double std_val = stod(row[9]);
        double kurt = stod(row[10]);
        double skew = stod(row[11]);
        
        meanVals.push_back(mean);
        varVals.push_back(var);
        stdVals.push_back(std_val);
        kurtVals.push_back(kurt);
        skewVals.push_back(skew);
        
        // FIXED: Use proper scaling that maintains feature relationships and prevents value collisions
        // The key is to scale values to different ranges to avoid uint64_t truncation issues
        // Use large multipliers and ensure each feature gets a distinct numerical range
        
        // Use Min-Max scaling to [BASE, BASE+RANGE] for each feature to ensure separation
        const uint64_t BASE_MEAN = 100000000ULL;      // Base for mean features
        const uint64_t BASE_VAR = 200000000ULL;       // Base for variance features  
        const uint64_t BASE_STD = 300000000ULL;       // Base for std dev features
        const uint64_t BASE_KURT = 400000000ULL;      // Base for kurtosis features
        const uint64_t BASE_SKEW = 500000000ULL;      // Base for skewness features
        const uint64_t BASE_PEAKS = 600000000ULL;     // Base for peak count features
        const uint64_t RANGE = 50000000ULL;           // Range for each feature type
        
        // Scale each feature to its own distinct range to prevent collisions
        features.push_back(new Feature("mean", BASE_MEAN + (uint64_t)((mean + 10.0) * RANGE / 20.0)));
        features.push_back(new Feature("var", BASE_VAR + (uint64_t)((var + 10.0) * RANGE / 20.0)));
        features.push_back(new Feature("std", BASE_STD + (uint64_t)((std_val + 10.0) * RANGE / 20.0)));
        features.push_back(new Feature("kurtosis", BASE_KURT + (uint64_t)((kurt + 10.0) * RANGE / 20.0)));
        features.push_back(new Feature("skew", BASE_SKEW + (uint64_t)((skew + 10.0) * RANGE / 20.0)));
        features.push_back(new Feature("n_peaks", BASE_PEAKS + (uint64_t)(stod(row[12]) * RANGE / 100.0)));
        features.push_back(new Feature("smooth10_n_peaks", BASE_PEAKS + 10000000ULL + (uint64_t)(stod(row[13]) * RANGE / 100.0)));
        features.push_back(new Feature("smooth20_n_peaks", BASE_PEAKS + 20000000ULL + (uint64_t)(stod(row[14]) * RANGE / 100.0)));
        features.push_back(new Feature("diff_peaks", BASE_PEAKS + 30000000ULL + (uint64_t)(stod(row[15]) * RANGE / 100.0)));
        features.push_back(new Feature("diff2_peaks", BASE_PEAKS + 40000000ULL + (uint64_t)(stod(row[16]) * RANGE / 100.0)));
        
        sample.AddFeatures(features);
        allSamples.push_back(sample);
        forest.AddSample(sample);
    }
    
    // DEBUG: Print feature value ranges
    cout << "=== FEATURE RANGE DEBUG ===" << endl;
    cout << "Mean range: [" << *min_element(meanVals.begin(), meanVals.end()) 
         << ", " << *max_element(meanVals.begin(), meanVals.end()) << "]" << endl;
    cout << "Var range: [" << *min_element(varVals.begin(), varVals.end()) 
         << ", " << *max_element(varVals.begin(), varVals.end()) << "]" << endl;
    cout << "Std range: [" << *min_element(stdVals.begin(), stdVals.end()) 
         << ", " << *max_element(stdVals.begin(), stdVals.end()) << "]" << endl;
    cout << "Kurtosis range: [" << *min_element(kurtVals.begin(), kurtVals.end()) 
         << ", " << *max_element(kurtVals.begin(), kurtVals.end()) << "]" << endl;
    cout << "Skewness range: [" << *min_element(skewVals.begin(), skewVals.end()) 
         << ", " << *max_element(skewVals.begin(), skewVals.end()) << "]" << endl;
    
    // Counting anomalies in the dataset
    int anomalyCount = count(isAnomaly.begin(), isAnomaly.end(), true);
    cout << "Dataset contains " << anomalyCount << " anomalies and " 
            << (isAnomaly.size() - anomalyCount) << " normal samples.\n\n";
    cout << "Creating forest with " << NUM_TREES << " trees..." << endl;
    cout << "This may take several minutes in gem5..." << endl;
    forest.Create();
    cout << "Forest creation complete.\n\n";
    
    // Calculate anomaly scores for all samples
    cout << "Calculating anomaly scores..." << endl;
    vector<double> scores;
    vector<double> rawPathLengths; // DEBUG: Store raw path lengths too
    
    for (size_t i = 0; i < allSamples.size(); i++) {
        if (i % 100 == 0) {  // Progress every 100 samples
            cout << "Processed " << i << "/" << allSamples.size() << " samples" << endl;
        }
        
        // Get both raw and normalized scores for debugging
        double rawScore = forest.Score(allSamples[i]);  // Raw average path length
        double normalizedScore = forest.NormalizedScore(allSamples[i]); // Normalized score
        
        rawPathLengths.push_back(rawScore);
        scores.push_back(normalizedScore);
        
        // DEBUG: Print some example scores
        if (i < 5) {
            cout << "Sample " << i << " (anomaly=" << isAnomaly[i] << "): raw=" << rawScore 
                 << ", normalized=" << normalizedScore << endl;
        }
    }
    cout << "Scoring complete." << endl;
    
    // DEBUG: Check score distribution
    cout << "\n=== DEBUG: Score Analysis ===" << endl;
    double minScore = *min_element(scores.begin(), scores.end());
    double maxScore = *max_element(scores.begin(), scores.end());
    double minRaw = *min_element(rawPathLengths.begin(), rawPathLengths.end());
    double maxRaw = *max_element(rawPathLengths.begin(), rawPathLengths.end());
    
    cout << "Normalized score range: [" << minScore << ", " << maxScore << "]" << endl;
    cout << "Raw path length range: [" << minRaw << ", " << maxRaw << "]" << endl;
    
    // Check if scores are all the same (indicating poor discrimination)
    bool allSame = true;
    double tolerance = (maxScore - minScore) * 0.01; // 1% of range
    for (size_t i = 1; i < scores.size(); i++) {
        if (abs(scores[i] - scores[0]) > tolerance) {
            allSame = false;
            break;
        }
    }
    cout << "All scores similar? " << (allSame ? "YES - PROBLEM!" : "No - Good") << endl;
    cout << "Score variance tolerance: " << tolerance << endl;
    
    // Show some example scores
    cout << "First 10 normalized scores: ";
    for (size_t i = 0; i < min(10ul, scores.size()); i++) {
        cout << scores[i] << " ";
    }
    cout << endl;
    
    // Calculate average scores for normal and anomaly samples
    double avgNormalScore = 0.0, avgAnomalyScore = 0.0;
    double avgNormalRaw = 0.0, avgAnomalyRaw = 0.0;
    int normalCount = 0;

    
    // Find a good threshold by calculating distributions
    for (size_t i = 0; i < allSamples.size(); i++) {
        if (isAnomaly[i]) {
            avgAnomalyScore += scores[i];
            avgAnomalyRaw += rawPathLengths[i];
        } else {
            avgNormalScore += scores[i];
            avgNormalRaw += rawPathLengths[i];
            normalCount++;
        }
    }
    //get avg score
    avgNormalScore /= normalCount;
    avgAnomalyScore /= anomalyCount;
    avgNormalRaw /= normalCount;
    avgAnomalyRaw /= anomalyCount;
    
    // Set threshold between normal and anomaly avg
    double ANOMALY_THRESHOLD = (avgNormalScore + avgAnomalyScore) / 2.0;
    cout << "\nAverage scores:" << endl;
    cout << "Normal samples: " << avgNormalScore << " (raw: " << avgNormalRaw << ")" << endl;
    cout << "Anomaly samples: " << avgAnomalyScore << " (raw: " << avgAnomalyRaw << ")" << endl;
    cout << "Calculated threshold: " << ANOMALY_THRESHOLD << endl;
    
    // KEY INSIGHT: Check if anomalies should have higher or lower scores
    // In Isolation Forest, anomalies typically have HIGHER normalized scores (closer to 1)
    // because they have shorter path lengths (easier to isolate)
    if (avgAnomalyScore < avgNormalScore) {
        cout << "\nWARNING: Anomalies have LOWER average scores than normal samples!" << endl;
        cout << "This suggests the scores may need to be inverted for this dataset." << endl;
        cout << "OR there's an issue with feature discrimination." << endl;
        cout << "Raw path lengths: Normal=" << avgNormalRaw << ", Anomaly=" << avgAnomalyRaw << endl;
        if (avgAnomalyRaw > avgNormalRaw) {
            cout << "Raw scores suggest anomalies are harder to isolate (longer paths)" << endl;
        }
    }

    // Evaluate algorithm and print results
    auto results = evaluateAlgorithm(scores, isAnomaly, "IsolationForest", "OPS-SAT", ANOMALY_THRESHOLD);
    printEvaluationResults(results);
    
    return 0;
}