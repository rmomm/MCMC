#include "MCMC.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

double BetaDensity(double x, double a, double b) {
    if (x < 0.0 || x > 1.0)
        return 0.0;

    return 30.0 * x * pow(1.0 - x, 4);
}

void BetaDistribution() {
    const double a = 2.0;
    const double b = 5.0;

    const int ITERATIONS = 100000;
    const int BURN_IN = 10000;
    const double d = 0.01;

    double x = 0.5;

    random_device rd;
    mt19937 gen(rd());

    normal_distribution<double> proposal(0.0, sqrt(d));
    uniform_real_distribution<double> uniform(0.0, 1.0);

    vector<double> samples;

    int accepted = 0;

    for (int i = 0; i < ITERATIONS; i++) {
        double candidate = x + proposal(gen);

        if (candidate >= 0.0 && candidate <= 1.0) {
            double alpha = min(1.0,
                BetaDensity(candidate, a, b) /
                BetaDensity(x, a, b));

            if (uniform(gen) < alpha)
            {
                x = candidate;
                accepted++;
            }
        }

        if (i >= BURN_IN)
        {
            samples.push_back(x);
        }
    }

    const int N = 50;

    const int n = samples.size();

    const double width = 1.0 / N;

    vector<int> counts(N, 0);

    for (double value : samples) {
        int index = static_cast<int>(value / width);

        if (index >= N)
            index = N - 1;

        counts[index]++;
    }

    ofstream file("Results/Beta_histogram.csv");

    file << "BinStart,BinEnd,EmpiricalFrequency,TheoreticalProbability\n";

    for (int j = 0; j < N; j++) {
        double left = j * width;
        double right = (j + 1) * width;

        double empirical = static_cast<double>(counts[j]) / n;

        auto cdf = [](double x) {
                return 1.0 - pow(1.0 - x, 5) * (1.0 + 5.0 * x); };

        double theoretical = cdf(right) - cdf(left);

        file << left << "," << right << "," << empirical << ","  << theoretical << "\n";
    }

    file.close();
}