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

// Define LCD pins
#define LCD_RS_PIN   // Port pin for RS
#define LCD_RW_PIN   // Port pin for R/W
#define LCD_E_PIN    // Port pin for E
#define LCD_DB4_PIN  // Port pin for DB4
#define LCD_DB5_PIN  // Port pin for DB5
#define LCD_DB6_PIN  // Port pin for DB6
#define LCD_DB7_PIN  // Port pin for DB7

// LCD commands
#define LCD_CLEAR           0x01
#define LCD_HOME            0x02
#define LCD_ENTRY_MODE_SET  0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_SHIFT           0x10
#define LCD_FUNCTION_SET    0x20
#define LCD_CGRAM_ADDR      0x40
#define LCD_DDRAM_ADDR      0x80

// Entry mode flags
#define LCD_ENTRY_RIGHT          0x00
#define LCD_ENTRY_LEFT           0x02
#define LCD_ENTRY_SHIFT_INC      0x01
#define LCD_ENTRY_SHIFT_DEC      0x00

// Display control flags
#define LCD_DISPLAY_ON           0x04
#define LCD_DISPLAY_OFF          0x00
#define LCD_CURSOR_ON            0x02
#define LCD_CURSOR_OFF           0x00
#define LCD_BLINK_ON             0x01
#define LCD_BLINK_OFF            0x00

// Shift flags
#define LCD_SHIFT_DISPLAY        0x08
#define LCD_SHIFT_CURSOR         0x00
#define LCD_SHIFT_RIGHT          0x04
#define LCD_SHIFT_LEFT           0x00

// Function set flags
#define LCD_8_BIT_MODE         0x10
#define LCD_4_BIT_MODE         0x00
#define LCD_2_LINE             0x08
#define LCD_1_LINE             0x00
#define LCD_5x10_FONT          0x04
#define LCD_5x8_FONT           0x00

//Global variables
char LCDDisplayControl = 0;
char LCDEntryMode = 0;

// Function Prototypes
void LCD_Init(int fd);
void LCD_Cmd(int fd, uint8_t command) {;
void LCD_Data(int fd, uint8_t data);
void LCD_String(int fd, char *str);

//---------------------------------------------------------------
// Initialize the LCD
//---------------------------------------------------------------
void LCD_Init(int fd) {
    // Initialize pins as needed for your microcontroller

    // According to the datasheet, the LCD needs time to stabilize after power-up.
    // A delay here is recommended

    // Function set - 8 bit, 2 lines, 5x8 font
    LCD_Cmd(fd ,LCD_FUNCTION_SET | LCD_8_BIT_MODE | LCD_2_LINE | LCD_5x8_FONT);
    // Delays may be needed here
    LCD_Cmd(fd, LCD_FUNCTION_SET | LCD_8_BIT_MODE | LCD_2_LINE | LCD_5x8_FONT);
    // Delays may be needed here
    LCD_Cmd(fd, LCD_FUNCTION_SET | LCD_8_BIT_MODE | LCD_2_LINE | LCD_5x8_FONT);

    // Display control - turn display off
    LCDDisplayControl = LCD_DISPLAY_OFF | LCD_CURSOR_OFF | LCD_BLINK_OFF;
    LCD_Cmd(fd, LCD_DISPLAY_CONTROL | LCDDisplayControl);

    // Clear display
    LCD_Cmd(fd, LCD_CLEAR);

    // Entry mode set - increment, no shift
    LCDEntryMode = LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DEC;
    LCD_Cmd(fd, LCD_ENTRY_MODE_SET | LCDEntryMode);

    // Display on
    LCDDisplayControl |= LCD_DISPLAY_ON;
    LCD_Cmd(fd, LCD_DISPLAY_CONTROL | LCDDisplayControl);
}

//---------------------------------------------------------------
// Send a command to the LCD
//---------------------------------------------------------------
void LCD_Cmd(int fd, uint8_t command) {
    writeLCD(fd, command, 0);  // RS = 0 for command
}

//---------------------------------------------------------------
// Send data to the LCD
//---------------------------------------------------------------
void LCD_Data(int fd, uint8_t data) {
    writeLCD(fd, data, 1);   // RS = 1 for data
}

//---------------------------------------------------------------
// Send a string to the LCD
//---------------------------------------------------------------
void LCD_String(int fd, char *str) {
    while (*str) {
        LCD_Data(fd, *str++);
    }
}

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

int writeLCD(int fd, uint8_t data, uint8_t rs) {
    // Combine data and RS into a single value
    unsigned int value_to_send = (rs << 8) | data; 

    if (ioctl(fd, LCD_DISPLAY) < 0) {
        std::cerr << "ioctl LCD_DISPLAY failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &value_to_send, sizeof(value_to_send)) != sizeof(value_to_send)) {
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

// Function to write to a display (left or right)
int writeRightDisplayNumber(int fd, uint8_t num1, uint8_t num2, uint8_t num3, uint8_t num4) {
    uint32_t displayValue = 0;

    // Construct the 32-bit value using patterns
    displayValue |= (static_cast<uint32_t>(segPatterns[num4]) << 0);
    displayValue |= (static_cast<uint32_t>(segPatterns[num3]) << 8);
    displayValue |= (static_cast<uint32_t>(segPatterns[num2]) << 16);
    displayValue |= (static_cast<uint32_t>(segPatterns[num1]) << 24);

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

int writeLeftDisplayNumber(int fd, uint8_t num1, uint8_t num2, uint8_t num3, uint8_t num4) {
    uint32_t displayValue = 0;

    // Construct the 32-bit value using patterns
    displayValue |= (static_cast<uint32_t>(segPatterns[num4]) << 0);
    displayValue |= (static_cast<uint32_t>(segPatterns[num3]) << 8);
    displayValue |= (static_cast<uint32_t>(segPatterns[num2]) << 16);
    displayValue |= (static_cast<uint32_t>(segPatterns[num1]) << 24);

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

	LCD_Init(); // Initialize the LCD

	LCD_String("Hello, World!"); // Display a string

	// Move cursor to the beginning of the second line (if a 2-line display)
	LCD_Cmd(LCD_DDRAM_ADDR | 0x40); 
	LCD_String("Second Line");

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

	unsigned int leftNum1, leftNum2, leftNum3, leftNum4;
    std::cout << "Enter Left Display number values (num1 num2 num3 num4, 0-9): ";
    std::cin >> leftNum1 >> leftNum2 >> leftNum3 >> leftNum4;

    if (writeLeftDisplayNumber(fd, leftNum1, leftNum2, leftNum3, leftNum4) != 0) {
        close(fd);
        return 1;
    }

    // Example: Control Right Display
    unsigned int rightNum1, rightNum2, rightNum3, rightNum4;
    std::cout << "Enter Right Display number values (num1 num2 num3 num4, 0-9): ";
    std::cin >> rightNum1 >> rightNum2 >> rightNum3 >> rightNum4;

    if (writeRightDisplayNumber(fd, rightNum1, rightNum2, rightNum3, rightNum4) != 0) {
        close(fd);
        return 1;
    }

    close(fd);

	std::cin >> std::hex >> ledValue;
    return 0;
}