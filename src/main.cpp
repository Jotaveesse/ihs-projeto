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

std::mutex deviceMutex; // Single mutex for all updates

void buttons_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd)
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

void switches_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd)
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

void red_leds_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd)
{
    unsigned int switchesStates = 0;
    unsigned int buttonStates = 0;
    bool deactivated = false;

    while (buttonStates != 15)
    {
        if (!deactivated)
        {
            buttonStates = buttons->getStatesAsNumber();
            switchesStates = switches->getStatesAsNumber();

            if(buttons->isButtonPressedLong(0, 2000)){
                redLeds->blink(0, 1000);
                redLeds->blink(1, 2000);
                redLeds->blink(2, 4000);
            }


        }
        else{
            redLeds->setAllStates(true);
        }

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            redLeds->update();
        }
    }
}

void green_leds_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd)
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

void seven_segment_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd)
{
    unsigned int switchesStates = 0;
    unsigned int buttonStates = 0;

    while (buttonStates != 15)
    {
        buttonStates = buttons->getStatesAsNumber();
        switchesStates = switches->getStatesAsNumber();

        sevenSegment->setAllDisplaysFromNumber(switchesStates);

        {
            std::lock_guard<std::mutex> lock(deviceMutex);
            sevenSegment->update();
        }
    }
}

void lcd_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd)
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

#pragma omp parallel sections num_threads(6)
    {
#pragma omp section
        {
            buttons_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd);
        }
#pragma omp section
        {
            switches_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd);
        }
#pragma omp section
        {
            red_leds_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd);
        }
#pragma omp section
        {
            green_leds_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd);
        }
#pragma omp section
        {
            seven_segment_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd);
        }
#pragma omp section
        {
            lcd_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd);
        }
    }

    if (fileDescriptor != -1)
    {
        close(fileDescriptor);
    }

    return 0;
}
