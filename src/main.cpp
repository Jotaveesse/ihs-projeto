#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "ioctl_cmds.h"

int main() {
    int fd = open("/dev/mydev", O_RDWR); // Adjust device path if needed
    if (fd < 0) {
        std::cerr << "Failed to open device: " << strerror(errno) << std::endl;
        return 1;
    }

    // Example: Control Green LEDs
    std::cout << "Enter a value to write to Green LEDs (hex): ";
    unsigned int ledValue;
    std::cin >> std::hex >> ledValue;

    ioctl(fd, WR_GREEN_LEDS);
    write(fd, &ledValue, sizeof(ledValue));

    // Example: Read Push Buttons
    ioctl(fd, RD_PBUTTONS);
    unsigned int buttons;
    read(fd, &buttons, sizeof(buttons));
    std::cout << "Push Buttons: 0x" << std::hex << buttons << std::endl;

    close(fd);

	std::cin >> std::hex >> ledValue;
    return 0;
}