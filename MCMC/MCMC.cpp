#include "MCMC.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iomanip>

using namespace std;

const vector<uint32_t> alphabet = {
    U'À', U'Á', U'Â', U'Ã', U'¥', U'Ä', U'Å', U'ª',
    U'Æ', U'Ç', U'È', U'²', U'¯', U'É', U'Ê', U'Ë',
    U'Ì', U'Í', U'Î', U'Ï', U'Ð', U'Ñ', U'Ò', U'Ó',
    U'Ô', U'Õ', U'Ö', U'×', U'Ø', U'Ù', U'Ü', U'Þ',
    U'ß'
};

const vector<uint32_t> lowercaseAlphabet = {
    U'à', U'á', U'â', U'ã', U'´', U'ä', U'å', U'º',
    U'æ', U'ç', U'è', U'³', U'¿', U'é', U'ê', U'ë',
    U'ì', U'í', U'î', U'ï', U'ð', U'ñ', U'ò', U'ó',
    U'ô', U'õ', U'ö', U'÷', U'ø', U'ù', U'ü', U'þ',
    U'ÿ'
};


uint32_t DecodeUTF8(const string& s, size_t& i) {
    unsigned char c = static_cast<unsigned char>(s[i]);

    if (c < 0x80) {
        i++;
        return c;
    }

    if ((c & 0xE0) == 0xC0 && i + 1 < s.size()) {
        uint32_t result = (c & 0x1F) << 6;
        result |= static_cast<unsigned char>(s[i + 1]) & 0x3F;
        i += 2;
        return result;
    }

    if ((c & 0xF0) == 0xE0 && i + 2 < s.size()) {
        uint32_t result = (c & 0x0F) << 12;
        result |= (static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6;
        result |= static_cast<unsigned char>(s[i + 2]) & 0x3F;
        i += 3;
        return result;
    }

    if ((c & 0xF8) == 0xF0 && i + 3 < s.size()) {
        uint32_t result = (c & 0x07) << 18;
        result |= (static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12;
        result |= (static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6;
        result |= static_cast<unsigned char>(s[i + 3]) & 0x3F;
        i += 4;
        return result;
    }

    i++;
    return 0;
}


void EncodeUTF8(uint32_t ch, string& result) {
    if (ch <= 0x7F) {
        result += static_cast<char>(ch);
    }
    else if (ch <= 0x7FF) {
        result += static_cast<char>(0xC0 | (ch >> 6));
        result += static_cast<char>(0x80 | (ch & 0x3F));
    }
    else if (ch <= 0xFFFF) {
        result += static_cast<char>(0xE0 | (ch >> 12));
        result += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (ch & 0x3F));
    }
    else {
        result += static_cast<char>(0xF0 | (ch >> 18));
        result += static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (ch & 0x3F));
    }
}


int LetterToIndex(uint32_t ch) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (alphabet[i] == ch || lowercaseAlphabet[i] == ch)
            return i;
    }

    return -1;
}


Text ReadText(const string& filename) {
    ifstream file(filename, ios::binary);

    if (!file.is_open()) {
        cerr << "Cannot open file: " << filename << endl;
        return {};
    }

    string content(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );

    Text result;

    size_t i = 0;

    if (content.size() >= 3 &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        i = 3;
    }

    while (i < content.size()) {
        uint32_t ch = DecodeUTF8(content, i);
        int index = LetterToIndex(ch);

        if (index != -1) {
            result.push_back(index);
        }
        else if (!result.empty() && result.back() != -1) {
            result.push_back(-1);
        }
    }

    if (!result.empty() && result.back() == -1)
        result.pop_back();

    return result;
}


void WriteText(const Text& text, const string& filename) {
    ofstream file(filename, ios::binary);

    if (!file.is_open()) {
        cerr << "Cannot create file: " << filename << endl;
        return;
    }

    string result;

    for (int index : text) {
        if (index == -1) {
            result += ' ';
        }
        else if (index >= 0 && index < ALPHABET_SIZE) {
            EncodeUTF8(alphabet[index], result);
        }
    }

    file << result;
}


void BuildBigramMatrix(const Text& text,BigramMatrix& matrix,const string& outputFile) {
    for (auto& row : matrix)
        row.fill(0);

    long long letterCount = 0;
    long long total = 0;

    for (int ch : text) {
        if (ch >= 0)
            letterCount++;
    }

    for (size_t i = 0; i + 1 < text.size(); i++) {
        int first = text[i];
        int second = text[i + 1];

        if (first >= 0 && second >= 0) {
            matrix[first][second]++;
            total++;
        }
    }

    ofstream file(outputFile);

    if (!file.is_open()) {
        cerr << "Cannot create matrix file." << endl;
        return;
    }

    file << "Letter";

    for (int i = 0; i < ALPHABET_SIZE; i++)
        file << "," << i;

    file << "\n";

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        file << i;

        for (int j = 0; j < ALPHABET_SIZE; j++)
            file << "," << matrix[i][j];

        file << "\n";
    }

    cout << "Bigram matrix was saved." << endl;
    cout << "Ukrainian letters: " << letterCount << endl;
    cout << "Counted bigrams: " << total << endl;
}


