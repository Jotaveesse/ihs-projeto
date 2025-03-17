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

    unsigned int testNumber = 0b101010101010101010;
    unsigned int buttonStates = 1;

    unsigned int buttonCounter = 0;
    unsigned int switchCounter = 0;

    while (buttonStates != 0)
    {
        switches.update();
        unsigned int switchStates = switches.getStatesAsNumber();

        buttons.update();
        buttonStates = buttons.getStatesAsNumber();

        redLeds.setStatesFromNumber(switchStates);
        redLeds.update();

        greenLeds.setStatesFromNumber(buttonStates);
        greenLeds.update();

        sevenSegment.setAllDisplaysFromNumber(switchStates);
        sevenSegment.update();

        lcd.clear();
        lcd.sendWrite(std::to_string(switchStates));

        if (buttons.isButtonPressed(0))
        {
            buttonCounter++;
            buttonCounter %= 16;
            std::cout << buttonCounter; 
        }

        if (buttons.isButtonReleased(1))
        {
            buttonCounter++;
            buttonCounter %= 16;
            std::cout << buttonCounter; 
        }

        if (switches.isSwitchToggledOn(0))
        {
            switchCounter++;
            switchCounter %= 16;
            std::cout << switchCounter; 
        }

        if (switches.isSwitchToggledOff(1))
        {
            switchCounter++;
            switchCounter %= 16;
            std::cout << switchCounter; 
        }

        sevenSegment.setDisplayFromNumber(7, buttonCounter);
        sevenSegment.setDisplayFromNumber(6, switchCounter);

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    // Close the device driver
    if (fileDescriptor != -1)
    {
        close(fileDescriptor);
    }

    return 0;
}