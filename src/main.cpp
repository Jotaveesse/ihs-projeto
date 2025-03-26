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

std::vector<char>
    array1 = {
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

    while (buttonStates != 15)
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

    while (buttonStates != 15)
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

        if (buttons->isButtonPressedLong(3, 2000))
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
                redLeds->blink(i, 500);
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
                    correctCombination = switchesStates == 0b111000000000000111;
                }
                break;
            case 2: // Blink
                if (onCount > blinkCount && std::any_of(id.begin(), id.end(), isEvenDigit))
                {
                    correctCombination = switchesStates == 0b000000111110000000;
                }
                else if (onCount < 7)
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
                    correctCombination = switchesStates == 0b000000111111111100;
                }
                else if (blinkCount == offCount)
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
                subtractTimer(timer, 15);
            }
        }
        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            redLeds->update();
        }
    }

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        redLeds->setAllStates(true);
        redLeds->update();
    }
}

void green_leds_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd, int *timer)
{
    unsigned int buttonStates = 0;

    while (buttonStates != 15)
    {
        buttonStates = buttons->getStatesAsNumber();

        greenLeds->setStatesFromNumber(buttonStates);

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            greenLeds->update();
        }
    }
}

void seven_segment_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd, int *timer)
{
    unsigned int switchesStates = 0;
    unsigned int buttonStates = 0;

    bool deactivated = false;
    int initialTimer = *timer;
    unsigned int startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    unsigned int currTime = startTime;
    while (!deactivated && *timer > 0)
    {
        currTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        setTimer(timer, initialTimer - (currTime - startTime) / 1000);

        int dig0 = *timer % 10;
        int dig1 = (*timer % 60) / 10;
        int dig2 = *timer / 60;
        int dig3 = *timer / 600;

        sevenSegment->setDisplayFromNumber(0, dig0);
        sevenSegment->setDisplayFromNumber(1, dig1);
        sevenSegment->setDisplayFromNumber(2, dig2);
        sevenSegment->setDisplayFromNumber(3, dig3);

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            sevenSegment->update();
        }
    }

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        sevenSegment->setAllStates(true);
        sevenSegment->update();
    }
}

void lcd_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd, int *timer)
{
    unsigned int switchesStates = 0;
    unsigned int buttonStates = 0;

    while (buttonStates != 15)
    {
        buttonStates = buttons->getStatesAsNumber();
        switchesStates = switches->getStatesAsNumber();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            lcd->clear();
            for (unsigned int i = 0; i < 10; i++)
            {
                lcd->sendWrite(array1[i]);
                lcd->sendWrite(" ");
            }

            lcd->update();
        }
    }
}

int main()
{
    int fileDescriptor = -1;

    fileDescriptor = open("/dev/mydev", O_RDWR);
    if (fileDescriptor < 0)
    {
        std::cerr << "Failed to open device: " << strerror(errno) << std::endl;
        return 1;
    }

    Leds redLeds(fileDescriptor, WR_RED_LEDS, 18);
    Leds greenLeds(fileDescriptor, WR_GREEN_LEDS, 9);
    Switches switches(fileDescriptor, RD_SWITCHES, 18);
    Buttons buttons(fileDescriptor, RD_PBUTTONS, 4);
    SevenSegmentDisplays sevenSegment(fileDescriptor, WR_L_DISPLAY, WR_R_DISPLAY, 8);
    LCD lcd(fileDescriptor, WR_LCD_DISPLAY);
    lcd.init();

    int timer = 120;

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

    if (fileDescriptor != -1)
    {
        close(fileDescriptor);
    }

    return 0;
}
