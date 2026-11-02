#include "MCMC.h"
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#include <algorithm>
#include <iomanip>

using namespace std;

void BivariateNormal() {
    const int SAMPLES = 2000;
    const int BURN_IN = 10000;
    const int ITERATIONS = SAMPLES + BURN_IN;

    const double rho = 0.7;
    const double STEP = 0.8;

    random_device rd;
    mt19937 gen(rd());

    normal_distribution<double> proposal(0.0, STEP);
    uniform_real_distribution<double> uniform(0.0, 1.0);


    double x = 0.0;
    double y = 0.0;

    int accepted = 0;

    ofstream file("Results/BivariateNormal.csv");

    if (!file.is_open()) {
        cout << "Error: cannot open Results/BivariateNormal.csv" << endl;
        return;
    }

    file << "X,Y\n";
    file << fixed << setprecision(8);


    for (int i = 0; i < ITERATIONS; i++) {
        double newX = x + proposal(gen);
        double newY = y + proposal(gen);

        double oldExponent = -(x * x - 2.0 * rho * x * y + y * y) / (2.0 * (1.0 - rho * rho));

        double newExponent = -(newX * newX - 2.0 * rho * newX * newY + newY * newY) / (2.0 * (1.0 - rho * rho));

        double alpha = min(1.0, exp(newExponent - oldExponent));

        if (uniform(gen) < alpha) {
            x = newX;
            y = newY;
            accepted++;
        }

        if (i >= BURN_IN) {
            file << x << "," << y << "\n";
        }
    }

    file.close();

    cout << fixed << setprecision(4);

    cout << "Bivariate Normal Distribution" << endl;
    cout << "Rho: " << rho << endl;
    cout << "Iterations: " << ITERATIONS << endl;
    cout << "Burn-in: " << BURN_IN << endl;
    cout << "Generated points: " << SAMPLES << endl;

    cout << "Acceptance rate: " << 100.0 * accepted / ITERATIONS << "%" << endl;

}
