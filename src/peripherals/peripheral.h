#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include <iostream>
#include <vector>
#include <unistd.h>     // Para write
#include <sys/ioctl.h>  // Para ioctl

class Peripheral {
protected:
    int fd;
    unsigned int command;
    unsigned int count;
    std::vector<bool> states;

public:
    Peripheral(int fileDescriptor, unsigned int cmd, unsigned int cnt)
        : fd(fileDescriptor), command(cmd), count(cnt), states(cnt, false) {}

    virtual int update() = 0;

    virtual void printStates() {
        std::cout << "Peripheral states: ";
        for (int i = count - 1; i >= 0; --i) { 
            std::cout << states[i] << " ";
        }
        std::cout << std::endl;
    }

    unsigned int getCount() const { return count; }

    virtual bool getState(unsigned int index) {
        if (index >= count) {
            std::cerr << "Error: Index out of range." << std::endl;
            return false;
        }
        return states[index];
    }

    virtual unsigned int getStatesAsNumber() {
        unsigned int number = 0;
        for (unsigned int i = 0; i < count; ++i) {
            if (states[i]) {
                number |= (1 << i);
            }
        }
        return number;
    }

    virtual std::vector<unsigned int> getStatesArray() {
        std::vector<unsigned int> result(count);
        for (unsigned int i = 0; i < count; ++i) {
            result[i] = states[i];
        }
        return result;
    }

    virtual ~Peripheral() {}
};

#endif // PERIPHERAL_H