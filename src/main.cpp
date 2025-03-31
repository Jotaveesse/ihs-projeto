#include "leds/leds.h"
#include "switches/switches.h"
#include "buttons/buttons.h"
#include "seven_segment_displays/seven_segment_displays.h"
#include "lcd/lcd.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "ioctl_cmds.h"
#include <chrono>
#include <thread>
#include <omp.h>
#include <mutex>
#include <random>
#include <algorithm>

char defeatSymbol = static_cast<char>(0xFF);
int buttonPressDelay = 1300;
std::vector<char> array1 = {
    static_cast<char>(0xC0), // 11000000
    static_cast<char>(0xD0), // 11010000
    static_cast<char>(0xAE), // 10101110
    static_cast<char>(0xD3), // 11010011
    static_cast<char>(0xB7), // 10110111
    static_cast<char>(0xFC), // 11111100
    static_cast<char>(0xC2), // 11000010
    static_cast<char>(0xD6), // 11010110
    static_cast<char>(0xD1), // 11010001
    static_cast<char>(0xD5)  // 11010101
};

std::vector<char> array2 = {
    static_cast<char>(0x80), // 10000000
    static_cast<char>(0x81), // 10000001
    static_cast<char>(0x82), // 10000010
    static_cast<char>(0xC6), // 11000110
    static_cast<char>(0x46), // 01000110
    static_cast<char>(0xAF), // 10101111
    static_cast<char>(0xF8), // 11111000
    static_cast<char>(0xFE), // 11111110
    static_cast<char>(0xA4), // 10100100
    static_cast<char>(0x56)  // 01010110
};

std::mutex timerMutex;
std::mutex deviceMutex;

bool isVowel(char c)
{
    c = std::tolower(c);
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}

bool isEvenDigit(char c)
{
    return (c == '0' || c == '2' || c == '4' || c == '6' || c == '8');
}

bool containsLetter(const std::string &str)
{
    return std::any_of(str.begin(), str.end(), ::isalpha);
}

char intToHexChar(int value)
{
    char hexChars = 0;

    if (value >= 0 && value <= 15)
    {
        if (value < 10)
        {
            hexChars = '0' + value; // '0' to '9'
        }
        else
        {
            hexChars = 'A' + value - 10; // 'A' to 'F'
        }
    }

    return hexChars;
}

void setTimer(int *timer, int value)
{
    {
        std::lock_guard<std::mutex> lock(timerMutex);
        *timer = value;
    }
}

void subtractTimer(int *timer, int value)
{
    {
        std::lock_guard<std::mutex> lock(timerMutex);
        *timer -= value;
    }
}

void addTimer(int *timer, int value)
{
    {
        std::lock_guard<std::mutex> lock(timerMutex);
        *timer += value;
    }
}

void buttons_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd, int *timer)
{
    unsigned int buttonStates = 0;

    while (*timer > 0)
    {
        buttonStates = buttons->getStatesAsNumber();

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            buttons->update();
        }
    }
}

void switches_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd, int *timer)
{
    unsigned int buttonStates = 0;

    while (*timer > 0)
    {
        buttonStates = buttons->getStatesAsNumber();

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            switches->update();
        }
    }
}

