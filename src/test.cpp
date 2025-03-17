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

int main()
{
    int fileDescriptor = -1;

    fileDescriptor = open("/dev/mydev", O_RDWR);
    if (fileDescriptor < 0) {
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

    // Test Outputs (LEDs)
    std::cout << "--- Testing red LEDs (Outputs) ---" << std::endl;
    redLeds.setStatesFromNumber(testNumber);
    redLeds.update();
    std::cout << "red LEDs set to: " << redLeds.getStatesAsNumber() << std::endl;

    // Test Outputs (LEDs)
    std::cout << "\n--- Testing green LEDs (Outputs) ---" << std::endl;
    greenLeds.setStatesFromNumber(testNumber);
    greenLeds.update();
    std::cout << "green LEDs set to: " << greenLeds.getStatesAsNumber() << std::endl;

    // Test Seven Segment Display (Output)
    std::cout << "\n--- Testing Seven Segment Display (Output) ---" << std::endl;
    sevenSegment.setAllDisplaysFromNumber(testNumber);
    sevenSegment.update();
    std::cout << "Seven Segment Display set to: " << testNumber << std::endl;

    // Test LCD(Output)
    std::cout << "\n--- Testing LCD (Output) ---" << std::endl;
    lcd.sendWrite(std::to_string(testNumber));

    // Test Inputs (Switches and Buttons)
    std::cout << "\n--- Testing Switches (Inputs) ---" << std::endl;
    switches.update();
    unsigned int switchStates = switches.getStatesAsNumber();
    std::cout << "Switch states: " << switchStates << std::endl;

    std::cout << "\n--- Testing Push Buttons (Inputs) ---" << std::endl;
    buttons.update();
    unsigned int buttonStates = buttons.getStatesAsNumber();
    std::cout << "Button states: " << buttonStates << std::endl;

    // Close the device driver
    if (fileDescriptor != -1)
    {
        close(fileDescriptor);
    }

    return 0;
}