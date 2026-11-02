
#include <iostream>
#include <filesystem>
#include <random>
#include <chrono>
#include <iomanip>
#include <fstream>

#include "MCMC.h"

using namespace std;

int main() {

    MetropolisHastings();
    BetaDistribution();
    PowerLaw();
    BivariateNormal();
    Ising();

    filesystem::create_directories("Results");

    mt19937 rng(static_cast<unsigned int>(chrono::steady_clock::now().time_since_epoch().count()));

    cout << "========== TRAINING TEXT ==========\n";

    Text trainingText = ReadText("training.txt");

    if (trainingText.empty()) {
        cout << "Training text is empty or could not be read.\n";
        return 1;
    }

    cout << "Training text size: " << trainingText.size() << endl;

    BigramMatrix trainingMatrix{};

    BuildBigramMatrix(trainingText,trainingMatrix,"Results/bigram_matrix.csv");

    cout << "\n========== ORIGINAL TEXT ==========\n";

    Text originalText = ReadText("original.txt");

    if (originalText.empty()) {
        cout << "Original text is empty or could not be read.\n";
        return 1;
    }

    long long originalBigrams = 0;
    long long originalLetters = 0;

    for (int ch : originalText) {
        if (ch >= 0)
            originalLetters++;
    }

    for (size_t i = 0; i + 1 < originalText.size(); i++) {
        if (originalText[i] >= 0 && originalText[i + 1] >= 0)
            originalBigrams++;
    }

    cout << "Original text size: " << originalText.size() << endl;
    cout << "Original letters: " << originalLetters << endl;
    cout << "Original bigrams: " << originalBigrams << endl;

    cout << "Original text was read successfully.\n";
    cout << "\n========== ENCRYPTION ==========\n";

    Key secretKey = GenerateRandomKey(rng);

    Text cipherText = EncryptText(originalText, secretKey);

    WriteText(cipherText, "Results/ciphertext.txt");

    SaveKey(secretKey, "Results/secret_key.txt");


    BigramMatrix cipherMatrix{};

    BuildBigramMatrix(cipherText,cipherMatrix,"Results/cipher_bigram_matrix.csv");

    cout << "Encryption completed.\n";
    cout << "Ciphertext saved.\n";

    cout << "\n========== MCMC DECRYPTION ==========\n";

    const int RUNS = 5;
    const int ITERATIONS = 100000;
    const double TEMPERATURE = 0.02;

    Key bestKey{};
    double bestScore = -1e100;

    ofstream statisticsFile("Results/mcmc_statistics.csv");

    if (!statisticsFile.is_open()) {
        cout << "Cannot create statistics file.\n";
        return 1;
    }

    statisticsFile << "Run,Iteration,BestScore,Accuracy\n";

    for (int run = 0; run < RUNS; run++) {

        cout << "\nRun " << run + 1 << " of " << RUNS << endl;

        Key initialKey;

        if (run == 0) {
            for (int i = 0; i < ALPHABET_SIZE; i++)
                initialKey[i] = i;

            cout << "Initial key: identity\n";
        }
        else {
            initialKey = GenerateRandomKey(rng);

            cout << "Initial key: random\n";
        }

        Key currentKey = RunMCMC(cipherMatrix,trainingMatrix,initialKey,cipherText,originalText,ITERATIONS,TEMPERATURE,run + 1,statisticsFile,rng);

        double currentScore = CalculateScore(cipherMatrix,trainingMatrix,currentKey);

        cout << "Run score: " << currentScore << endl;

        if (currentScore > bestScore) {
            bestScore = currentScore;
            bestKey = currentKey;
        }

        Text decryptedText = DecryptText(cipherText, currentKey);

        WriteText(decryptedText,"Results/decrypted_run_" + to_string(run + 1) + ".txt");

        double accuracy = CalculateAccuracy(originalText, decryptedText);

        cout << "Accuracy: " << fixed << setprecision(2) << accuracy << "%\n";
    }

    statisticsFile.close();

    cout << "\n========== BEST RESULT ==========\n";

    Text bestDecryption = DecryptText(cipherText, bestKey);

    WriteText(bestDecryption, "Results/decrypted_best.txt");

    SaveKey(bestKey, "Results/recovered_key.txt");

    double finalAccuracy = CalculateAccuracy(originalText,bestDecryption );

    cout << "Best score: " << bestScore << endl;

    cout << "Final accuracy: " << fixed << setprecision(2) << finalAccuracy << "%\n";

    cout << "\nAll results saved in Results folder.\n";

    return 0;
}
