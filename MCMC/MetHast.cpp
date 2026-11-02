#include "MCMC.h"
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#include <iomanip>
#include <string>

using namespace std;

const int ITERATIONS = 100000;
const int BURN_IN = 10000;

const double MEAN = 2.4;
const double VARIANCE = 0.8;

void RunExperiment(double d, const string& filename, ofstream& statistics) {
    random_device rd;
    mt19937 gen(rd());

    normal_distribution<double> proposal(0.0, sqrt(d));

    uniform_real_distribution<double> uniform(0.0, 1.0);

    double theta = 0.0;

    ofstream output(filename);

    if (!output.is_open()) {
        cout << "Cannot open file: " << filename << endl;
        return;
    }

    output << "Iteration,Theta\n";
    output << fixed << setprecision(10);

    int accepted = 0;
    int sampleCount = 0;

    double sum = 0.0;
    double sumSquared = 0.0;

    for (int i = 1; i <= ITERATIONS; i++) {
        double proposedTheta = theta + proposal(gen);

        double logAlpha = -(pow(proposedTheta - MEAN, 2) - pow(theta - MEAN, 2)) / (2.0 * VARIANCE);

        double u = uniform(gen);

        if (logAlpha >= 0.0 || log(u) < logAlpha) {
            theta = proposedTheta;

            if (i > BURN_IN)
                accepted++;
        }

        output << i << "," << theta << "\n";

        if (i > BURN_IN) {
            sampleCount++;

            sum += theta;
            sumSquared += theta * theta;
        }
    }

    output.close();

    double sampleMean = sum / sampleCount;

    double sampleVariance = (sumSquared - sum * sum / sampleCount) / (sampleCount - 1);

    double acceptanceRate = 100.0 * accepted / (ITERATIONS - BURN_IN);

    statistics << fixed << setprecision(6);

    statistics << "Дисперсія пропозиції d = " << d << "\n";
    statistics << "Середнє значення вибірки: " << sampleMean << "\n";
    statistics << "Теоретичне математичне сподівання: " << MEAN << "\n";
    statistics << "Дисперсія вибірки: " << sampleVariance << "\n";
    statistics << "Теоретична дисперсія: " << VARIANCE << "\n";
    statistics << "Частота прийняття переходів: " << acceptanceRate << "%\n";
    statistics << "-----------------------------------\n";
}

void MetropolisHastings() {
    ofstream statistics("Results/statistics.txt");

    if (!statistics.is_open()) {
        cout << "Cannot create statistics file!" << endl;
        return;
    }

    cout << "METROPOLIS-HASTINGS ALGORITHM\n";

    statistics << "METROPOLIS-HASTINGS ALGORITHM\n\n";

    RunExperiment(0.01, "Results/d_0_01.csv", statistics);
    RunExperiment(1.0, "Results/d_1.csv", statistics);
    RunExperiment(100.0, "Results/d_100.csv", statistics);

    statistics.close();

}
