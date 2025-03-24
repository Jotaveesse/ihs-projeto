#include "buttons.h"
#include <chrono>      // Para std::chrono
#include <thread>      // Para std::this_thread
#include <cstring>     // Para strerror
#include <cerrno>      // Para errno
#include <unistd.h>    // Para write
#include <sys/ioctl.h> // Para ioctl

Buttons::Buttons(int fileDescriptor, unsigned int command, unsigned int buttonCount)
    : InputPeripheral(fileDescriptor, command, buttonCount),
        lastStates(buttonCount, false),
        buttonPressStartTimes(buttonCount, NULL) {}

int Buttons::update()
{
    unsigned int number;
    // std::cout << "Enter the button states as a number: ";
    // std::cin >> number;


    if (ioctl(fd, command) < 0)
    {
        std::cerr << "ioctl failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (read(fd, &number, sizeof(number)) != sizeof(number))
    {
        std::cerr << "read failed: " << strerror(errno) << std::endl;
        return -1;
    }

    for (unsigned int i = 0; i < count; ++i)
    {
        lastStates[i] = states[i];
        states[i] = (number & (1 << i)) == 0;

        if (!lastStates[i] && states[i]) {
            buttonPressStartTimes[i] =  std::chrono::duration_cast<std::chrono::milliseconds > (std::chrono::system_clock::now().time_since_epoch()).count();
        } else if (lastStates[i] && !states[i]) {
            buttonPressStartTimes[i] = NULL;
        }
    }

    printStates();
    return 0;

    // return 0;
}

bool Buttons::isButtonPressed(unsigned int button)
{
    if (button >= count)
        return false;
    return states[button];
}

bool Buttons::isButtonClicked(unsigned int button)
{
    if (button >= count)
        return false;
    return !lastStates[button] && states[button];
}

bool Buttons::isButtonReleased(unsigned int button)
{
    if (button >= count)
        return false;
    return lastStates[button] && !states[button];
}

bool Buttons::isButtonPressedLong(unsigned int button, unsigned int duration)
{
    if (button >= count)
        return false;

    if (!states[button]) {
        return false;
    }

    if (buttonPressStartTimes[button] == NULL) {
        return false;
    }

    auto currentTime =  std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
    unsigned long elapsedTime = currentTime - buttonPressStartTimes[button];

    return elapsedTime >= duration;
}