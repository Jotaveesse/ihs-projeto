#include "lcd.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <unistd.h>     // Para write
#include <sys/ioctl.h>  // Para ioctl

void delayMicroseconds(int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void delayMilliseconds(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

LCD::LCD(int fileDescriptor, unsigned int command)
    : OutputPeripheral(fileDescriptor, command, 12) {}

int LCD::update(){
    unsigned int number = 0;
    for (int i = 0; i < count; ++i) {
        if (states[i]) {
            number |= (1 << i);
        }
    }

    if (ioctl(fd, command) < 0) {
        std::cerr << "ioctl failed: " << strerror(errno) << std::endl;
        return -1;
    }

    if (write(fd, &number, sizeof(state)) != sizeof(number)) {
        std::cerr << "write failed: " << strerror(errno) << std::endl;
        return -1;
    }

    printStates();
    return 0;
}

void LCD::init() {

    turnOn();
    // Initialization sequence (similar to the datasheet, but in 8-bit mode)
    delayMilliseconds(50);  // Wait for power-up
    
    // Function set (8-bit mode, 2 lines, 5x8 font)
    sendInstruction(LCD_FUNCTION_SET | LCD_8_BIT_MODE | LCD_2_LINE | LCD_5x8_FONT);
    delayMicroseconds(100);

    // Display control (Display on, cursor off, blink off)
    sendInstruction(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_BLINK_ON);
    delayMicroseconds(100);

    // Entry mode set (Increment, no shift)
    sendInstruction(LCD_ENTRY_MODE_SET | LCD_ENTRY_SHIFT_INC);
    delayMicroseconds(100);

    clear();
    home();
}

void LCD::turnOn(){
    setState(LCD_ON_PIN, 1);
    update();
}

void LCD::turnOff(){
    setState(LCD_ON_PIN, 0);
    update();
}

void LCD::clear() {
    sendInstruction(LCD_CLEAR);
    delayMilliseconds(5); //Clear display takes longer
}

void LCD::home() {
    sendInstruction(LCD_HOME);
    delayMilliseconds(5); // Home also takes longer
}

void LCD::setCursor(int col, int row) {
    int address;
    if (row == 0) {
        address = col;
    } else {
        address = 0x40 + col; // Start of second line in DDRAM
    }
    setDDRAMAddress(address);
}

void LCD::write(const std::string& text) {
    for (char c : text) {
        write(c);
    }
}

void LCD::write(char character) {
    setState(LCD_RS_PIN, 1); // Data mode
    sendData(character);
    delayMicroseconds(100); // Adjust as needed
}

void LCD::sendCommand(unsigned int cmd) {
    sendInstruction(cmd);
}

void LCD::setEntryMode(unsigned int flags) {
    sendInstruction(LCD_ENTRY_MODE_SET | flags);
    delayMicroseconds(100);
}

void LCD::setDisplayControl(unsigned int flags) {
    sendInstruction(LCD_DISPLAY_CONTROL | flags);
    delayMicroseconds(100);
}

void LCD::shift(unsigned int flags) {
    sendInstruction(LCD_SHIFT | flags);
    delayMicroseconds(100);
}

void LCD::setCGRAMAddress(unsigned int address) {
    sendInstruction(LCD_CGRAM_ADDR | (address & 0x3F)); // Only 6 bits for CGRAM address
    delayMicroseconds(100);
}

void LCD::setDDRAMAddress(unsigned int address) {
    sendInstruction(LCD_DDRAM_ADDR | (address & 0x7F)); // Only 7 bits for DDRAM address
    delayMicroseconds(100);
}

unsigned int LCD::readBusyFlagAndAddress() {
    setState(LCD_RS_PIN, 0);
    setState(LCD_RW_PIN, 1);  // Read mode
    setState(LCD_E_PIN, 1);
    update();
    delayMicroseconds(100);
    unsigned int data = 0;
    if (getState(LCD_DB7_PIN)) data |= 0x80;
    if (getState(LCD_DB6_PIN)) data |= 0x40;
    if (getState(LCD_DB5_PIN)) data |= 0x20;
    if (getState(LCD_DB4_PIN)) data |= 0x10;
    if (getState(LCD_DB3_PIN)) data |= 0x08;
    if (getState(LCD_DB2_PIN)) data |= 0x04;
    if (getState(LCD_DB1_PIN)) data |= 0x02;
    if (getState(LCD_DB0_PIN)) data |= 0x01;
    setState(LCD_E_PIN, 0);
    setState(LCD_RW_PIN, 0);
    return data;
}

void LCD::writeData(unsigned int data) {
    setState(LCD_RS_PIN, 1); // Data mode
    sendData(data);
    delayMicroseconds(100); // Adjust as needed
}

unsigned int LCD::readData() {
    setState(LCD_RS_PIN, 1);
    setState(LCD_RW_PIN, 1);  // Read mode
    setState(LCD_E_PIN, 1);
    update();
    delayMicroseconds(100);
    unsigned int data = 0;
    if (getState(LCD_DB7_PIN)) data |= 0x80;
    if (getState(LCD_DB6_PIN)) data |= 0x40;
    if (getState(LCD_DB5_PIN)) data |= 0x20;
    if (getState(LCD_DB4_PIN)) data |= 0x10;
    if (getState(LCD_DB3_PIN)) data |= 0x08;
    if (getState(LCD_DB2_PIN)) data |= 0x04;
    if (getState(LCD_DB1_PIN)) data |= 0x02;
    if (getState(LCD_DB0_PIN)) data |= 0x01;
    setState(LCD_E_PIN, 0);
    setState(LCD_RW_PIN, 0);
    return data;
}

void LCD::sendData(unsigned int data) {
    setState(LCD_DB7_PIN, (data >> 7) & 0x01);
    setState(LCD_DB6_PIN, (data >> 6) & 0x01);
    setState(LCD_DB5_PIN, (data >> 5) & 0x01);
    setState(LCD_DB4_PIN, (data >> 4) & 0x01);
    setState(LCD_DB3_PIN, (data >> 3) & 0x01);
    setState(LCD_DB2_PIN, (data >> 2) & 0x01);
    setState(LCD_DB1_PIN, (data >> 1) & 0x01);
    setState(LCD_DB0_PIN, data & 0x01);
    pulseEnable();
}

void LCD::sendInstruction(unsigned int instruction) {
    setState(LCD_RS_PIN, 0); // Instruction mode
    sendData(instruction);
}

void LCD::pulseEnable() {
    setState(LCD_E_PIN, 1);
    update();
    delayMicroseconds(100); // tEH minimum 230ns
    setState(LCD_E_PIN, 0);
    update();
    delayMicroseconds(100); // tEL minimum 230ns
}

bool LCD::isBusy() {
    setState(LCD_RS_PIN, 0);
    setState(LCD_RW_PIN, 1);  // Read mode
    setState(LCD_E_PIN, 1);
    delayMicroseconds(100);
    bool busy = getState(LCD_DB7_PIN); // Busy flag is on DB7
    setState(LCD_E_PIN, 0);
    setState(LCD_RW_PIN, 0);
    return busy;
}