Key GenerateRandomKey(mt19937& rng) {
    Key key;

    iota(key.begin(), key.end(), 0);
    shuffle(key.begin(), key.end(), rng);

    return key;
}

Text EncryptText(const Text& text, const Key& key) {
    Text result = text;

    Key inverseKey{};

    for (int i = 0; i < ALPHABET_SIZE; i++)
        inverseKey[key[i]] = i;

    for (int& ch : result) {
        if (ch >= 0)
            ch = inverseKey[ch];
    }

    return result;
}


Text DecryptText(const Text& text, const Key& key) {
    Text result = text;

    for (int& ch : result) {
        if (ch >= 0)
            ch = key[ch];
    }

    return result;
}

double CalculateScore(const BigramMatrix& cipherMatrix,const BigramMatrix& trainingMatrix,const Key& key) {
    const double smoothing = 0.1;

    long long rowSum[ALPHABET_SIZE]{};

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        for (int j = 0; j < ALPHABET_SIZE; j++) {
            rowSum[i] += trainingMatrix[i][j];
        }
    }

    double score = 0.0;
    long long total = 0;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        for (int j = 0; j < ALPHABET_SIZE; j++) {

            int count = static_cast<int>(cipherMatrix[i][j]);

            if (count == 0)
                continue;

            int plainFirst = key[i];
            int plainSecond = key[j];

            double probability =
                (trainingMatrix[plainFirst][plainSecond] + smoothing) /
                (rowSum[plainFirst] + smoothing * ALPHABET_SIZE);

            score += count * log(probability);
            total += count;
        }
    }

    if (total == 0)
        return -1e100;

    return score / total;
}


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
)
{
    Key currentKey = initialKey;
    Key bestKey = currentKey;

    double currentScore =
        CalculateScore(cipherMatrix, trainingMatrix, currentKey);

    double bestScore = currentScore;

    uniform_int_distribution<int> letterDist(0, ALPHABET_SIZE - 1);
    uniform_real_distribution<double> probabilityDist(0.0, 1.0);

    bool reached95 = false;
    int firstSuccessfulIteration = -1;

    for (int iteration = 0; iteration < iterations; iteration++) {

        Key candidateKey = currentKey;

        int first = letterDist(rng);
        int second = letterDist(rng);

        while (second == first)
            second = letterDist(rng);

        swap(candidateKey[first], candidateKey[second]);

        double candidateScore =
            CalculateScore(cipherMatrix, trainingMatrix, candidateKey);

        double difference = candidateScore - currentScore;

        if (difference >= 0 ||
            probabilityDist(rng) < exp(difference / temperature)) {

            currentKey = candidateKey;
            currentScore = candidateScore;
        }

        if (currentScore > bestScore) {
            bestScore = currentScore;
            bestKey = currentKey;
        }

        if ((iteration + 1) % 10000 == 0) {

            Text decryptedText = DecryptText(cipherText, bestKey);

            double accuracy =
                CalculateAccuracy(originalText, decryptedText);

            statisticsFile
                << runNumber << ","
                << iteration + 1 << ","
                << fixed << setprecision(6) << bestScore << ","
                << setprecision(2) << accuracy << "\n";

            cout << "Iteration: " << iteration + 1
                << " | Best score: " << bestScore
                << " | Accuracy: " << accuracy << "%\n";

            if (!reached95 && accuracy >= 95.0) {
                reached95 = true;
                firstSuccessfulIteration = iteration + 1;
            }
        }
    }

    if (reached95) {
        cout << "95% accuracy first reached at iteration: "
            << firstSuccessfulIteration << endl;
    }
    else {
        cout << "95% accuracy was not reached in this run." << endl;
    }

    return bestKey;
}



void SaveKey(const Key& key, const string& filename) {
    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "Cannot save key." << endl;
        return;
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        file << i << " -> " << key[i] << "\n";
    }
}



double CalculateAccuracy(const Text& original, const Text& decrypted) {
    long long correct = 0;
    long long total = 0;

    size_t length = min(original.size(), decrypted.size());

    for (size_t i = 0; i < length; i++) {
        if (original[i] >= 0 && decrypted[i] >= 0) {
            total++;

            if (original[i] == decrypted[i])
                correct++;
        }
    }

    if (total == 0)
        return 0.0;

    return 100.0 * correct / total;
}