void red_leds_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd, int *timer)
{
    unsigned int switchesStates = 0;
    unsigned int buttonStates = 0;

    bool heldButton = false;
    bool deactivated = false;
    int offCount = 0;
    int onCount = 0;
    int blinkCount = 0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 2);

    std::vector<int> ledModes(redLeds->getCount());

    for (unsigned int i = 0; i < redLeds->getCount(); ++i)
    {
        int mode = dist(gen);

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
        ledModes[i] = mode;
    }

    while (!deactivated && *timer > 0)
    {
        buttonStates = buttons->getStatesAsNumber();
        switchesStates = switches->getStatesAsNumber();

        int idHigher = sevenSegment->getNumberFromDisplay(5);
        int idLower = sevenSegment->getNumberFromDisplay(4);

        std::string id;
        id.push_back(intToHexChar(idHigher));
        id.push_back(intToHexChar(idLower));

        if (buttons->isButtonPressedLong(3, buttonPressDelay))
        {
            heldButton = true;
        }

        for (unsigned int i = 0; i < redLeds->getCount(); ++i)
        {
            switch (ledModes[i])
            {
            case 0: // Off
                redLeds->setState(i, false);
                break;
            case 1: // On
                redLeds->setState(i, true);
                break;
            case 2: // Blink
                redLeds->blink(i, 1000);
                break;
            }
        }

        // confirmação da escolha
        if (heldButton && !buttons->isButtonPressed(3))
        {
            heldButton = false;
            bool correctCombination = false;

            switch (ledModes[redLeds->getCount() - 1])
            {
            case 1: // On
                if (blinkCount == offCount)
                {
                    correctCombination = switchesStates == 0b000000000000000111;
                }
                else if (offCount > blinkCount)
                {
                    correctCombination = switchesStates == 0b000000000000001111;
                }
                else if (offCount > 5 && std::any_of(id.begin(), id.end(), isVowel))
                {
                    correctCombination = switchesStates == 0b111000000000000000;
                }
                else if (onCount > 6)
                {
                    correctCombination = switchesStates == 0b000000000000111111;
                }
                else
                {
                    correctCombination = switchesStates == 0b000000000000000000;
                }
                break;
            case 2: // Blink
                if (onCount > blinkCount && std::any_of(id.begin(), id.end(), isEvenDigit))
                {
                    correctCombination = switchesStates == 0b000000111110000000;
                }
                else if (onCount < 6)
                {
                    correctCombination = switchesStates == 0b111000000000000001;
                }
                else if (offCount == onCount)
                {
                    correctCombination = switchesStates == 0b111110000000000111;
                }
                else if (blinkCount > offCount)
                {
                    correctCombination = switchesStates == 0b000000111110000000;
                }
                else
                {
                    correctCombination = switchesStates == 0b000000000000000000;
                }
                break;
            case 0: // Off
                if (blinkCount < 5)
                {
                    correctCombination = switchesStates == 0b111110000000000001;
                }
                else if (blinkCount > offCount && containsLetter(id))
                {
                    correctCombination = switchesStates == 0b111100000000000000;
                }
                else if (onCount > blinkCount)
                {
                    correctCombination = switchesStates == 0b000000001111111111;
                }
                else if (blinkCount == offCount)
                {
                    correctCombination = switchesStates == 0b100000000000000000;
                }
                else
                {
                    correctCombination = switchesStates == 0b000000000000000000;
                }
                break;
            }

            if (correctCombination)
            {
                deactivated = true;
            }
            else
            {
                subtractTimer(timer, 15000000);
            }
        }
        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            redLeds->update();
        }
    }

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        redLeds->setAllStates(*timer <= 0);
        redLeds->update();
    }
}

std::vector<std::vector<int>> greenOrder = {
    {6, 4, 11, 1, 14, 7, 0, 10, 16, 5, 9, 2, 15, 8, 13, 3, 12, 17},
    {11, 2, 17, 0, 8, 13, 5, 16, 4, 14, 7, 1, 10, 3, 12, 6, 15, 9},
    {4, 11, 13, 14, 6, 3, 10, 7, 12, 1, 15, 9, 0, 16, 2, 8, 17, 5},
    {17, 0, 3, 4, 8, 13, 6, 12, 11, 10, 7, 15, 1, 16, 5, 9, 14, 2}};

unsigned int vectorToBinary(const std::vector<int> &positions)
{
    unsigned int binaryNumber = 0;

    for (int position : positions)
    {
        if (position >= 0 && position <= 17)
        {
            binaryNumber |= (1ULL << (position)); // Set the bit at the given position
        }
        else
        {
            std::cerr << "Warning: Invalid position " << position << ". Ignoring." << std::endl;
        }
    }

    return binaryNumber;
}

