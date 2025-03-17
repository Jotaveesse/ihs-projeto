#include "switches.h"
#include <iostream>
#include <unistd.h>     // Para write
#include <sys/ioctl.h>  // Para ioctl

Switches::Switches(int fileDescriptor, unsigned int command, unsigned int switchCount)
    : InputPeripheral(fileDescriptor, command, switchCount),
      lastStates(switchCount, false) {}

int Switches::update()
{
    unsigned int number;
    // std::cout << "Enter the switch states as a number: ";
    // std::cin >> number;

    for (unsigned int i = 0; i < count; ++i)
    {
        lastStates[i] = states[i];
        states[i] = (number & (1 << i)) != 0;
    }

    if (ioctl(fd, command) < 0) {
        std::cerr << "ioctl failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (read(fd, &number, sizeof(number)) != sizeof(number)) {
        std::cerr << "read failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;


    printStates();
    return 0;
}

bool Switches::isSwitchOn(unsigned int switchIndex)
{
    if (switchIndex >= count)
        return false;
    return states[switchIndex];
}

bool Switches::isSwitchToggledOn(unsigned int switchIndex)
{
    if (switchIndex >= count)
        return false;
    return !lastStates[switchIndex] && states[switchIndex];
}

bool Switches::isSwitchToggledOff(unsigned int switchIndex)
{
    if (switchIndex >= count)
        return false;
    return lastStates[switchIndex] && !states[switchIndex];
}