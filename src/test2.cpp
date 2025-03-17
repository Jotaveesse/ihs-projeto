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

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }

    // Close the device driver
    if (fileDescriptor != -1)
    {
        close(fileDescriptor);
    }

    return 0;
}