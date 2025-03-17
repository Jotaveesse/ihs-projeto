#ifndef LCD_H
#define LCD_H

#include "../output_peripheral.h"
#include <iostream> // For basic output if needed
#include <vector>
#include <cstring> // Para strerror
#include <cerrno>  // Para errno
// #include <unistd.h>     // Para write
// #include <sys/ioctl.h>  // Para ioctl

// Define LCD pins
#define LCD_DB0_PIN 0
#define LCD_DB1_PIN 1
#define LCD_DB2_PIN 2
#define LCD_DB3_PIN 3 
#define LCD_DB4_PIN 4 
#define LCD_DB5_PIN 5 
#define LCD_DB6_PIN 6
#define LCD_DB7_PIN 7 
#define LCD_E_PIN 8
#define LCD_RW_PIN 9
#define LCD_RS_PIN 10
#define LCD_ON_PIN 11

// LCD commands
#define LCD_CLEAR 0x01
#define LCD_HOME 0x02
#define LCD_ENTRY_MODE_SET 0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_SHIFT 0x10
#define LCD_FUNCTION_SET 0x20
#define LCD_CGRAM_ADDR 0x40
#define LCD_DDRAM_ADDR 0x80

// Entry mode flags
#define LCD_ENTRY_RIGHT 0x00
#define LCD_ENTRY_LEFT 0x01
#define LCD_ENTRY_SHIFT_INC 0x02
#define LCD_ENTRY_SHIFT_DEC 0x00

// Display control flags
#define LCD_DISPLAY_ON 0x04
#define LCD_DISPLAY_OFF 0x00
#define LCD_CURSOR_ON 0x02
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_ON 0x01
#define LCD_BLINK_OFF 0x00

// Shift flags
#define LCD_SHIFT_DISPLAY 0x08
#define LCD_SHIFT_CURSOR 0x00
#define LCD_SHIFT_RIGHT 0x04
#define LCD_SHIFT_LEFT 0x00

// Function set flags
#define LCD_8_BIT_MODE 0x10
#define LCD_4_BIT_MODE 0x00
#define LCD_2_LINE 0x08
#define LCD_1_LINE 0x00
#define LCD_5x10_FONT 0x04
#define LCD_5x8_FONT 0x00

class LCD : public OutputPeripheral
{
public:
    LCD(int fileDescriptor, unsigned int command);
    void init();
    void turnOn();
    void turnOff();
    void clear();
    void home();
    void setCursor(int col, int row);
    void write(const std::string &text);
    void write(char character);
    void command(uint8_t cmd);
    void setEntryMode(uint8_t flags);
    void setDisplayControl(uint8_t flags);
    void shift(uint8_t flags);
    void setFunction(uint8_t flags);
    void setCGRAMAddress(uint8_t address);
    void setDDRAMAddress(uint8_t address);
    uint8_t readBusyFlagAndAddress();
    void writeData(uint8_t data);
    uint8_t readData();
    int update() override;
    void sendData(uint8_t data);
    void sendInstruction(uint8_t instruction);
    void pulseEnable();
    bool isBusy();

};

#endif