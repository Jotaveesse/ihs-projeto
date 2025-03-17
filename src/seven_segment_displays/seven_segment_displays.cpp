#include "seven_segment_displays.h"
#include <thread>  // Para std::this_thread
#include <cstring> // Para strerror
#include <cerrno>  // Para errno
#include <iomanip> // Para setw
#include <iostream>
#include <unistd.h>    // Para write
#include <sys/ioctl.h> // Para ioctl

unsigned int segPatterns[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // B
    0x39, // C
    0x5E, // D
    0x79, // E
    0x71  // F
};

SevenSegmentDisplays::SevenSegmentDisplays(int fileDescriptor, unsigned int commandLeft, unsigned int commandRight, unsigned int segCount)
    : OutputPeripheral(fileDescriptor, commandLeft, segCount * 8), commandRight(commandRight) {}

int SevenSegmentDisplays::update()
{
    unsigned int stateRight = 0;
    for (int i = 0; i < count / 2; ++i)
    {
        if (!states[i])
        {
            stateRight |= (1 << i);
        }
    }

    unsigned int stateLeft = 0;
    for (int i = 0; i < count / 2; ++i)
    {
        if (!states[i + count / 2])
        {
            stateLeft |= (1 << i);
        }
    }

    if (ioctl(fd, command) < 0)
    {
        std::cerr << "ioctl display write failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &stateRight, sizeof(stateRight)) != sizeof(stateRight))
    {
        std::cerr << "write display failed: " << strerror(errno) << std::endl;
        return -1;
    }

    if (ioctl(fd, commandRight) < 0)
    {
        std::cerr << "ioctl display write failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &stateLeft, sizeof(stateLeft)) != sizeof(stateLeft))
    {
        std::cerr << "write display failed: " << strerror(errno) << std::endl;
        return -1;
    }

    // printDisplays();

    return 0;
}

void SevenSegmentDisplays::setDisplayFromNumber(unsigned int displayIndex, unsigned int number)
{
    number = number % 16;

    unsigned int indexOffset = (((count / 8) - 1) - displayIndex) * 8;

    unsigned segCode = segPatterns[number];

    for (unsigned int i = 0; i < 7; ++i)
    {
        states[indexOffset + i] = (segCode & (1 << i)) != 0;
    }
}

void SevenSegmentDisplays::setAllDisplaysFromNumber(unsigned int number)
{

    std::vector<unsigned int> digits;

    unsigned int tempNumber = number;
    for (unsigned int displayIndex = 0; displayIndex < count / 8; ++displayIndex)
    {
        digits.insert(digits.begin(), tempNumber % 10);
        tempNumber /= 10;
    }

    setAllDisplaysFromNumberArray(digits);
}

void SevenSegmentDisplays::setAllDisplaysFromNumberArray(std::vector<unsigned int> numbers)
{

    for (unsigned int displayIndex = 0; displayIndex < count / 8; ++displayIndex)
    {
        unsigned int digit = numbers[displayIndex];
        digit = digit % 16;

        unsigned int segCode = segPatterns[digit];
        unsigned int indexOffset = (((count / 8) - 1) - displayIndex) * 8;

        for (unsigned int i = 0; i < 7; ++i)
        {
            states[indexOffset + i] = (segCode & (1 << i)) != 0;
        }
    }
}

void SevenSegmentDisplays::printDisplays()
{
    // Linhas para cada parte do display
    std::string line1, line2, line3;

    for (unsigned int displayIndex = 0; displayIndex < count / 8; ++displayIndex)
    {
        unsigned int indexOffset = displayIndex * 8;
        unsigned char segCode = 0;

        // Construir o código do segmento a partir do vetor states
        for (unsigned int i = 0; i < 7; ++i)
        {
            if (states[indexOffset + i])
            {
                segCode |= (1 << i);
            }
        }

        // Construir as linhas para o display atual
        line1 += (segCode & 0x01 ? " _ " : "   ");
        line2 += (segCode & 0x20 ? std::string("|") : std::string(" ")) +
                 (segCode & 0x40 ? std::string("_") : std::string(" ")) +
                 (segCode & 0x02 ? std::string("|") : std::string(" "));
        line3 += (segCode & 0x10 ? std::string("|") : std::string(" ")) +
                 (segCode & 0x08 ? std::string("_") : std::string(" ")) +
                 (segCode & 0x04 ? std::string("|") : std::string(" "));

        // Adiciona um espaço entre os displays
        if (displayIndex < (count / 8) - 1)
        {
            line1 += " ";
            line2 += " ";
            line3 += " ";
        }
    }

    // Imprimir as linhas
    std::cout << line1 << std::endl;
    std::cout << line2 << std::endl;
    std::cout << line3 << std::endl;
}