#ifndef OUTPUT_PERIPHERAL_H
#define OUTPUT_PERIPHERAL_H

#include "peripheral.h"
#include <vector>

class OutputPeripheral : public Peripheral {
public:
    OutputPeripheral(int fileDescriptor, unsigned int cmd, unsigned int cnt)
        : Peripheral(fileDescriptor, cmd, cnt) {}

    virtual void setState(unsigned int index, bool state) {
        if (index >= count) {
            std::cerr << "Error: Index out of range." << std::endl;
            return;
        }
        states[index] = state;
    }

    virtual void setStatesFromNumber(unsigned int number) {
        for (unsigned int i = 0; i < count; ++i) {
            states[i] = (number & (1 << i)) != 0;
        }
    }

    virtual void setStatesFromArray(const std::vector<bool>& statesArray) {
        if (statesArray.size() != count) {
            std::cerr << "Error: Array size does not match count." << std::endl;
            return;
        }
        states = statesArray;
    }

    virtual void setAllStates(bool state) {
        for (unsigned int i = 0; i < count; ++i) {
            states[i] = state;
        }
    }

    virtual ~OutputPeripheral() {}
};

#endif // OUTPUT_PERIPHERAL_H