#include "MCMC.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <iomanip>

using namespace std;

void PowerLaw() {

    const int ITERATIONS = 1000000;
    const int BURN_IN = 10000;
    const int MAX_VALUE = 50;

    int x = 1;

    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<int> direction(0, 1);
    uniform_real_distribution<double> uniform(0.0, 1.0);


    vector<int> samples;
    samples.reserve(ITERATIONS - BURN_IN);

    int accepted = 0;

    for (int i = 0; i < ITERATIONS; i++) {
        int candidate = (direction(gen) == 0)
            ? x - 1
            : x + 1;

        if (candidate >= 1) {
            double alpha = min(1.0, pow(static_cast<double>(x) / candidate, 1.5));

            if (uniform(gen) < alpha) {
                x = candidate;
                accepted++;
            }
        }


        if (i >= BURN_IN) {
            samples.push_back(x);
        }
    }


    vector<int> counts(MAX_VALUE + 1, 0);

    for (int value : samples) {
        if (value <= MAX_VALUE) {
            counts[value]++;
        }
    }

    const double C = 2.6123753487;

    const int n = static_cast<int>(samples.size());

    ofstream file("Results/PowerLaw.csv");

    if (!file.is_open()) {
        cout << "Error: cannot open Results/PowerLaw.csv" << endl;
        return;
    }

    file << "Value,Empirical,Theoretical\n";
    file << fixed << setprecision(10);

    for (int i = 1; i <= MAX_VALUE; i++) {
        double empirical = static_cast<double>(counts[i]) / n;

        double theoretical = 1.0 / (C * pow(i, 1.5));

        file << i << "," << empirical << "," << theoretical << "\n";
    }

    file.close();


    cout << fixed << setprecision(4);

    cout << "Power Law distribution" << endl;
    cout << "Iterations: " << ITERATIONS << endl;
    cout << "Burn-in: " << BURN_IN << endl;
    cout << "Sample size: " << n << endl;

    cout << "Acceptance rate: " << 100.0 * accepted / ITERATIONS << "%" << endl;

}
