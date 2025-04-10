#include "utils.h"
#include "constants.h"
#include "peripherals/seven_segment_displays/seven_segment_displays.h"

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

std::string intToBinaryString(int number) {
    std::bitset<18> binary(number);
    return binary.to_string();
}

std::string getIdString(SevenSegmentDisplays *sevenSegment) {
    std::string id;
    int idHigher = sevenSegment->getNumberFromDisplay(5);
    int idLower = sevenSegment->getNumberFromDisplay(4);
    id.push_back(intToHexChar(idHigher));
    id.push_back(intToHexChar(idLower));
    return id;
}

int getCombinationGreenLeds(std::vector<int> blinkPeriods) {
    std::vector<int> chosenNumbers(blinkPeriods.size(), -1);
    for (int i = blinkPeriods.size() - 1; i >= 0; --i) {
        int period = (blinkPeriods[i] / 1000) - 1;
        int chosenNum = -1;
        bool alreadyChosen = true;
        for (unsigned int j = 0; alreadyChosen; ++j) {
            chosenNum = greenOrder[period][j];
            alreadyChosen = count(chosenNumbers.begin(), chosenNumbers.end(), chosenNum) > 0;
        }
        chosenNumbers[i] = chosenNum;
    }
    return vectorToBinary(chosenNumbers);
}

int getCombinationRedLeds(std::vector<int> ledModes, const std::string& id) {
    unsigned int correctCombinationNumber = 0;
    int offCount = 0;
    int onCount = 0;
    int blinkCount = 0;

    for (unsigned int i = 0; i < ledModes.size(); ++i)
    {
        int mode = ledModes[i];

        switch (mode)
        {
        case 0: // Off
            offCount++;
            break;
        case 1: // On
            onCount++;
            break;
        case 2: // Blink
            blinkCount++;
            break;
        }
    }

    switch (ledModes[ledModes.size() - 1]) {
        case 1: // On
            if (blinkCount == offCount) {
                correctCombinationNumber = 0b000000000000000111;
            } else if (offCount > blinkCount && std::any_of(id.begin(), id.end(), isVowel)) {
                correctCombinationNumber = 0b000000000000001111;
            } else if (offCount > 5) {
                correctCombinationNumber = 0b111000000000000000;
            } else if (onCount > 6) {
                correctCombinationNumber = 0b000000000000111111;
            } else {
                correctCombinationNumber = 0b000000000000000000;
            }
            break;
        case 2: // Blink
            if (onCount > blinkCount && std::any_of(id.begin(), id.end(), isEvenDigit)) {
                correctCombinationNumber = 0b000000000000011111;
            } else if (onCount < 6) {
                correctCombinationNumber = 0b111000000000000001;
            } else if (offCount == onCount) {
                correctCombinationNumber = 0b111110000000011111;
            } else if (blinkCount > offCount) {
                correctCombinationNumber = 0b000000000000011111;
            } else {
                correctCombinationNumber = 0b000000000000000000;
            }
            break;
        case 0: // Off
            if (blinkCount < 5) {
                correctCombinationNumber = 0b111110000000000001;
            } else if (blinkCount > offCount && containsLetter(id)) {
                correctCombinationNumber = 0b111100000000000000;
            } else if (onCount > blinkCount) {
                correctCombinationNumber = 0b000000001111111111;
            } else if (blinkCount == offCount) {
                correctCombinationNumber = 0b100000000000000000;
            } else {
                correctCombinationNumber = 0b000000000000000000;
            }
            break;
        default:
            correctCombinationNumber = 0;
            break;
    }

    return correctCombinationNumber;
}

std::array<int, 4> getCorrectCombinationMemory(const std::vector<int>& displayedNumbers) {
    std::array<int, 4> correctButtons; // Array to store correct buttons
    std::vector<int> pressedPositions(4, 0); // Vector to store pressed positions

    for (int stage = 0; stage < 4; ++stage) {
        switch (stage + 1) {
            case 1:
                switch (displayedNumbers[stage]) {
                    case 1:
                    case 2:
                        correctButtons[stage] = 2;
                        pressedPositions[stage] = 2;
                        break;
                    case 3:
                        correctButtons[stage] = 1;
                        pressedPositions[stage] = 1;
                        break;
                    case 4:
                        correctButtons[stage] = 4;
                        pressedPositions[stage] = 4;
                        break;
                }
                break;
            case 2:
                switch (displayedNumbers[stage]) {
                    case 1:
                        correctButtons[stage] = displayedNumbers[0];
                        pressedPositions[stage] = displayedNumbers[0];
                        break;
                    case 2:
                        correctButtons[stage] = pressedPositions[0];
                        pressedPositions[stage] = pressedPositions[0];
                        break;
                    case 3:
                        correctButtons[stage] = 1;
                        pressedPositions[stage] = 1;
                        break;
                    case 4:
                        correctButtons[stage] = pressedPositions[0];
                        pressedPositions[stage] = pressedPositions[0];
                        break;
                }
                break;
            case 3:
                switch (displayedNumbers[stage]) {
                    case 1:
                        correctButtons[stage] = pressedPositions[1];
                        pressedPositions[stage] = pressedPositions[1];
                        break;
                    case 2:
                        correctButtons[stage] = pressedPositions[0];
                        pressedPositions[stage] = pressedPositions[0];
                        break;
                    case 3:
                        correctButtons[stage] = 3;
                        pressedPositions[stage] = 3;
                        break;
                    case 4:
                        correctButtons[stage] = displayedNumbers[1];
                        pressedPositions[stage] = displayedNumbers[1];
                        break;
                }
                break;
            case 4:
                switch (displayedNumbers[stage]) {
                    case 1:
                        correctButtons[stage] = pressedPositions[1];
                        pressedPositions[stage] = pressedPositions[1];
                        break;
                    case 2:
                        correctButtons[stage] = pressedPositions[0];
                        pressedPositions[stage] = pressedPositions[0];
                        break;
                    case 3:
                        correctButtons[stage] = pressedPositions[1];
                        pressedPositions[stage] = pressedPositions[1];
                        break;
                    case 4:
                        correctButtons[stage] = pressedPositions[2];
                        pressedPositions[stage] = pressedPositions[2];
                        break;
                }
                break;
        }
    }

    return correctButtons;
}