#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>

// Helper function to generate random data with noise
std::vector<std::vector<double>> generateData(size_t numSamples, size_t numFeatures) {
    std::vector<std::vector<double>> data(numSamples, std::vector<double>(numFeatures + 1));
    std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::normal_distribution<double> noise(0.0, 0.1);  // Gaussian noise with mean 0 and std deviation 0.1
    std::uniform_real_distribution<double> featureDist(0.0, 100.0);  // Random features between 0 and 100

    for (size_t i = 0; i < numSamples; ++i) {
        double target = 0.0;
        for (size_t j = 0; j < numFeatures; ++j) {
            double feature = featureDist(generator);
            data[i][j] = feature;
            target += feature * (j + 1);  // Assuming weights are 1, 2, ..., numFeatures
        }
        target += noise(generator);  // Add noise to the target
        data[i][numFeatures] = target;
    }

    return data;
}

// Function to compute linear regression coefficients using Normal Equation
std::vector<double> linearRegression(const std::vector<std::vector<double>>& data, size_t numFeatures) {
    size_t numSamples = data.size();
    std::vector<double> weights(numFeatures + 1, 0.0);  // Weights including bias

    std::vector<std::vector<double>> X(numSamples, std::vector<double>(numFeatures + 1, 1.0));
    std::vector<double> Y(numSamples);

    for (size_t i = 0; i < numSamples; ++i) {
        for (size_t j = 0; j < numFeatures; ++j) {
            X[i][j + 1] = data[i][j];
        }
        Y[i] = data[i][numFeatures];
    }

    // Compute XT * X
    std::vector<std::vector<double>> XT_X(numFeatures + 1, std::vector<double>(numFeatures + 1, 0.0));
    for (size_t i = 0; i <= numFeatures; ++i) {
        for (size_t j = 0; j <= numFeatures; ++j) {
            for (size_t k = 0; k < numSamples; ++k) {
                XT_X[i][j] += X[k][i] * X[k][j];
            }
        }
    }

    // Compute XT * Y
    std::vector<double> XT_Y(numFeatures + 1, 0.0);
    for (size_t i = 0; i <= numFeatures; ++i) {
        for (size_t k = 0; k < numSamples; ++k) {
            XT_Y[i] += X[k][i] * Y[k];
        }
    }

    // Solve XT_X * weights = XT_Y using Gaussian elimination (simplified, not robust for all cases)
    for (size_t i = 0; i <= numFeatures; ++i) {
        double pivot = XT_X[i][i];
        for (size_t j = i; j <= numFeatures; ++j) {
            XT_X[i][j] /= pivot;
        }
        XT_Y[i] /= pivot;

        for (size_t k = i + 1; k <= numFeatures; ++k) {
            double factor = XT_X[k][i];
            for (size_t j = i; j <= numFeatures; ++j) {
                XT_X[k][j] -= factor * XT_X[i][j];
            }
            XT_Y[k] -= factor * XT_Y[i];
        }
    }

    // Back-substitution
    for (int i = numFeatures; i >= 0; --i) {
        weights[i] = XT_Y[i];
        for (int j = i + 1; j <= numFeatures; ++j) {
            weights[i] -= XT_X[i][j] * weights[j];
        }
    }

    return weights;
}

int main() {
    const size_t numSamples = 25000000;  // Adjust to generate ~8GB of data (estimate per row size)
    const size_t numFeatures = 16;

    std::cout << "Generating data..." << std::endl;
    std::vector<std::vector<double>> data = generateData(numSamples, numFeatures);

    std::cout << "Performing linear regression..." << std::endl;
    std::vector<double> weights = linearRegression(data, numFeatures);

    std::cout << "Regression coefficients:" << std::endl;
    for (const auto& weight : weights) {
        std::cout << weight << " ";
    }
    std::cout << std::endl;

    return 0;
}
