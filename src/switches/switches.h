#ifndef SWITCHES_H
#define SWITCHES_H

#include "../input_peripheral.h"
#include <iostream>
#include <vector>
#include <cstring> // Para strerror
#include <cerrno>  // Para errno
// #include <unistd.h>     // Para write
// #include <sys/ioctl.h>  // Para ioctl

class Switches : public InputPeripheral
{
private:
    std::vector<bool> lastStates;

public:
    Switches(int fileDescriptor, unsigned int command, unsigned int switchCount);

    int update() override;

    bool isSwitchOn(unsigned int switchIndex);
    bool isSwitchToggledOn(unsigned int switchIndex);
    bool isSwitchToggledOff(unsigned int switchIndex);
};

#endif // SWITCHES_H