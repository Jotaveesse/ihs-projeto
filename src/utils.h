#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <algorithm>
#include <cctype> // For std::tolower, std::isalpha
#include <iostream> // For std::cerr
#include <mutex>
#include <cstdint> // For uint64_t

bool isVowel(char c);
bool isEvenDigit(char c);
bool containsLetter(const std::string& str);
char intToHexChar(int value);
void subtractTimer(int* timer, int value, std::mutex& timerMutex);
unsigned int vectorToBinary(const std::vector<int>& positions);
int getCorrectGreenCombination(std::vector<int> blinkPeriods);

#endif // UTILS_H