#include "utils.h"
#include "constants.h"

bool isVowel(char c) {
    c = std::tolower(c);
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}

bool isEvenDigit(char c) {
    return (c == '0' || c == '2' || c == '4' || c == '6' || c == '8');
}

bool containsLetter(const std::string& str) {
    return std::any_of(str.begin(), str.end(), ::isalpha);
}

char intToHexChar(int value) {
    char hexChars = 0;
    if (value >= 0 && value <= 15) {
        if (value < 10) {
            hexChars = '0' + value;
        } else {
            hexChars = 'A' + value - 10;
        }
    }
    return hexChars;
}

void subtractTimer(int* timer, int value, std::mutex& timerMutex) {
    {
        std::lock_guard<std::mutex> lock(timerMutex);
        *timer -= value;
    }
}

unsigned int vectorToBinary(const std::vector<int>& positions) {
    unsigned int binaryNumber = 0;
    for (int position : positions) {
        if (position >= 0 && position <= 17) {
            binaryNumber |= (1ULL << (position));
        } else {
            std::cerr << "Warning: Invalid position " << position << ". Ignoring." << std::endl;
        }
    }
    return binaryNumber;
}

int getCorrectGreenCombination(std::vector<int> blinkPeriods) {
    std::vector<int> chosenNumbers(blinkPeriods.size(), -1);
    std::cout << "green leds: ";
    for (int i = blinkPeriods.size() - 1; i >= 0; --i) {
        int period = (blinkPeriods[i] / 1000) - 1;
        int chosenNum = -1;
        bool alreadyChosen = true;
        for (unsigned int j = 0; alreadyChosen; ++j) {
            chosenNum = greenOrder[period][j];
            alreadyChosen = count(chosenNumbers.begin(), chosenNumbers.end(), chosenNum) > 0;
        }
        std::cout << chosenNum << " ";
        chosenNumbers[i] = chosenNum;
    }
    std::cout << std::endl;
    return vectorToBinary(chosenNumbers);
}