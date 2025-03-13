#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "ioctl_cmds.h"

// Define SIMULATION_MODE as true for simulation, false for hardware
const bool SIMULATION_MODE = true; // Change to false for hardware

#if SIMULATION_MODE
// Simulation Functions
void simulateGreenLeds(unsigned int value) {
    std::cout << "[SIMULATION] Green LEDs: 0x" << std::hex << value << std::endl;
}

void simulateRedLeds(unsigned int value) {
    std::cout << "[SIMULATION] Red LEDs: 0x" << std::hex << value << std::endl;
}

void simulateLCD(unsigned int value) {
    std::cout << "[SIMULATION] LCD: 0x" << std::hex << value << std::endl;
}

void simulateDisplay(unsigned int command, uint8_t num1, uint8_t num2, uint8_t num3, uint8_t num4, bool useDigits) {
    std::cout << "[SIMULATION] ";
    if (command == WR_L_DISPLAY) {
        std::cout << "Left Display: ";
    } else if (command == WR_R_DISPLAY) {
        std::cout << "Right Display: ";
    }
    if (useDigits) {
        std::cout << (int)num1 << (int)num2 << (int)num3 << (int)num4 << std::endl;
    } else {
        std::cout << "0x" << std::hex << (int)num1 << " 0x" << (int)num2 << " 0x" << (int)num3 << " 0x" << (int)num4 << std::endl;
    }

}
#endif

// Function to write to Green LEDs
int writeGreenLeds(int fd, unsigned int value) {
#if SIMULATION_MODE
    simulateGreenLeds(value);
    return 0;
#else
    if (ioctl(fd, WR_GREEN_LEDS) < 0) {
        std::cerr << "ioctl WR_GREEN_LEDS failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "write Green LEDs failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
#endif
}

// Function to write to Red LEDs
int writeRedLeds(int fd, unsigned int value) {
#if SIMULATION_MODE
    simulateRedLeds(value);
    return 0;
#else
    if (ioctl(fd, WR_RED_LEDS) < 0) {
        std::cerr << "ioctl WR_RED_LEDS failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "write Red LEDs failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
#endif
}

// Function to write to LCD
int writeLCD(int fd, unsigned int value) {
#if SIMULATION_MODE
    simulateLCD(value);
    return 0;
#else
    if (ioctl(fd, LCD_DISPLAY) < 0) {
        std::cerr << "ioctl LCD_DISPLAY failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "write LCD failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
#endif
}

// Function to write to a display
int writeDisplay(int fd, unsigned int command, uint8_t num1, uint8_t num2, uint8_t num3, uint8_t num4, bool useDigits) {
#if SIMULATION_MODE
    simulateDisplay(command, num1, num2, num3, num4, useDigits);
    return 0;
#else
    uint32_t displayValue = 0;
    uint8_t seg1, seg2, seg3, seg4;
    const uint8_t segPatterns[] = {
        0xC0, // 0
        0xF9, // 1
        0xA4, // 2
        0xB0, // 3
        0x99, // 4
        0x92, // 5
        0x82, // 6
        0xF8, // 7
        0x80, // 8
        0x90  // 9
    };

    if (useDigits) {
        seg1 = segPatterns[num1];
        seg2 = segPatterns[num2];
        seg3 = segPatterns[num3];
        seg4 = segPatterns[num4];
    } else {
        seg1 = num1;
        seg2 = num2;
        seg3 = num3;
        seg4 = num4;
    }

    displayValue |= (static_cast<uint32_t>(seg4) << 0);
    displayValue |= (static_cast<uint32_t>(seg3) << 8);
    displayValue |= (static_cast<uint32_t>(seg2) << 16);
    displayValue |= (static_cast<uint32_t>(seg1) << 24);

    if (ioctl(fd, command) < 0) {
        std::cerr << "ioctl display write failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &displayValue, sizeof(displayValue)) != sizeof(displayValue)) {
        std::cerr << "write display failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
#endif
}

// Function to read Push Buttons
int readPushButtons(int fd, unsigned int& value) {
#if SIMULATION_MODE
    std::cout << "[SIMULATION] Reading Push Buttons..." << std::endl;
    value = 0; // Simulate reading 0
    return 0;
#else
    if (ioctl(fd, RD_PBUTTONS) < 0) {
        std::cerr << "ioctl RD_PBUTTONS failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (read(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "read Push Buttons failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
#endif
}

// Function to read Switches
int readSwitches(int fd, unsigned int& value) {
#if SIMULATION_MODE
    std::cout << "[SIMULATION] Reading Switches..." << std::endl;
    value = 0; // Simulate reading 0
    return 0;
#else
    if (ioctl(fd, RD_SWITCHES) < 0) {
        std::cerr << "ioctl RD_SWITCHES failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (read(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "read Switches failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
#endif
}

int main() {
    int fd;
#if !SIMULATION_MODE
    fd = open("/dev/mydev", O_RDWR);
    if (fd < 0) {
        std::cerr << "Failed to open device: " << strerror(errno) << std::endl;
        return 1;
    }
#endif

    // Example Usage:
    unsigned int ledValue, buttons, switches;
    uint8_t leftNum1, leftNum2, leftNum3, leftNum4, rightNum1, rightNum2, rightNum3, rightNum4;

    if (readPushButtons(fd, buttons) != 0) {
#if !SIMULATION_MODE
        close(fd);
#endif
        return 1;