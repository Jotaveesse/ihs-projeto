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

// Function to write to Green LEDs
int writeGreenLeds(int fd, unsigned int value) {
    if (ioctl(fd, WR_GREEN_LEDS) < 0) {
        std::cerr << "ioctl WR_GREEN_LEDS failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "write Green LEDs failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

// Function to write to Red LEDs
int writeRedLeds(int fd, unsigned int value) {
    if (ioctl(fd, WR_RED_LEDS) < 0) {
        std::cerr << "ioctl WR_RED_LEDS failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "write Red LEDs failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

// Function to write to Left Display
int writeLeftDisplay(int fd, unsigned int value) {
    if (ioctl(fd, WR_L_DISPLAY) < 0) {
        std::cerr << "ioctl WR_L_DISPLAY failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "write Left Display failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

// Function to write to Right Display
int writeRightDisplay(int fd, unsigned int value) {
    if (ioctl(fd, WR_R_DISPLAY) < 0) {
        std::cerr << "ioctl WR_R_DISPLAY failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "write Right Display failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int writeLCD(int fd, unsigned int value) {
    if (ioctl(fd, LCD_DISPLAY) < 0) {
        std::cerr << "ioctl LCD_DISPLAY failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "write LCD failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

// Function to read Push Buttons
int readPushButtons(int fd, unsigned int& value) {
    if (ioctl(fd, RD_PBUTTONS) < 0) {
        std::cerr << "ioctl RD_PBUTTONS failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (read(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "read Push Buttons failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

// Function to read Switches
int readSwitches(int fd, unsigned int& value) {
    if (ioctl(fd, RD_SWITCHES) < 0) {
        std::cerr << "ioctl RD_SWITCHES failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (read(fd, &value, sizeof(value)) != sizeof(value)) {
        std::cerr << "read Switches failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int writeRightDisplay(int fd, uint8_t seg1, uint8_t seg2, uint8_t seg3, uint8_t seg4) {
    uint32_t displayValue = 0;

    // Construct the 32-bit value
    displayValue |= (static_cast<uint32_t>(seg1) << 0);
    displayValue |= (static_cast<uint32_t>(seg2) << 8);
    displayValue |= (static_cast<uint32_t>(seg3) << 16);
    displayValue |= (static_cast<uint32_t>(seg4) << 24);

    if (ioctl(fd, WR_R_DISPLAY) < 0) {
        std::cerr << "ioctl display write failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &displayValue, sizeof(displayValue)) != sizeof(displayValue)) {
        std::cerr << "write display failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int writeLeftDisplay(int fd, uint8_t seg1, uint8_t seg2, uint8_t seg3, uint8_t seg4) {
    uint32_t displayValue = 0;

    // Construct the 32-bit value
    displayValue |= (static_cast<uint32_t>(seg1) << 0);
    displayValue |= (static_cast<uint32_t>(seg2) << 8);
    displayValue |= (static_cast<uint32_t>(seg3) << 16);
    displayValue |= (static_cast<uint32_t>(seg4) << 24);

    if (ioctl(fd, WR_L_DISPLAY) < 0) {
        std::cerr << "ioctl display write failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &displayValue, sizeof(displayValue)) != sizeof(displayValue)) {
        std::cerr << "write display failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int main() {
    int fd = open("/dev/mydev", O_RDWR);
    if (fd < 0) {
        std::cerr << "Failed to open device: " << strerror(errno) << std::endl;
        return 1;
    }

    // Example Usage:
    unsigned int ledValue, buttons, switches, displayValue;

    if (readPushButtons(fd, buttons) != 0) { close(fd); return 1; }
    std::cout << "Push Buttons: 0x" << std::hex << buttons << std::endl;

    if (readSwitches(fd, switches) != 0) { close(fd); return 1; }
    std::cout << "Switches: 0x" << std::hex << switches << std::endl;

    std::cout << "Enter Green LED value (hex): ";
    std::cin >> std::hex >> ledValue;
    if (writeGreenLeds(fd, ledValue) != 0) { close(fd); return 1; }

    std::cout << "Enter Red LED value (hex): ";
    std::cin >> std::hex >> ledValue;
    if (writeRedLeds(fd, ledValue) != 0) { close(fd); return 1; }

    uint8_t leftSeg1, leftSeg2, leftSeg3, leftSeg4;
    std::cout << "Enter Left Display segment values (seg1 seg2 seg3 seg4, hex): ";
    std::cin >> std::hex >> leftSeg1 >> leftSeg2 >> leftSeg3 >> leftSeg4;

    if (writeLeftDisplay(fd, leftSeg1, leftSeg2, leftSeg3, leftSeg4) != 0) {
        close(fd);
        return 1;
    }

    uint8_t rightSeg1, rightSeg2, rightSeg3, rightSeg4;
    std::cout << "Enter Right Display segment values (seg1 seg2 seg3 seg4, hex): ";
    std::cin >> std::hex >> rightSeg1 >> rightSeg2 >> rightSeg3 >> rightSeg4;

    if (writeRightDisplay(fd, rightSeg1, rightSeg2, rightSeg3, rightSeg4) != 0) {
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}