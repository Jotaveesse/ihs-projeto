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

void buttons_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd)
{
    unsigned int buttonStates = 0;

    while (buttonStates != 15)
    {

        buttonStates = buttons->getStatesAsNumber();

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        buttons->printStates();
        buttons->update();
    }
}

void switches_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd)
{
    unsigned int buttonStates = 0;

    while (buttonStates != 15)
    {

        buttonStates = buttons->getStatesAsNumber();

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        switches->printStates();
        switches->update();
    }
}

void red_leds_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd)
{
    unsigned int switchesStates = 0;
    unsigned int buttonStates = 0;

    while (buttonStates != 15)
    {
        buttonStates = buttons->getStatesAsNumber();
        switchesStates = switches->getStatesAsNumber();

        redLeds->setStatesFromNumber(switchesStates);

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        redLeds->update();
    }
}

void green_leds_module(Buttons *buttons, Switches *switches, Leds *redLeds, Leds *greenLeds, SevenSegmentDisplays *sevenSegment, LCD *lcd)
{
    unsigned int buttonStates = 0;

    while (buttonStates != 15)
    {
        buttonStates = buttons->getStatesAsNumber();

        greenLeds->setStatesFromNumber(buttonStates);

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        greenLeds->update();
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

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        sevenSegment->update();
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

        lcd->clear();
        lcd->sendWrite(std::to_string(switchesStates));

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        lcd->update();
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
            // seven_segment_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd);
        }
#pragma omp section
        {
            // lcd_module(&buttons, &switches, &redLeds, &greenLeds, &sevenSegment, &lcd);
        }
    }

    if (fileDescriptor != -1)
    {
        close(fileDescriptor);
    }

    return 0;
}