int getCorrectGreenCombination(std::vector<int> blinkPeriods)
{
    std::vector<int> chosenNumbers(blinkPeriods.size(), -1);
    std::cout << "greeen leds: ";
    for (unsigned int i = 0; i < blinkPeriods.size(); ++i)
    {
        int period = (blinkPeriods[i] / 1000) - 1;
        int chosenNum = -1;
        bool alreadyChosen = true;

        for (unsigned int j = 0; alreadyChosen; ++j)
        {
            chosenNum = greenOrder[period][j];
            alreadyChosen = count(chosenNumbers.begin(), chosenNumbers.end(), chosenNum) > 0;
        }

        std::cout << chosenNum << " ";
        chosenNumbers[i] = chosenNum;
    }

    std::cout << std::endl;

    return vectorToBinary(chosenNumbers);
}

void green_leds_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd, int *timer)
{
    unsigned int buttonStates = 0;
    unsigned int switchesStates = 0;
    bool heldButton = false;
    bool deactivated = false;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 4);

    std::vector<int> blinkPeriods(greenLeds->getCount());

    for (unsigned int i = 0; i < greenLeds->getCount(); ++i)
    {
        int period = dist(gen);
        blinkPeriods[i] = period * 1000;
    }

    int combination = getCorrectGreenCombination(blinkPeriods);

    // std::cout << combination << std::endl;

    while (!deactivated && *timer > 0)
    {
        buttonStates = buttons->getStatesAsNumber();
        switchesStates = switches->getStatesAsNumber();

        if (buttons->isButtonPressedLong(2, buttonPressDelay))
        {
            heldButton = true;
        }

        for (unsigned int i = 0; i < greenLeds->getCount(); ++i)
        {
            greenLeds->blink(i, blinkPeriods[i]);
        }

        // confirmação da escolha
        if (heldButton && !buttons->isButtonPressed(2))
        {
            heldButton = false;
            bool correctCombination = switches->getStatesAsNumber() == combination;

            if (correctCombination)
            {
                deactivated = true;
            }
            else
            {
                subtractTimer(timer, 15000000);
            }
        }
        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            greenLeds->update();
        }
    }

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        greenLeds->setAllStates(*timer <= 0);
        greenLeds->update();
    }
}

