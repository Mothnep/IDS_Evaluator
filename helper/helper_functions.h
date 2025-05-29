#ifndef HELPER_FCTS_H
#define HELPER_FCTS_H

#include <vector>
#include <string>
#include <map>
#include <tuple>

using namespace std;
/**
 * This file provides useful functions for users to
 * evaluate their algorithms.
 */

/**
 * Generate ROC curve data points and save them to a CSV file.
 * 
 * @param scores Prediction scores from your algorithm (higher = more likely positive)
 * @param labels Actual binary labels (true for positive, false for negative)
 * @param algorithm_name Name of your algorithm for the output filename
 * @param dataset_name Name of the dataset used for the output filename
 * @return AUC (Area Under Curve) value
 */
double saveRocDataToCSV(
    const vector<double>& scores,
    const vector<bool>& labels,
    const string& algorithm_name,
    const string& dataset_name);

/**
 * Calculate a confusion matrix from predictions and labels.
 * 
 * @param scores Prediction scores from algorithm
 * @param labels True labels (true for anomaly, false for normal)
 * @param threshold Threshold for classifying anomalies
 * @return tuple of (TP, FP, TN, FN)
 */
tuple<int, int, int, int> calculateConfusionMatrix(
    const vector<double>& scores, 
    const vector<bool>& labels, 
    double threshold);

/**
 * Calculate evaluation metrics from confusion matrix.
 * 
 * @param confusionMatrix Tuple containing (TP, FP, TN, FN)
 * @return Map of metric names to values
 */
map<string, double> calculateMetrics(
    const tuple<int, int, int, int>& confusionMatrix);

/**
 * Evaluate algorithm performance and generate ROC curve data.
 * 
 * @param scores Prediction scores from algorithm
 * @param labels True labels (true for anomaly, false for normal)
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
    double threshold = -1);

/**
 * Print evaluation results in a formatted way.
 * 
 * @param results Map of evaluation metrics
 * @param showPercentages Whether to show percentage values
 */
void printEvaluationResults(
    const map<string, double>& results, 
    bool showPercentages = true);

/**
 * Read a CSV file and return its contents as a vector of vectors of strings.
 *
 * @param headerRow Whether the first row contains headers
 * @param filename Name of the CSV file to read
 * @return Vector of string vectors representing the CSV rows
 */
vector<vector<string>> readCSV(bool headerRow, const string& filename);
#endif // HELPER_FCTS_H