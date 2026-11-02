#include "MCMC.h"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

int CalculateDeltaEnergy(const vector<vector<int>>& lattice, int i, int j){
    const int SIZE = lattice.size();

    int spin = lattice[i][j];

    int neighbors =
        lattice[(i - 1 + SIZE) % SIZE][j] +
        lattice[(i + 1) % SIZE][j] +
        lattice[i][(j - 1 + SIZE) % SIZE] +
        lattice[i][(j + 1) % SIZE];

    return 2 * spin * neighbors;
}


double CalculateEnergy(const vector<vector<int>>& lattice) {
    const int SIZE = lattice.size();

    double energy = 0.0;

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            energy -= lattice[i][j] * (lattice[(i + 1) % SIZE][j] + lattice[i][(j + 1) % SIZE]);
        }
    }

    return energy;
}


double CalculateMagnetization(const vector<vector<int>>& lattice) {
    const int SIZE = lattice.size();
    int magnetization = 0;

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            magnetization += lattice[i][j];
        }
    }

    return static_cast<double>(magnetization) / (SIZE * SIZE);
}


void PrintLattice(const vector<vector<int>>& lattice) {
    const int SIZE = lattice.size();

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            cout << (lattice[i][j] == 1 ? "+ " : "- ");
        }

        cout << endl;
    }
}


void SaveLattice(const vector<vector<int>>& lattice, const string& filename) {
    ofstream file(filename);

    if (!file.is_open()) {
        cout << "Error: cannot open " << filename << endl;
        return;
    }

    const int SIZE = lattice.size();

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            file << lattice[i][j];

            if (j < SIZE - 1)
                file << ",";
        }

        file << "\n";
    }

    file.close();
}


void RunIsing(double beta, mt19937& gen) {
    const int SIZE = 30;
    const int ITERATIONS = 100000;
    const int SAVE_EVERY = 100;

    uniform_int_distribution<int> spinDistribution(0, 1);

    vector<vector<int>> lattice(SIZE, vector<int>(SIZE));

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            lattice[i][j] = spinDistribution(gen) == 0 ? -1 : 1;
        }
    }

    cout << "\n====================================\n";
    cout << "Ising Model" << endl;
    cout << "Beta: " << beta << endl;
    cout << "Grid size: " << SIZE << "x" << SIZE << endl;

    cout << "\nInitial lattice:\n";

    PrintLattice(lattice);


    ostringstream betaName;
    betaName << fixed << setprecision(2) << beta;

    string csvName = "Results/Ising_beta_" +
        betaName.str() + ".csv";

    string latticeName = "Results/Ising_lattice_" +
        betaName.str() + ".csv";


    ofstream file(csvName);

    if (!file.is_open()) {
        cout << "Error: cannot open " << csvName << endl;
        return;
    }

    file << "Iteration,Energy,Magnetization,AcceptanceRate\n";
    file << fixed << setprecision(8);

    int accepted = 0;

    uniform_int_distribution<int> indexDistribution(0, SIZE - 1);
    uniform_real_distribution<double> uniform(0.0, 1.0);

    for (int t = 1; t <= ITERATIONS; t++) {
        int i = indexDistribution(gen);
        int j = indexDistribution(gen);

        int deltaE = CalculateDeltaEnergy(lattice, i, j);

        double probability = min(1.0, exp(-beta * deltaE));

        if (uniform(gen) < probability) {
            lattice[i][j] *= -1;
            accepted++;
        }

        if (t % SAVE_EVERY == 0) {
            double energy = CalculateEnergy(lattice);
            double magnetization =
                CalculateMagnetization(lattice);

            file << t << "," << energy << "," << magnetization << "," << 100.0 * accepted / t << "\n";
        }
    }

    file.close();

    SaveLattice(lattice, latticeName);

    double energy = CalculateEnergy(lattice);
    double magnetization = CalculateMagnetization(lattice);

    cout << "\nSimulation results:\n";
    cout << fixed << setprecision(6);

    cout << "Beta: " << beta << endl;
    cout << "Iterations: " << ITERATIONS << endl;
    cout << "Accepted transitions: " << accepted << endl;

    cout << "Acceptance rate: " << 100.0 * accepted / ITERATIONS << "%" << endl;

    cout << "Energy: " << energy << endl;
    cout << "Magnetization: " << magnetization << endl;

    cout << "Results saved to: " << csvName << endl;
    cout << "Final lattice saved to: " << latticeName << endl;
}


void Ising() {
    random_device rd;
    mt19937 gen(rd());

    const double betas[] = { 0.2, 0.44, 0.6 };

    for (double beta : betas) {
        RunIsing(beta, gen);
    }
}
