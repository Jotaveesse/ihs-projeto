#ifndef BUTTONS_H
#define BUTTONS_H

#include "../input_peripheral.h"
#include <iostream>
#include <vector>
#include <cstring>      // Para strerror
#include <cerrno>       // Para errno
#include <chrono>      // Para std::chrono

// #include <unistd.h>     // Para write
// #include <sys/ioctl.h>  // Para ioctl

class Buttons : public InputPeripheral {
private:
    std::vector<unsigned long> buttonPressStartTimes;
    std::vector<bool> lastStates;

public:
    Buttons(int fileDescriptor, unsigned int command, unsigned int buttonCount);

    int update() override;

    bool isButtonPressed(unsigned int button);
    bool isButtonClicked(unsigned int button);
    bool isButtonReleased(unsigned int button);
    bool isButtonPressedLong(unsigned int button, unsigned int duration);
};

#endif // BUTTONS_H