#ifndef SEVEN_SEGMENT_DISPLAYS_H
#define SEVEN_SEGMENT_DISPLAYS_H

#include "../output_peripheral.h"
#include <iostream>
#include <vector>
#include <cstring>      // Para strerror
#include <cerrno>       // Para errno

class SevenSegmentDisplays : public OutputPeripheral {
private:
    unsigned int commandRight;

public:
    SevenSegmentDisplays(int fileDescriptor, unsigned int commandLeft, unsigned int commandRight, unsigned int segCount);

    int update() override;

    unsigned int getNumberFromDisplay(unsigned int displayIndex);

    void setAllDisplay(unsigned int displayIndex, bool val);

    void setDisplayFromNumber(unsigned int displayIndex, unsigned int number);

    void setAllDisplaysFromNumber(unsigned int number);

    void setAllDisplaysFromNumberArray( std::vector<unsigned int> numbers);

    void printDisplays();

};

#endif // SEVEN_SEGMENT_DISPLAYS_H