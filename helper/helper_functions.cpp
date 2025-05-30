#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <utility>
#include <filesystem>
#include <limits> 
#include <map>

using namespace std;



/**
 * This file provides useful functions for users to
 * evaluate their algorithms.
 */

/**
 * Generate ROC curve data points and save them to a CSV file.
 * This function computes the True Positive Rate (TPR) and False Positive Rate (FPR).
 * Example usage:
 * 
 * int main() {
 *    // These would come from your algorithm's predictions
 *    vector<double> scores = {0.9, 0.8, 0.7, 0.6, 0.55, 0.54, 0.53, 0.52, 0.51, 0.4};
 *    vector<double> true_labels = {1, 1, 0, 1, 1, 0, 0, 0, 1, 0};
 *    
 *    double auc = saveRocDataToCSV(scores, true_labels, "MyAlgorithm", "TestDataset");
 *    cout << "AUC: " << auc << endl;
 *    
 *    return 0;
 *      }
 * @param scores Prediction scores from your algorithm (higher = more likely positive)
 * @param labels Actual binary labels (1 for positive, 0 for negative)
 * @param algorithm_name Name of your algorithm for the output filename
 * @param dataset_name Name of the dataset used for the output filename
 * @return AUC (Area Under Curve) value
 */

double saveRocDataToCSV(
    const vector<double>& scores,
    const vector<bool>& labels,  // Changed from double to bool
    const string& algorithm_name,
    const string& dataset_name) 
{

    // Validate inputs
    if (scores.size() != labels.size() || scores.empty()) {
        cerr << "Error: Invalid input data (scores and labels must be same length and non-empty)" << endl;
        return 0.0;
    }

    // Create pairs of (score, label)
    vector<pair<double, bool>> score_label_pairs;
    for (size_t i = 0; i < scores.size(); i++) {
        score_label_pairs.push_back({scores[i], labels[i]});
    }

    // Sort pairs by score in descending order
    sort(score_label_pairs.begin(), score_label_pairs.end(),
         [](const auto& a, const auto& b) { return a.first > b.first; });

    // Calculate ROC curve points
    vector<double> tpr, fpr, thresholds;
    
    double total_positives = 0.0, total_negatives = 0.0;
    for (const auto& pair : score_label_pairs) {
        if (pair.second) total_positives += 1.0;
        else total_negatives += 1.0;
    }
    
    if (total_positives == 0 || total_negatives == 0) {
        cerr << "Error: Data must contain both positive and negative samples" << endl;
        return 0.0;
    }

    double true_positives = 0.0, false_positives = 0.0;
    double prev_score = numeric_limits<double>::max();//Keep track for threshholds
    
    // Add (0,0) point at the beginning
    fpr.push_back(0.0);
    tpr.push_back(0.0);
    thresholds.push_back(numeric_limits<double>::max());
    
    for (const auto& [score, label] : score_label_pairs) {
        // If score changes, add a point to the ROC curve
        if (score != prev_score) {
            fpr.push_back(false_positives / total_negatives);
            tpr.push_back(true_positives / total_positives);
            thresholds.push_back(prev_score);
            prev_score = score;
        }
        
        // Update counts
        if (label) true_positives += 1.0;
        else false_positives += 1.0;
    }
    
    // Add the final point (1,1)
    fpr.push_back(false_positives / total_negatives);
    tpr.push_back(true_positives / total_positives);
    thresholds.push_back(prev_score);
    
    // Calculate AUC using trapezoidal rule
    double auc = 0.0;
    for (size_t i = 1; i < fpr.size(); i++) {
        auc += (fpr[i] - fpr[i-1]) * (tpr[i] + tpr[i-1]) / 2.0;
    }


    string filename = "ROC_CSV/" + algorithm_name + "_" + dataset_name + "_roc.csv";
    ofstream outfile(filename);
    
    if (!outfile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return auc;
    }
    
    // Write header
    outfile << "threshold,fpr,tpr" << endl;
    
    // Write data points
    for (size_t i = 0; i < fpr.size(); i++) {
        if (i < thresholds.size()) {
            outfile << thresholds[i] << "," << fpr[i] << "," << tpr[i] << endl;
        }
    }
    
    outfile.close();
    cout << "ROC data saved to " << filename << endl;
    cout << "AUC: " << auc << endl;
    return auc;
}


