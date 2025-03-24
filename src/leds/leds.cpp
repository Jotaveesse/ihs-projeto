#include "leds.h"
#include <unistd.h>     // Para write
#include <sys/ioctl.h>  // Para ioctl

Leds::Leds(int fileDescriptor, unsigned int command, unsigned int ledCount)
    : OutputPeripheral(fileDescriptor, command, ledCount) {}

int Leds::update()
{
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
    if (write(fd, &number, sizeof(number)) != sizeof(number)) {
        std::cerr << "write failed: " << strerror(errno) << std::endl;
        return -1;
    }

    // printStates();
    return 0;
}