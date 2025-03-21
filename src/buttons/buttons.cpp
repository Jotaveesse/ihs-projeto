#include "buttons.h"
#include <chrono>      // Para std::chrono
#include <thread>      // Para std::this_thread
#include <cstring>     // Para strerror
#include <cerrno>      // Para errno
#include <unistd.h>    // Para write
#include <sys/ioctl.h> // Para ioctl

Buttons::Buttons(int fileDescriptor, unsigned int command, unsigned int buttonCount)
    : InputPeripheral(fileDescriptor, command, buttonCount),
      lastStates(buttonCount, false) {}

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

    if (!states[button])
    {
        return false;
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    while (states[button])
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        if (elapsedTime >= duration)
        {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        update();
    }
    return false;
}