void seven_segment_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd, int *timer)
{
    unsigned int switchesStates = 0;
    unsigned int buttonStates = 0;

    bool deactivated = false;

    int initialTimer = *timer;
    unsigned int startTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    unsigned int currTime = startTime;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 4);

    std::uniform_int_distribution<> idDist(0, 15);
    int num1 = idDist(gen);
    int num2 = idDist(gen);
    sevenSegment->setDisplayFromNumber(5, num2);
    sevenSegment->setDisplayFromNumber(4, num1);

    int stage = 0;
    std::vector<int> displayedNumbers(4);
    std::vector<int> pressedPositions(4);

    for (unsigned int i = 0; i < 4; ++i)
    {
        int num = dist(gen);
        displayedNumbers[i] = num;
    }

    bool buttonReleased = false;
    int buttonPressed = -1;
    int chosenButton = -1;
    while (!deactivated && *timer > 0)
    {
        currTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        subtractTimer(timer, (currTime - startTime));
        startTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        int dig0 = (*timer / 1000000) % 10;
        int dig1 = ((*timer / 1000000) % 60) / 10;
        int dig2 = ((*timer / 1000000) / 60) % 10;
        int dig3 = ((*timer / 1000000) / 600) % 10;

        sevenSegment->setDisplayFromNumber(0, dig0);
        sevenSegment->setDisplayFromNumber(1, dig1);
        sevenSegment->setDisplayFromNumber(2, dig2);
        sevenSegment->setDisplayFromNumber(3, dig3);

        sevenSegment->setDisplayFromNumber(6, stage + 1);
        sevenSegment->setDisplayFromNumber(7, displayedNumbers[stage]);

        buttonReleased = false;
        if (buttons->isButtonPressed(0))
        {
            buttonPressed = 0;
        }
        if (buttons->isButtonPressed(1))
        {
            buttonPressed = 1;
        }
        if (buttons->isButtonPressed(2))
        {
            buttonPressed = 2;
        }
        if (buttons->isButtonPressed(3))
        {
            buttonPressed = 3;
        }
        if (buttons->isButtonPressedLong(0, buttonPressDelay) || buttons->isButtonPressedLong(1, buttonPressDelay) || buttons->isButtonPressedLong(2, buttonPressDelay) || buttons->isButtonPressedLong(3, buttonPressDelay))
        {
            buttonPressed = -1;
        }

        if (buttonPressed != -1 && !buttons->isButtonPressed(buttonPressed) && !buttons->isButtonPressedLong(buttonPressed, buttonPressDelay))
        {
            chosenButton = 4 - buttonPressed;
            buttonReleased = true;
            buttonPressed = -1;
        }

        if (buttonReleased)
        {
            bool correctButton = false;
            switch (stage + 1)
            {
            case 1:
                switch (displayedNumbers[stage])
                {
                case 1:
                    correctButton = chosenButton == 2;
                    pressedPositions[stage] = 2;
                    break;
                case 2:
                    correctButton = chosenButton == 2;
                    pressedPositions[stage] = 2;
                    break;
                case 3:
                    correctButton = chosenButton == 1;
                    pressedPositions[stage] = 1;
                    break;
                case 4:
                    correctButton = chosenButton == 4;
                    pressedPositions[stage] = 4;
                    break;
                }
                break;
            case 2:
                switch (displayedNumbers[stage])
                {
                case 1:
                    correctButton = chosenButton == displayedNumbers[1 - 1];
                    pressedPositions[stage] = displayedNumbers[1 - 1];
                    break;
                case 2:
                    correctButton = chosenButton == pressedPositions[1 - 1];
                    pressedPositions[stage] = pressedPositions[1 - 1];
                    break;
                case 3:
                    correctButton = chosenButton == 1;
                    pressedPositions[stage] = 1;
                    break;
                case 4:
                    correctButton = chosenButton == pressedPositions[1 - 1];
                    pressedPositions[stage] = pressedPositions[1 - 1];
                    break;
                }
                break;
            case 3:
                switch (displayedNumbers[stage])
                {
                case 1:
                    correctButton = chosenButton == pressedPositions[2 - 1];
                    pressedPositions[stage] = pressedPositions[2 - 1];
                    break;
                case 2:
                    correctButton = chosenButton == pressedPositions[1 - 1];
                    pressedPositions[stage] = pressedPositions[1 - 1];
                    break;
                case 3:
                    correctButton = chosenButton == 3;
                    pressedPositions[stage] = 3;
                    break;
                case 4:
                    correctButton = chosenButton == displayedNumbers[2 - 1];
                    pressedPositions[stage] = displayedNumbers[2 - 1];
                    break;
                }
                break;
            case 4:
                switch (displayedNumbers[stage])
                {
                case 1:
                    correctButton = chosenButton == pressedPositions[2 - 1];
                    pressedPositions[stage] = pressedPositions[2 - 1];
                    break;
                case 2:
                    correctButton = chosenButton == pressedPositions[1 - 1];
                    pressedPositions[stage] = pressedPositions[1 - 1];
                    break;
                case 3:
                    correctButton = chosenButton == pressedPositions[2 - 1];
                    pressedPositions[stage] = pressedPositions[2 - 1];
                    break;
                case 4:
                    correctButton = chosenButton == pressedPositions[3 - 1];
                    pressedPositions[stage] = pressedPositions[3 - 1];
                    break;
                }
                break;
            }

            std::cerr << correctButton << std::endl;

            if (correctButton)
            {
                stage += 1;
                if (stage >= 4)
                {
                    deactivated = true;
                }
            }
            else
            {
                stage = 0;
                subtractTimer(timer, 15000000);
            }
        }

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            sevenSegment->update();
        }
    }
    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        sevenSegment->setAllStates(*timer <= 0);
        sevenSegment->update();
    }
}

