#ifndef LEDS_H
#define LEDS_H

#include "../output_peripheral.h"
#include <iostream>
#include <vector>
#include <cstring>      // Para strerror
#include <cerrno>       // Para errno
// #include <unistd.h>     // Para write
// #include <sys/ioctl.h>  // Para ioctl

class Leds : public OutputPeripheral {
public:
    Leds(int fileDescriptor, unsigned int command, unsigned int ledCount);

    int update() override;
};

#endif // LEDS_H