/**
 * Calculate a confusion matrix from predictions and labels.
 * 
 * @param scores Prediction scores from algorithm
 * @param labels True labels (1 for anomaly, 0 for normal)
 * @param threshold Threshold for classifying anomalies
 * @return tuple of (TP, FP, TN, FN)
 */
tuple<int, int, int, int> calculateConfusionMatrix(
    const vector<double>& scores, 
    const vector<bool>& labels, 
    double threshold) 
{
    int truePositives = 0, falsePositives = 0;
    int trueNegatives = 0, falseNegatives = 0;
    
    for (size_t i = 0; i < scores.size(); i++) {
        bool predictedAnomaly = scores[i] > threshold;
        bool isAnomaly = labels[i]; 
        
        if (isAnomaly && predictedAnomaly) truePositives++;
        else if (!isAnomaly && predictedAnomaly) falsePositives++;
        else if (!isAnomaly && !predictedAnomaly) trueNegatives++;
        else if (isAnomaly && !predictedAnomaly) falseNegatives++;
    }
    
    return {truePositives, falsePositives, trueNegatives, falseNegatives};
}

/**
 * Calculate evaluation metrics from confusion matrix.
 * 
 * @param confusionMatrix Tuple containing (TP, FP, TN, FN)
 * @return Map of metric names to values
 */
map<string, double> calculateMetrics(const tuple<int, int, int, int>& confusionMatrix) 
{
    auto [tp, fp, tn, fn] = confusionMatrix;
    map<string, double> metrics;
    
    // Basic counts
    metrics["true_positives"] = tp;
    metrics["false_positives"] = fp;
    metrics["true_negatives"] = tn;
    metrics["false_negatives"] = fn;
    
    // Derived metrics
    double total = tp + fp + tn + fn;
    metrics["accuracy"] = total > 0 ? (tp + tn) / total : 0.0;
    metrics["precision"] = (tp + fp) > 0 ? tp / static_cast<double>(tp + fp) : 0.0;
    metrics["recall"] = (tp + fn) > 0 ? tp / static_cast<double>(tp + fn) : 0.0;
    metrics["specificity"] = (tn + fp) > 0 ? tn / static_cast<double>(tn + fp) : 0.0;
    
    // F1 score
    metrics["f1_score"] = (metrics["precision"] + metrics["recall"]) > 0 ? 
        2.0 * metrics["precision"] * metrics["recall"] / 
        (metrics["precision"] + metrics["recall"]) : 0.0;
    
    return metrics;
}

//forward declaration
void printEvaluationResults(const map<string, double>& results, bool showPercentages = true);

/**
 * Evaluate algorithm performance and generate ROC curve data.
 * 
 * @param scores Prediction scores from algorithm
 * @param labels True labels (1 for anomaly, 0 for normal)
 * @param algorithm_name Name of algorithm for output file
 * @param dataset_name Name of dataset for output file
 * @param threshold Optional threshold for binary classification (if not provided, uses average)
 * @return Evaluation results including metrics and AUC
 */