void lcd_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd, int *timer)
{
    unsigned int buttonStates = 0;
    unsigned int switchesStates = 0;
    bool heldButton = false;
    bool deactivated = false;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 7);

    int order = dist(gen);
    std::cout << "lcd: " << order << std::endl;
    std::vector<int> shownCombination;

    for (unsigned int i = 0; i < 10; ++i)
    {
        if (i != (((order - 1) % 10 + 10) % 10) && i != ((order + 5) % 10))
        {
            shownCombination.push_back(i);
            std::cout << i;
        }
    }
    std::cout << std::endl;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);

    std::shuffle(shownCombination.begin(), shownCombination.end(), rng);

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        lcd->clear();
        for (unsigned int i = 0; i < 8; i++)
        {
            lcd->sendWrite(array1[shownCombination[i]]);
            lcd->sendWrite(" ");
        }

        lcd->update();
    }

    while (!deactivated && *timer > 0)
    {
        buttonStates = buttons->getStatesAsNumber();
        switchesStates = switches->getStatesAsNumber();

        if (buttons->isButtonPressedLong(1, buttonPressDelay))
        {
            heldButton = true;
        }

        // confirmação da escolha
        if (heldButton && !buttons->isButtonPressed(1))
        {
            heldButton = false;
            bool correctCombination = switches->getStatesAsNumber() == (1 << order);

            if (correctCombination)
            {
                deactivated = true;
            }
            else
            {
                subtractTimer(timer, 15000000);
            }
        }
        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            lcd->update();
        }
    }

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        lcd->clear();

        if (*timer <= 0)
        {
            for (unsigned int i = 0; i < 16; i++)
            {
                lcd->sendWrite(defeatSymbol);
            }
        }

        lcd->update();
    }
}

int main() {
    bool restart = true;
    int initialTimerValue = 240; // Default timer value

    std::cout << "Enter the timer value in seconds: ";
    std::cin >> initialTimerValue;

    if (std::cin.fail()) {
        std::cerr << "Invalid input. Using default timer: 240 seconds." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        initialTimerValue = 240;
    }

    while (restart) {
        int fileDescriptor = -1;

        fileDescriptor = open("/dev/mydev", O_RDWR);
        if (fileDescriptor < 0) {
            std::cerr << "Failed to open device: " << strerror(errno) << std::endl;
            unsigned int number;
            std::cin >> number;
            return 1;
        }

        Leds redLeds(fileDescriptor, WR_RED_LEDS, 18);
        Leds greenLeds(fileDescriptor, WR_GREEN_LEDS, 8);
        Switches switches(fileDescriptor, RD_SWITCHES, 18);
        Buttons buttons(fileDescriptor, RD_PBUTTONS, 4);
        SevenSegmentDisplays sevenSegment(fileDescriptor, WR_L_DISPLAY, WR_R_DISPLAY, 8);
        LCD lcd(fileDescriptor, WR_LCD_DISPLAY);
        lcd.init();

        int timer = initialTimerValue * 1000000; // Timer in microseconds

#pragma omp parallel sections num_threads(6) shared(timer)
        {
#pragma omp section
            {
                buttons_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd, &timer);
            }
#pragma omp section
            {
                switches_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd, &timer);
            }
#pragma omp section
            {
                red_leds_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd, &timer);
            }
#pragma omp section
            {
                green_leds_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd, &timer);
            }
#pragma omp section
            {
                seven_segment_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd, &timer);
            }
#pragma omp section
            {
                lcd_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd, &timer);
            }
        }

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            redLeds.setAllStates(timer <= 0);
            greenLeds.setAllStates(timer <= 0);
            sevenSegment.setAllStates(timer <= 0);
            lcd.clear();

            if (timer <= 0) {
                for (unsigned int i = 0; i < 16; i++) {
                    lcd.sendWrite(defeatSymbol);
                }
            }

            redLeds.update();
            greenLeds.update();
            sevenSegment.update();
            lcd.update();

            if (fileDescriptor != -1) {
                close(fileDescriptor);
            }
        }

        std::cout << "Do you want to restart? (y/n): ";
        char choice;
        std::cin >> choice;

        if (choice != 'y' && choice != 'Y') {
            restart = false;
        }
    }

    return 0;
}