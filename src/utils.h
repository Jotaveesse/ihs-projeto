#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <algorithm>
#include <cctype> // For std::tolower, std::isalpha
#include <iostream> // For std::cerr
#include <mutex>
#include <cstdint> // For uint64_t
#include <bitset>
#include <limits>
#include "peripherals/seven_segment_displays/seven_segment_displays.h"


bool isVowel(char c);
bool isEvenDigit(char c);
bool containsLetter(const std::string& str);
char intToHexChar(int value);
void subtractTimer(int* timer, int value, std::mutex& timerMutex);
unsigned int vectorToBinary(const std::vector<int>& positions);
int getCombinationGreenLeds(std::vector<int> blinkPeriods);
int getCombinationRedLeds(std::vector<int> ledModes, const std::string& id);
std::array<int, 4> getCorrectCombinationMemory(const std::vector<int>& displayedNumbers);

std::string getIdString(SevenSegmentDisplays *sevenSegment);
std::string intToBinaryString(int number);

#endif // UTILS_H