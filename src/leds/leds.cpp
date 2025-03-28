#include "leds.h"
#include <unistd.h>    // Para write
#include <sys/ioctl.h> // Para ioctl
#include <chrono>      // Para std::chrono

Leds::Leds(int fileDescriptor, unsigned int command, unsigned int ledCount)
    : OutputPeripheral(fileDescriptor, command, ledCount),
      startTime(std::chrono::high_resolution_clock::now()) {}
int Leds::update()
{
    unsigned int number = 0;
    for (int i = 0; i < count; ++i)
    {
        if (states[i])
        {
            number |= (1 << i);
        }
    }

    if (ioctl(fd, command) < 0)
    {
        std::cerr << "ioctl failed: " << strerror(errno) << std::endl;
        return -1;
    }
    if (write(fd, &number, sizeof(number)) != sizeof(number))
    {
        std::cerr << "write failed: " << strerror(errno) << std::endl;
        return -1;
    }

    // printStates();
    return 0;
}

void blink(unsigned int led, unsigned int offTimeMs)
{
    if (led >= count)
    {
        return; // Invalid LED index
    }

    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

    // Calculate the total cycle time (on time + off time)
    unsigned int cycleTime = offTimeMs;

    // Calculate the current position within the cycle
    unsigned int cyclePosition = elapsed % cycleTime;

    if (cyclePosition < 200)
    {
        states[led] = true; // LED is on for 200ms
    }
    else
    {
        states[led] = false; // LED is off for offTimeMs
    }
}