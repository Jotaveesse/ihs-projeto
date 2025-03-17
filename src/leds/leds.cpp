#include "leds.h"

Leds::Leds(int fileDescriptor, unsigned int command, unsigned int ledCount)
    : OutputPeripheral(fileDescriptor, command, ledCount) {}

int Leds::update()
{
    printStates();
    return 0;

    // unsigned int number = 0;
    // for (int i = 0; i < count; ++i) {
    //     if (states[i]) {
    //         number |= (1 << i);
    //     }
    // }

    // if (ioctl(fd, command) < 0) {
    //     std::cerr << "ioctl failed: " << strerror(errno) << std::endl;
    //     return -1;
    // }
    // if (write(fd, &number, sizeof(number)) != sizeof(number)) {
    //     std::cerr << "write failed: " << strerror(errno) << std::endl;
    //     return -1;
    // }
    // return 0;
}