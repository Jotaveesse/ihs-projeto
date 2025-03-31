#ifndef LEDS_H
#define LEDS_H

#include "../output_peripheral.h"
#include <iostream>
#include <vector>
#include <cstring>      // Para strerror
#include <cerrno>       // Para errno
#include <chrono>      // For std::chrono
// #include <unistd.h>     // Para write
// #include <sys/ioctl.h>  // Para ioctl

class Leds : public OutputPeripheral {
public:
    Leds(int fileDescriptor, unsigned int command, unsigned int ledCount);

    int update() override;
    void blink(unsigned int led, unsigned int intervalMs);
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

#endif // LEDS_H