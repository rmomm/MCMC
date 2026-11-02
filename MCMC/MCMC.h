#pragma once

#include <string>
#include <vector>
#include <array>
#include <random>
#include <fstream>

using namespace std;

const int ALPHABET_SIZE = 33;

using Text = vector<int>;
using Key = array<int, ALPHABET_SIZE>;
using BigramMatrix = array<array<long long, ALPHABET_SIZE>,ALPHABET_SIZE>;

Text ReadText(const string& filename);
void WriteText(const Text& text, const string& filename);


void BuildBigramMatrix(const Text& text, BigramMatrix& matrix, const string& outputFile);


Key GenerateRandomKey(mt19937& rng);
Text EncryptText(const Text& text, const Key& key);
Text DecryptText(const Text& text, const Key& key);


//double CalculateScore(const Text& cipherText, const BigramMatrix& trainingMatrix, const Key& key);
double CalculateScore(const BigramMatrix& cipherMatrix,const BigramMatrix& trainingMatrix, const Key& key);

Key RunMCMC(
    const BigramMatrix& cipherMatrix,
    const BigramMatrix& trainingMatrix,
    const Key& initialKey,
    const Text& cipherText,
    const Text& originalText,
    int iterations,
    double temperature,
    int runNumber,
    ofstream& statisticsFile,
    mt19937& rng
);


void SaveKey(const Key& key, const string& filename);
double CalculateAccuracy(const Text& original, const Text& decrypted);

void MetropolisHastings();
void BetaDistribution();
void PowerLaw();
void BivariateNormal();
void Ising();