map<string, double> evaluateAlgorithm(
    const vector<double>& scores,
    const vector<bool>& labels,
    const string& algorithm_name,
    const string& dataset_name,
    double threshold) 
{
    // Validate inputs
    if (scores.size() != labels.size() || scores.empty()) {
        cerr << "Error: Invalid input data (scores and labels must be same length and non-empty)" << endl;
        return {{"error", 1.0}};
    }

    // If threshold not provided, calculate one
    if (threshold < 0) {
        double avgPositiveScore = 0.0, avgNegativeScore = 0.0;
        int posCount = 0, negCount = 0;
        
        for (size_t i = 0; i < scores.size(); i++) {
            if (labels[i]) {
                avgPositiveScore += scores[i];
                posCount++;
            } else {
                avgNegativeScore += scores[i];
                negCount++;
            }
        }
        
        if (posCount > 0) avgPositiveScore /= posCount;
        if (negCount > 0) avgNegativeScore /= negCount;
        
        threshold = (avgPositiveScore + avgNegativeScore) / 2.0;
    }
    
    // Calculate confusion matrix and metrics
    auto confMat = calculateConfusionMatrix(scores, labels, threshold);
    map<string, double> results = calculateMetrics(confMat);
    
    // Add threshold to results
    results["threshold"] = threshold;
    
    // Generate ROC curve and calculate AUC
    double auc = saveRocDataToCSV(scores, labels, algorithm_name, dataset_name);
    results["auc"] = auc;
    
    // Automatically print evaluation results
    printEvaluationResults(results);
    
    return results;
}

/**
 * Print evaluation results in a formatted way.
 * 
 * @param results Map of evaluation metrics
 * @param showPercentages Whether to show percentage values
 */
void printEvaluationResults(const map<string, double>& results, bool showPercentages) 
{
    cout << "\n===== Algorithm Evaluation Results =====\n";
    
    // Print threshold if available
    if (results.count("threshold")) {
        cout << "Threshold: " << results.at("threshold") << endl;
    }
    
    // Print confusion matrix
    if (results.count("true_positives") && results.count("false_positives") && 
        results.count("true_negatives") && results.count("false_negatives")) {
        cout << "\nConfusion Matrix:\n";
        cout << "TP: " << static_cast<int>(results.at("true_positives")) << "\t";
        cout << "FP: " << static_cast<int>(results.at("false_positives")) << "\n";
        cout << "FN: " << static_cast<int>(results.at("false_negatives")) << "\t";
        cout << "TN: " << static_cast<int>(results.at("true_negatives")) << "\n";
    }
    
    // Print metrics
    cout << "\nMetrics:\n";
    vector<string> metricsToShow = {"accuracy", "precision", "recall", "specificity", "f1_score"};
    
    for (const auto& metric : metricsToShow) {
        if (results.count(metric)) {
            cout << metric << ": ";
            if (showPercentages && metric != "auc") {
                cout << results.at(metric) * 100 << "%";
            } else {
                cout << results.at(metric);
            }
            cout << endl;
        }
    }
    
    cout << "=======================================\n";
}

/**
 * Read a CSV file and return its contents as a vector of vectors of strings.
 * 
 * @param headerRow Whether the CSV has a header row to skip
 * @param filename Name of the CSV file to read
 * @return Vector of rows, each row is a vector of strings (cells)
 */

vector<vector<string>> readCSV(bool headerRow, const string& filename) { //const reference
    vector<vector<string>> data; //initialization of data
    ifstream file(filename); //define input file stream
    
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return data;
    }

    
    string line; //string to hold each line
    if(headerRow){
        getline(file, line); //skip header line
    }
 
    while (getline(file, line)) { //while there are still lines in file
        vector<string> row; //creates new row vector
        stringstream ss(line); //string stream to parse current line
        string cell;//variable to hold each cell
        
        while (getline(ss, cell, ',')) {//loop through each cell in the current line stream
            row.push_back(cell);//add cell to row vector
        }
        
        if (!row.empty()) {
            data.push_back(row);//add row to data vector
        }
    }
    
    file.close();
    if (data.empty()) {
        cerr << "Warning: No data found in file " << filename << endl;
    }
    else {
        cout << "Successfully read " << data.size() << " samples from " << filename << endl;
    }

    return data;
}